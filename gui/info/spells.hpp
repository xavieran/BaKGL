#pragma once

#include "bak/gameState.hpp"

#include "gui/backgrounds.hpp"
#include "gui/fontManager.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/icons.hpp"

namespace Gui {

class SpellsScreen : public Widget
{
public:
    SpellsScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("DIALOG.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mGuiManager{guiManager},
        mFont{font},
        mGameState{gameState},
        mSelectedCharacter{0},
        mLogger{Logging::LogState::GetLogger("Gui::SpellsScreen")}
    {
        AddChildren();
    }

    void SetSelectedCharacter(BAK::ActiveCharIndex character)
    {
        mSelectedCharacter = character;
        mGameState.GetParty().GetCharacter(character).UpdateSkills();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ Exit(); return true; },
            [this](const RightMousePress& p){ Exit(); return true; },
            [](const auto& p){ return false; }
            },
            event);
    }

private:
    void Exit()
    {
        mGuiManager.DoFade(0.8, [this]{ mGuiManager.ExitSimpleScreen(); });
    }

    void AddChildren()
    {
    }


    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    BAK::ActiveCharIndex mSelectedCharacter;

    const Logging::Logger& mLogger;
};

}
