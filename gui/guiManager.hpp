#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "gui/IGuiManager.hpp"

#include "gui/dialogFrame.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/gdsScene.hpp"
#include "gui/mainView.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

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
            [this]{ DialogFinished(); }
        },
        mWorldDialogFrame{mBackgrounds},
        mSpriteManager{spriteManager},
        mMainView{spriteManager},
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

    void EnterGDSScene(const BAK::HotspotRef& hotspot) override
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
                mScreenStack,
                static_cast<IGuiManager&>(*this)));
    }

    void ExitGDSScene() override
    {
        mScreenStack.PopChild();
        mCursor.PopCursor();
        mGdsScenes.pop_back();
        if (mGdsSceneFinished)
            mGdsSceneFinished();
    }

    void SetGDSSceneFinished(std::function<void()>&& gdsSceneFinished)
    {
        mGdsSceneFinished = gdsSceneFinished;
    }

    void StartDialog(
        BAK::Target target,
        bool isTooltip,
        IDialogScene* scene) override
    {
        mCursor.PushCursor(0);
        mScreenStack.PushScreen(&mWorldDialogFrame);
        mScreenStack.PushScreen(&mDialogRunner);
        mDialogScene = scene;
        mDialogRunner.SetDialogScene(scene);
        mDialogRunner.BeginDialog(target, isTooltip);
    }

    void DialogFinished()
    {
        assert(mDialogScene);
        mScreenStack.PopScreen(); // Dialog frame
        mScreenStack.PopScreen(); // Dialog runner
        mCursor.PopCursor();
        mDialogScene->DialogFinished();
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
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;
    BAK::GameState& mGameState;
    IDialogScene* mDialogScene;
    std::function<void()> mGdsSceneFinished;

    const Logging::Logger& mLogger;
};

}
