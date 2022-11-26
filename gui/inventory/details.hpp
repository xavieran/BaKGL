#pragma once

#include "bak/inventoryItem.hpp"

#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>


namespace Gui {

class Details : public Widget
{
public:
    Details(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font)
    :
        Widget{
            RectTag{},
            glm::vec2{15, 11},
            glm::vec2{289, 121},
            Color::black,
            true
        },
        mIcons{icons},
        mFont{font},
        mName{
            glm::vec2{15, 12},
            glm::vec2{100, 60}
        },
        mItem{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(
                icons.GetInventoryIcon(0)),
            std::get<Graphics::TextureIndex>(
                icons.GetInventoryIcon(0)),
            glm::vec2{13, 15 + 29 *2},
            std::get<glm::vec2>(
                icons.GetInventoryIcon(0)),
            false
        },
        mStatusText{
            glm::vec2{10, 80},
            glm::vec2{100, 60},
        },
        mDescriptionText{
            glm::vec2{103, 11},
            glm::vec2{204, 121}
        }
    {
    }

    void AddItem(const BAK::InventoryItem& item)
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryIcon(item.GetObject().mImageIndex);
        mItem.SetSpriteSheet(ss);
        mItem.SetTexture(ti);
        mItem.SetDimensions(dims);
        mItem.SetCenter(GetCenter() - GetTopLeft());

        mDescriptionText.AddText(mFont, "#Item desc");
        mStatusText.AddText(mFont, "#Repairable");

        std::stringstream sss{};
        sss << "#" << item.GetObject().mName;
        mName.AddText(mFont, sss.str());

        AddChildren();
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mName);
        AddChildBack(&mItem);
        AddChildBack(&mStatusText);
        AddChildBack(&mDescriptionText);
    }

private:
    const Icons& mIcons;
    const Font& mFont;
    TextBox mName;
    Widget mItem;
    TextBox mStatusText;
    TextBox mDescriptionText;
};

}
