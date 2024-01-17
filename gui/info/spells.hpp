#pragma once

#include "bak/gameState.hpp"

#include "gui/backgrounds.hpp"
#include "gui/fontManager.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/icons.hpp"
#include "gui/textBox.hpp"

namespace Gui {


class SpellList : public Widget
{
public:
    SpellList(
        unsigned symbolButtonIndex,
        glm::vec2 pos,
        const Icons& icons)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            std::get<Graphics::SpriteSheetIndex>(icons.GetButton(symbolButtonIndex)),
            std::get<Graphics::TextureIndex>(icons.GetButton(symbolButtonIndex)),
            Graphics::ColorMode::Texture,
            Color::black,
            pos,
            std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)),
            true},
        mSpells{
            glm::vec2{
                std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)).x + 4,
                4},
            glm::vec2{240, 40}}
    {
        AddChildBack(&mSpells);
    }

    void AddSpells(const Font& font)
    {
        mSpells.SetText(font, "Bane of Black Slayers, Evil Seek, Fetters of Rime, Flamecast, Life Drain, Skyfire", false, false, true);
    }

private:
    TextBox mSpells;
};

class SpellsScreen : public Widget
{
    static constexpr auto sCombatSpellButtonBaseIndex = 36;
    static constexpr auto sWorldSpellButtonBaseIndex = 55;
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
        const auto& [_, __, dims] = icons.GetButton(36);
        glm::vec2 pos{4,8};
        mSpellLists.reserve(6);
        for (unsigned i = 0; i < 4; i++)
        {
            mSpellLists.emplace_back(
                sCombatSpellButtonBaseIndex + i,
                pos,
                icons);
            mSpellLists.back().AddSpells(font);
            pos += glm::vec2{0, dims.y + 2};
        }

        for (unsigned i = 0; i < 2; i++)
        {
            mSpellLists.emplace_back(
                sWorldSpellButtonBaseIndex + i,
                pos,
                icons);
            mSpellLists.back().AddSpells(font);
            pos += glm::vec2{0, dims.y + 2};
        }
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
        for (auto& spellList : mSpellLists)
        {
            AddChildBack(&spellList);
        }
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    BAK::ActiveCharIndex mSelectedCharacter;

    std::vector<SpellList> mSpellLists;

    const Logging::Logger& mLogger;
};

}
