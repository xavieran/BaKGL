#include "gui/cast/symbol.hpp"

#include "bak/gameState.hpp"
#include "bak/types.hpp"

#include "gui/fontManager.hpp"
#include "gui/icons.hpp"

namespace Gui::Cast {

namespace detail {

Icon::Icon(
    unsigned icon,
    glm::vec2 pos,
    const Font& font,
    std::function<void(bool)>&& selected)
:
    Widget{
        Graphics::DrawMode::Sprite,
        font.GetSpriteSheet(),
        static_cast<Graphics::TextureIndex>(
            font.GetFont().GetIndex(icon + 1)),
        Graphics::ColorMode::Texture,
        glm::vec4{1.0f, 0.f, 0.f, 1.f},
        pos,
        glm::vec2{font.GetFont().GetWidth(icon), font.GetFont().GetHeight()},
        true},
    mCallback{selected}
{
}

void Icon::SetHighlighted()
{
    SetColorMode(Graphics::ColorMode::ReplaceColor);
}

void Icon::Entered()
{
    SetColorMode(Graphics::ColorMode::TintColor);
    mCallback(true);
}

void Icon::Exited()
{
    SetColorMode(Graphics::ColorMode::Texture);
    mCallback(false);
}

}

Symbol::Symbol(
    const Font& spellFont,
    BAK::GameState& gameState,
    std::function<void(BAK::SpellIndex)> clickedCallback,
    std::function<void(BAK::SpellIndex, bool)> highlightCallback)
:
    Widget{
        RectTag{},
        glm::vec2{-4, -4},
        glm::vec2{100,100},
        glm::vec4{},
        true
    },
    mGameState{gameState},
    mSpellFont{spellFont},
    mLines{},
    mClickedCallback{std::move(clickedCallback)},
    mHighlightCallback{std::move(highlightCallback)},
    mSymbolIndex{0},
    mHidden{false}
{
}

void Symbol::Hide()
{
    mHidden = true;
    AddChildren();
}

void Symbol::SetActiveCharacter(BAK::ActiveCharIndex character)
{
    mSelectedCharacter = character;
}

void Symbol::SetSymbol(unsigned symbolIndex)
{
    mHidden = false;
    mSymbolIndex = symbolIndex;
    const auto& symbol = BAK::SpellDatabase::Get().GetSymbols()[mSymbolIndex - 1];
    mSpells.clear();
    mSpells.reserve(symbol.GetSymbolSlots().size());
    for (const auto& slot : symbol.GetSymbolSlots())
    {
        const auto icon = slot.mSpellIcon;
        const auto spellIndex = slot.mSpell;

        if (!mGameState.CanCastSpell(spellIndex, mSelectedCharacter)) continue;

        mSpells.emplace_back(
            [this, spellIndex=spellIndex]{ mClickedCallback(spellIndex); },
            icon,
            slot.mPosition,
            mSpellFont,
            [this, spellIndex=spellIndex](bool selected){ mHighlightCallback(spellIndex, selected); });
    }

    AddChildren();
}

bool Symbol::OnMouseEvent(const MouseEvent& event)
{
    bool handled = false;
    for (auto& widget : mSpells)
    {
        handled |= widget.OnMouseEvent(event);
    }

    return handled;
}

const std::vector<BAK::SymbolSlot>& Symbol::GetSpells()
{
    return BAK::SpellDatabase::Get().GetSymbols()[mSymbolIndex - 1].GetSymbolSlots();
}

unsigned Symbol::GetSymbolIndex() const
{
    return mSymbolIndex;
}

void Symbol::AddChildren()
{
    ClearChildren();
    if (mHidden) return;

    for (auto& spell : mSpells)
    {
        AddChildBack(&spell);
    }
}
}
