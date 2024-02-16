#pragma once 

#include "bak/gameState.hpp"
#include "bak/spells.hpp"

#include "gui/fontManager.hpp"
#include "gui/core/line.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

namespace Gui::Cast {

namespace detail {

class Icon : public Widget
{
public:
    Icon(
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

    void SetHighlighted()
    {
        SetColorMode(Graphics::ColorMode::ReplaceColor);
    }

public:
    void Entered()
    {
        SetColorMode(Graphics::ColorMode::TintColor);
        mCallback(true);
    }

    void Exited()
    {
        SetColorMode(Graphics::ColorMode::Texture);
        mCallback(false);
    }

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

    void Hide()
    {
        mHidden = true;
        AddChildren();
    }

    void SetActiveCharacter(BAK::ActiveCharIndex character)
    {
        mSelectedCharacter = character;
    }

    void SetSymbol(unsigned symbolIndex)
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

    bool OnMouseEvent(const MouseEvent& event) override
    {
        bool handled = false;
        for (auto& widget : mSpells)
        {
            handled |= widget.OnMouseEvent(event);
        }

        return handled;
    }

    const auto& GetSpells()
    {
        return BAK::SpellDatabase::Get().GetSymbols()[mSymbolIndex - 1].GetSymbolSlots();
    }

    unsigned GetSymbolIndex() const
    {
        return mSymbolIndex;
    }
private:
    void AddChildren()
    {
        ClearChildren();
        if (mHidden) return;

        for (auto& spell : mSpells)
        {
            AddChildBack(&spell);
        }
    }

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
