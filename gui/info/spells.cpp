#include "gui/info/spells.hpp"

#include "bak/gameState.hpp"

#include "gui/backgrounds.hpp"
#include "gui/button.hpp"
#include "gui/fontManager.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/icons.hpp"
#include "gui/textBox.hpp"

namespace Gui {


SpellList::SpellList(
    unsigned symbolButtonIndex,
    glm::vec2 pos,
    const Icons& icons)
:
    Button{
        pos,
        std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)) + glm::vec2{3, 2},
        Color::infoBackground,
        Color::frameMaroon,
        Color::frameMaroon
    },
    mSymbolImage{
        Graphics::DrawMode::Sprite,
        std::get<Graphics::SpriteSheetIndex>(icons.GetButton(symbolButtonIndex)),
        std::get<Graphics::TextureIndex>(icons.GetButton(symbolButtonIndex)),
        Graphics::ColorMode::Texture,
        Color::black,
        glm::vec2{2, 1},
        std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)),
        true},
    mSpells{
        glm::vec2{
            std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)).x + 5,
            2},
        glm::vec2{260, std::get<glm::vec2>(icons.GetButton(symbolButtonIndex)).y - 2}}
{
    AddChildBack(&mSymbolImage);
    AddChildBack(&mSpells);
}

void SpellList::SetSpells(const Font& font, std::string text)
{
    mSpells.SetText(font, text, false, true, true);
}

SpellsScreen::SpellsScreen(
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
    mSelectedCharacter{0}
{
    const auto& [_, __, dims] = icons.GetButton(36);
    glm::vec2 pos{4,8};
    mSpellLists.reserve(6);
    for (unsigned i = 0; i < 6; i++)
    {
        mSpellLists.emplace_back(
            sSpellSymbolIndexToButtonIndex[i],
            pos,
            icons);
        pos += glm::vec2{0, dims.y + 3};
    }

    AddChildren();
}

void SpellsScreen::SetSelectedCharacter(BAK::ActiveCharIndex character)
{
    mSelectedCharacter = character;
    assert(mGameState.GetParty().GetCharacter(character).IsSpellcaster());

    const auto& characterSpells = mGameState.GetParty().GetCharacter(character).GetSpells();
    const auto& spellDb = BAK::SpellDatabase::Get();
    const auto& symbols = spellDb.GetSymbols();
    for (unsigned i = 0; i < 6; i++)
    {
        std::stringstream spellText{};
        std::string sep{""};
        for (const auto& spell : symbols[i].GetSymbolSlots())
        {
            if (characterSpells.HaveSpell(spell.mSpell))
            {
                spellText << sep << spellDb.GetSpellName(spell.mSpell);
                sep = ", ";
            }
        }
        mSpellLists[i].SetSpells(mFont, spellText.str());
    }
}

bool SpellsScreen::OnMouseEvent(const MouseEvent& event)
{
    return std::visit(overloaded{
        [this](const LeftMousePress& p){ Exit(); return true; },
        [this](const RightMousePress& p){ Exit(); return true; },
        [](const auto& p){ return false; }
        },
        event);
}

void SpellsScreen::Exit()
{
    mGuiManager.DoFade(0.5, [this]{ mGuiManager.ExitSimpleScreen(); });
}

void SpellsScreen::AddChildren()
{
    for (auto& spellList : mSpellLists)
    {
        AddChildBack(&spellList);
    }
}
}
