#pragma once

#include "bak/IZoneLoader.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/assert.hpp"

#include "gui/IGuiManager.hpp"

#include "gui/animatorStore.hpp"
#include "gui/dialogFrame.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/gdsScene.hpp"
#include "gui/icons.hpp"
#include "gui/fontManager.hpp"
#include "gui/info/infoScreen.hpp"
#include "gui/inventory/inventoryScreen.hpp"
#include "gui/lock/lockScreen.hpp"
#include "gui/mainView.hpp"
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

class GuiManager : public Widget, public IGuiManager
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
        mInfoScreen{
            *this,
            mActors,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState},
        mInventoryScreen{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState},
        mLockScreen{
            *this,
            mBackgrounds,
            mIcons,
            mFontManager.GetGameFont(),
            mGameState},
        mGdsScenes{},
        mDialogScene{nullptr},
        mGuiScreens{},
        mAnimatorStore{},
        mZoneLoader{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
    {
        AddChildBack(&mScreenStack);
    }

    void SetZoneLoader(BAK::IZoneLoader* zoneLoader)
    {
        ASSERT(zoneLoader);
        mZoneLoader = zoneLoader;
    }

    void EnterMainView()
    {
        mScreenStack.PushScreen(&mMainView);
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
        mGuiScreens.push(std::move(finished));
        mScreenStack.PushScreen(mGdsScenes.back().get());
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
        mInfoScreen.SetSelectedCharacter(character);
        mInfoScreen.UpdateCharacter();
        mScreenStack.PushScreen(&mInfoScreen);
    }

    void ExitCharacterPortrait() override
    {
        mScreenStack.PopScreen();
    }

    void ShowInventory(BAK::ActiveCharIndex character) override
    {
        mCursor.PushCursor(0);
        mGuiScreens.push(GuiScreen{[](){}});

        mInventoryScreen.SetSelectedCharacter(character);
        mScreenStack.PushScreen(&mInventoryScreen);
    }

    void ShowContainer(BAK::IContainer* container) override
    {
        mCursor.PushCursor(0);
        mLogger.Debug() << __FUNCTION__ << ":" << (*container).GetLockData() << "\n";

        if (!mLockScreen.IsUnlocked()
            && container->GetLockData().mRating > 0 
            && container->GetContainerType() == BAK::ContainerType::FairyChest)
        {
            ShowLock(container);
        }
        else
        {
            mGuiScreens.push(GuiScreen{[&](){
                mInventoryScreen.ClearContainer();
            }});

            mInventoryScreen.SetContainer(container);
            mScreenStack.PushScreen(&mInventoryScreen);
            mLockScreen.ResetUnlocked();
        }
    }

    void ExitInventory() override
    {
        mCursor.PopCursor();
        PopAndRunGuiScreen();
        mScreenStack.PopScreen();
    }

    void ShowLock(BAK::IContainer* container) override
    {
        mCursor.PushCursor(0);
        mLockScreen.SetContainer(container);
        mScreenStack.PushScreen(&mLockScreen);
    }

    void ExitLock() override
    {
        mCursor.PopCursor();
        mScreenStack.PopScreen();

        if (mLockScreen.IsUnlocked())
        {
            ShowContainer(mLockScreen.GetContainer());
        }
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
    InfoScreen mInfoScreen;
    InventoryScreen mInventoryScreen;
    LockScreen mLockScreen;
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;

    IDialogScene* mDialogScene;
    std::stack<GuiScreen> mGuiScreens;

    AnimatorStore mAnimatorStore;
    BAK::IZoneLoader* mZoneLoader;

    const Logging::Logger& mLogger;
};

}
