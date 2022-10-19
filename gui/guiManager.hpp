#pragma once

#include "audio/audio.hpp"

#include "bak/IZoneLoader.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/assert.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/animatorStore.hpp"
#include "gui/dialogFrame.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/fadeScreen.hpp"
#include "gui/gdsScene.hpp"
#include "gui/icons.hpp"
#include "gui/fontManager.hpp"
#include "gui/info/infoScreen.hpp"
#include "gui/inventory/inventoryScreen.hpp"
#include "gui/lock/lockScreen.hpp"
#include "gui/lock/moredhelScreen.hpp"
#include "gui/fullMap.hpp"
#include "gui/mainView.hpp"
#include "gui/mainMenuScreen.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GuiScreen
{
public:
    GuiScreen(std::function<void()> finished)
    :
        mFinished{finished}
    {}

    std::function<void()> mFinished;
};

class GuiManager final : public Widget, public IGuiManager
{
public:

    GuiManager(
        Cursor& cursor,
        Graphics::SpriteManager& spriteManager,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Rect,
            Graphics::SpriteSheetIndex{0},
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mFontManager{spriteManager},
        mActors{spriteManager},
        mBackgrounds{spriteManager},
        mIcons{spriteManager},
        mCursor{cursor},
        mGameState{gameState},
        mScreenStack{},
        mDialogRunner{
            glm::vec2{0, 0},
            glm::vec2{320, 240},
            mActors,
            mBackgrounds,
            mFontManager.GetGameFont(),
            gameState,
            mScreenStack,
            [this](const auto& choice){ DialogFinished(choice); }
        },
        mWorldDialogFrame{mBackgrounds},
        mSpriteManager{spriteManager},
        mMainView{*this, mBackgrounds, mIcons},
        mMainMenu{*this, mBackgrounds, mIcons, mFontManager.GetGameFont()},
        mInfoScreen{
            *this,
            mActors,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState
        },
        mInventoryScreen{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState
        },
        mLockScreen{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState
        },
        mFullMap{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState
        },
        mMoredhelScreen{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetAlienFont(),
            mFontManager.GetPuzzleFont(),
            mGameState
        },
        mFadeScreen{
            *this,
            [this]{ FadeInDone(); },
            [this]{ FadeOutDone(); }
        },
        mFadeFunction{},
        mGdsScenes{},
        mDialogScene{nullptr},
        mGuiScreens{},
        mAnimatorStore{},
        mZoneLoader{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
    {
        mGdsScenes.reserve(4);
        AddChildBack(&mScreenStack);
    }

    ScreenStack& GetScreenStack() override
    {
        return mScreenStack;
    }

    void LoadGame(std::string save) override
    {
        ASSERT(mZoneLoader);
        mZoneLoader->LoadGame(save);
        EnterMainView();
    }

    void SetZoneLoader(BAK::IZoneLoader* zoneLoader)
    {
        ASSERT(zoneLoader);
        mZoneLoader = zoneLoader;
    }

    void DoFade(double duration, std::function<void()>&& fadeFunction) override
    {
        if (!HaveChild(&mFadeScreen))
        {
            mFadeFunction = std::move(fadeFunction);
            AddChildBack(&mFadeScreen);
            mFadeScreen.FadeIn(duration);
        }
    }

    void EnterMainView() override
    {
        mMainView.UpdatePartyMembers(mGameState);
        DoFade(1.0,[this]{
            mScreenStack.PopScreen();
            mScreenStack.PushScreen(&mMainView);
        });
    }

    void EnterMainMenu(bool gameRunning) override
    {
        DoFade(1.0, [this, gameRunning]{
            if (gameRunning)
            {
                mScreenStack.PopScreen();
            }
            mScreenStack.PushScreen(&mMainMenu);
            mMainMenu.EnterMainMenu(gameRunning);
        });
    }

    void TeleportToGDS(
        const BAK::HotspotRef& hotspot)
    {
        mLogger.Debug() << __FUNCTION__ << ":" << hotspot << "\n";
        // When teleporting we need to add the "root" GDS scene to the stack
        // because it won't have been there...
        if (hotspot.mGdsNumber < 12 && hotspot.mGdsChar != 'A')
            EnterGDSScene(
                BAK::HotspotRef{hotspot.mGdsNumber, 'A'},
                []{});
        EnterGDSScene(hotspot, []{});
    }

    void OnTimeDelta(double delta)
    {
        mAnimatorStore.OnTimeDelta(delta);
    }

    void AddAnimator(LinearAnimator&& animator) override
    {
        mAnimatorStore.AddAnimator(std::move(animator));
    }

    void EnterGDSScene(
        const BAK::HotspotRef& hotspot,
        std::function<void()>&& finished) override
    {
        DoFade(1.0, [this, hotspot=hotspot, finished=std::move(finished)]() mutable {
        mLogger.Debug() << __FUNCTION__ << ":" << hotspot << "\n";
        mCursor.PushCursor(0);

        mGdsScenes.emplace_back(
            std::make_unique<GDSScene>(
                mCursor,
                hotspot,
                mSpriteManager,
                mActors,
                mBackgrounds,
                mFontManager.GetGameFont(),
                mGameState,
                static_cast<IGuiManager&>(*this)));

        const auto song = mGdsScenes.back()->GetSong();
        if (song != 0)
        {
            AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{song});
            mGuiScreens.push(GuiScreen{
                [fin = std::move(finished)](){
                    AudioA::AudioManager::Get().PopTrack();
                    std::invoke(fin);
            }});
        }
        else
        {
            mGuiScreens.push(finished);
        }

        mScreenStack.PushScreen(mGdsScenes.back().get());
        });
    }

    void ExitGDSScene() override
    {
        mLogger.Debug() << "Exiting GDS Scene" << std::endl;
        RemoveGDSScene(true);
    }

    void RemoveGDSScene(bool runFinished=false)
    {
        mScreenStack.PopChild();
        mCursor.PopCursor();
        mCursor.PopCursor();
        if (runFinished)
            PopAndRunGuiScreen();
        else
            PopGuiScreen();
        mGdsScenes.pop_back();
        mLogger.Debug() << "Removed GDS Scene" << std::endl;
    }

    void StartDialog(
        BAK::Target dialog,
        bool isTooltip,
        bool drawWorldFrame,
        IDialogScene* scene) override
    {
        mCursor.PushCursor(0);
        if (drawWorldFrame)
            mScreenStack.PushScreen(&mWorldDialogFrame);
        mGuiScreens.push(GuiScreen{[this, drawWorldFrame](){
            if (drawWorldFrame)
                mScreenStack.PopScreen();
        }});

        mScreenStack.PushScreen(&mDialogRunner);
        mDialogScene = scene;
        mDialogRunner.SetDialogScene(scene);
        mDialogRunner.BeginDialog(dialog, isTooltip);
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mDialogScene);
        PopAndRunGuiScreen();
        mScreenStack.PopScreen(); // Dialog runner
        mCursor.PopCursor();

        mLogger.Debug() << "Finished dialog with choice : " << choice << "\n";
        mDialogScene->DialogFinished(choice);
        const auto teleport = mDialogRunner.GetAndResetPendingTeleport();
        if (teleport)
        {
            mLogger.Info() << "Teleporting to teleport index: " << *teleport << "\n";
            // Clear all stacked GDS scenes
            while (!mGdsScenes.empty())
                RemoveGDSScene();

            if (mZoneLoader)
                mZoneLoader->DoTeleport(*teleport);
        }
    }

    void ShowCharacterPortrait(BAK::ActiveCharIndex character) override
    {
        DoFade(.8, [this, character]{
            mInfoScreen.SetSelectedCharacter(character);
            mInfoScreen.UpdateCharacter();
            mScreenStack.PushScreen(&mInfoScreen);
        });
    }

    void ExitCharacterPortrait() override
    {
        DoFade(.8, [this]{
            mScreenStack.PopScreen();
        });
    }

    void ShowInventory(BAK::ActiveCharIndex character) override
    {
        DoFade(.8, [this, character]{
            mCursor.PushCursor(0);
            mGuiScreens.push(GuiScreen{[](){}});

            mInventoryScreen.SetSelectedCharacter(character);
            mScreenStack.PushScreen(&mInventoryScreen);
        });
    }

    void ShowContainer(BAK::IContainer* container) override
    {
        mCursor.PushCursor(0);
        ASSERT(container->GetInventory().GetCapacity() > 0);

        mGuiScreens.push(GuiScreen{[&](){
            mInventoryScreen.ClearContainer();
        }});

        mInventoryScreen.SetContainer(container);
        mLogger.Debug() << __FUNCTION__ << " Pushing inv\n";
        mScreenStack.PushScreen(&mInventoryScreen);
    }

    void ExitInventory() override
    {
        mCursor.PopCursor();
        PopAndRunGuiScreen();
        mScreenStack.PopScreen();
    }

    void ShowLock(
        BAK::IContainer* container,
        std::function<void()>&& finished) override
    {
        ASSERT(container->HasLock()
            && (container->GetLock().IsFairyChest()
                || !container->GetLock().IsTrapped()));

        mCursor.PushCursor(0);

        if (container->GetLock().IsFairyChest())
        {
            mMoredhelScreen.SetContainer(container);
            mScreenStack.PushScreen(&mMoredhelScreen);
            AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::PUZZLE_CHEST_THEME);
            mGuiScreens.push(GuiScreen{
                [fin = std::move(finished)](){
                    AudioA::AudioManager::Get().PopTrack();
                    std::invoke(fin);
            }});
        }
        else
        {
            mLockScreen.SetContainer(container);
            mScreenStack.PushScreen(&mLockScreen);
            mGuiScreens.push(finished);
        }
    }

    void ShowFullMap() override
    {
        DoFade(.8, [this]{
            mScreenStack.PushScreen(&mFullMap);
        });
    }

    void ExitLock() override
    {
        mCursor.PopCursor();
        mScreenStack.PopScreen();
        PopAndRunGuiScreen();
    }

    bool IsLockOpened() const override
    {
        return mLockScreen.IsUnlocked();
    } 

    bool IsWordLockOpened() const override
    {
        return mMoredhelScreen.IsUnlocked();
    }

    void PopGuiScreen()
    {
        mGuiScreens.pop();
    }

    void PopAndRunGuiScreen()
    {
        // Avoids reentrancy issue by ensuring this gui screen is not
        // in the stack when it is executed.
        auto guiScreen = mGuiScreens.top();
        mGuiScreens.pop();
        guiScreen.mFinished();
    }

//private:
    void FadeInDone()
    {
        ASSERT(mFadeFunction);
        mFadeFunction();
        mFadeScreen.FadeOut();
    }

    void FadeOutDone()
    {
        RemoveChild(&mFadeScreen);
    }

    FontManager mFontManager;
    Actors mActors;
    Backgrounds mBackgrounds;
    Icons mIcons;

    Cursor& mCursor;
    BAK::GameState& mGameState;
    ScreenStack mScreenStack;
    DialogRunner mDialogRunner;
    WorldDialogFrame mWorldDialogFrame;

    Graphics::SpriteManager& mSpriteManager;

    MainView mMainView;
    MainMenuScreen mMainMenu;
    InfoScreen mInfoScreen;
    InventoryScreen mInventoryScreen;
    LockScreen mLockScreen;
    FullMap mFullMap;
    MoredhelScreen mMoredhelScreen;
    FadeScreen mFadeScreen;
    std::function<void()> mFadeFunction;
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;

    IDialogScene* mDialogScene;
    std::stack<GuiScreen> mGuiScreens;

    AnimatorStore mAnimatorStore;
    BAK::IZoneLoader* mZoneLoader;

    const Logging::Logger& mLogger;
};

}
