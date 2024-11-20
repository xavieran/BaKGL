#pragma once 

#include "bak/types.hpp"

#include "gui/core/line.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

namespace BAK {
class GameState;
struct SymbolSlot;
}

namespace Gui {
class Font;
class Icons;
}

namespace Gui::Cast {

namespace detail {

class Icon : public Widget
{
public:
    Icon(
        unsigned icon,
        glm::vec2 pos,
        const Font& font,
        std::function<void(bool)>&& selected);

    void SetHighlighted();
public:
    void Entered();
    void Exited();

    std::function<void(bool)> mCallback;
};

}

class Symbol : public Widget
{
public:
    Symbol(
        const Font& spellFont,
        BAK::GameState& gameState,
        std::function<void(BAK::SpellIndex)> clickedCallback,
        std::function<void(BAK::SpellIndex, bool)> highlightCallback);

    void Hide();
    void SetActiveCharacter(BAK::ActiveCharIndex character);
    void SetSymbol(unsigned symbolIndex);
    bool OnMouseEvent(const MouseEvent& event) override;
    const std::vector<BAK::SymbolSlot>& GetSpells();
    unsigned GetSymbolIndex() const;
private:
    void AddChildren();

    using SpellIcon = Highlightable<
        Clickable<
            detail::Icon,
            LeftMousePress,
            std::function<void()>>,
        true>;

    BAK::GameState& mGameState;

    const Font& mSpellFont;

    BAK::ActiveCharIndex mSelectedCharacter;

    std::vector<SpellIcon> mSpells;
    std::vector<Line> mLines;

    std::function<void(BAK::SpellIndex)> mClickedCallback;
    std::function<void(BAK::SpellIndex, bool)> mHighlightCallback;
    unsigned mSymbolIndex;
    bool mHidden;
};

}
