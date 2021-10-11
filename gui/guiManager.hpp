#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "gui/IGuiManager.hpp"

#include "gui/dialogFrame.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/gdsScene.hpp"
#include "gui/mainView.hpp"
#include "gui/portraitScreen.hpp"
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
        mFont{"GAME.FNT", spriteManager},
        mActors{spriteManager},
        mBackgrounds{spriteManager},
        mCursor{cursor},
        mScreenStack{},
        mDialogRunner{
            glm::vec2{0, 0},
            glm::vec2{320, 240},
            mActors,
            mBackgrounds,
            mFont,
            gameState,
            mScreenStack,
            [this](const auto& choice){ DialogFinished(choice); }
        },
        mWorldDialogFrame{mBackgrounds},
        mSpriteManager{spriteManager},
        mMainView{spriteManager, *this},
        mPortraitScreen{spriteManager, *this, mFont},
        mGdsScenes{},
        mGameState{gameState},
        mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
    {
        AddChildBack(&mScreenStack);
    }

    void EnterMainView()
    {
        mScreenStack.PushScreen(&mMainView);
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
                mFont,
                mGameState,
                static_cast<IGuiManager&>(*this)));
        mGuiScreens.push(std::move(finished));
        mScreenStack.PushScreen(mGdsScenes.back().get());
    }

    void ExitGDSScene() override
    {
        mScreenStack.PopChild();
        mCursor.PopCursor();
        mGdsScenes.pop_back();
        // Run the GDS scene exit fn
        mGuiScreens.top().mFinished();
        mGuiScreens.pop();
    }

    void StartDialog(
        BAK::Target dialog,
        bool isTooltip,
        IDialogScene* scene) override
    {
        mCursor.PushCursor(0);
        mScreenStack.PushScreen(&mWorldDialogFrame);
        mScreenStack.PushScreen(&mDialogRunner);
        mDialogScene = scene;
        mDialogRunner.SetDialogScene(scene);
        mDialogRunner.BeginDialog(dialog, isTooltip);
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        assert(mDialogScene);
        mScreenStack.PopScreen(); // Dialog frame
        mScreenStack.PopScreen(); // Dialog runner
        mCursor.PopCursor();
        mDialogScene->DialogFinished(choice);
    }

    void ShowCharacterPortrait(unsigned character) override
    {
        mScreenStack.PushScreen(&mPortraitScreen);
    }

    void ExitCharacterPortrait() override
    {
        mScreenStack.PopScreen();
    }

    void RunContainer(BAK::KeyTarget dialogTarget)
    {
    }

//private:
    Font mFont;
    Actors mActors;
    Backgrounds mBackgrounds;

    Cursor& mCursor;
    ScreenStack mScreenStack;
    DialogRunner mDialogRunner;
    WorldDialogFrame mWorldDialogFrame;

    Graphics::SpriteManager& mSpriteManager;

    MainView mMainView;
    PortraitScreen mPortraitScreen;
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;
    BAK::GameState& mGameState;
    IDialogScene* mDialogScene;
    std::stack<GuiScreen> mGuiScreens;

    const Logging::Logger& mLogger;
};

}
