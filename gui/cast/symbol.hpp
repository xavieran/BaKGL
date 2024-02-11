#pragma once 

#include "bak/spells.hpp"

#include "gui/fontManager.hpp"
#include "gui/core/line.hpp"
#include "gui/core/widget.hpp"

namespace Gui::Cast {

class Symbol : public Widget
{
public:
    Symbol(
        const Font& spellFont)
    :
        Widget{
            RectTag{},
            glm::vec2{0},
            glm::vec2{100,100},
            glm::vec4{},
            true
        },
        mSpellFont{spellFont},
        mLines{}
    {}

    void SetSymbol(unsigned symbolIndex)
    {
        const auto& symbol = BAK::SpellDatabase::Get().GetSymbols()[symbolIndex - 1];
        mSpells.clear();
        mSpells.reserve(symbol.GetSymbolSlots().size());
        for (const auto& slot : symbol.GetSymbolSlots())
        {
            const auto icon = slot.mSpellIcon;
            mSpells.emplace_back(Widget{
                Graphics::DrawMode::Sprite,
                mSpellFont.GetSpriteSheet(),
                static_cast<Graphics::TextureIndex>(
                    mSpellFont.GetFont().GetIndex(icon)),
                Graphics::ColorMode::Texture,
                glm::vec4{1.3f, 0.f, 0.f, 1.f},
                slot.mPosition,
                glm::vec2{mSpellFont.GetFont().GetWidth(icon), mSpellFont.GetFont().GetHeight()},
                true});
        }

        AddChildren();
    }

private:

    void AddChildren()
    {
        ClearChildren();
        for (auto& spell : mSpells)
        {
            AddChildBack(&spell);
        }
    }

    const Font& mSpellFont;
    std::vector<Widget> mSpells;
    std::vector<Line> mLines;
};

}
