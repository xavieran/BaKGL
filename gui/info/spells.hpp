#pragma once

#include "bak/types.hpp"

#include "gui/button.hpp"
#include "gui/textBox.hpp"

namespace BAK {
class GameState;
}

namespace Gui {
class IGuiManager;
class Backgrounds;
class Font;
class Icons;

class SpellList : public Button
{
public:
    SpellList(
        unsigned symbolButtonIndex,
        glm::vec2 pos,
        const Icons& icons);
    
    void SetSpells(const Font& font, std::string text);

private:
    Widget mSymbolImage;
    TextBox mSpells;
};

class SpellsScreen : public Widget
{
    static constexpr std::array<unsigned, 6> sSpellSymbolIndexToButtonIndex = 
    {
        37, 36, 38, 39, 56, 55
    };

public:
    SpellsScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);
   
    void SetSelectedCharacter(BAK::ActiveCharIndex character);

    bool OnMouseEvent(const MouseEvent& event) override;
private:
    void Exit();
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    BAK::ActiveCharIndex mSelectedCharacter;

    std::vector<SpellList> mSpellLists;

};

}
