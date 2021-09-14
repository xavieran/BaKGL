#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "gui/IGuiManager.hpp"

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
        mSpriteManager{spriteManager},
        mMainView{spriteManager},
        mGdsScenes{},
        mScreens{},
        mGameState{gameState},
        mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
    {
        AddChildBack(&mScreens);
    }

    void EnterMainView()
    {
        mScreens.PushScreen(&mMainView);
    }

    void EnterGDSScene(const BAK::HotspotRef& hotspot) override
    {
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
                mScreens,
                static_cast<IGuiManager&>(*this)));
    }

    void ExitGDSScene() override
    {
        mScreens.PopChild();
        mCursor.PopCursor();
        //mGdsScenes.pop_back();
    }

    void RunDialog(BAK::KeyTarget dialogTarget)
    {
    }

    void RunContainer(BAK::KeyTarget dialogTarget)
    {
    }

//private:
    Font mFont;
    Actors mActors;
    Backgrounds mBackgrounds;

    Cursor& mCursor;
    Graphics::SpriteManager& mSpriteManager;

    MainView mMainView;
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;
    ScreenStack mScreens;

    BAK::GameState& mGameState;

    const Logging::Logger& mLogger;
};

}
