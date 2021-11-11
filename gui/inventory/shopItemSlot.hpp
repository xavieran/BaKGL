#pragma once

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/inventory/inventorySlot.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class ShopItemSlot;

using DraggableShopItem = Draggable<ShopItemSlot>;

class ShopItemSlot :
    public InventorySlot
{
public:
    ShopItemSlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const Icons& icons,
        BAK::InventoryIndex itemIndex,
        const BAK::InventoryItem& item,
        BAK::Royals sellPrice,
        std::function<void()>&& showItemDescription)
    :
        InventorySlot{
            pos,
            dims,
            font,
            icons,
            itemIndex,
            item,
            std::move(showItemDescription)
        },
        mSellPrice{sellPrice},
        mDescription{
            glm::vec2{0, 0},
            glm::vec2{84, 50}
        }
    {
        ClearChildren();
        UpdateDescription(font, item);
        AddChildren();
    }

    void UpdateDescription(
        const Font& font,
        const BAK::InventoryItem& item)
    {
        ASSERT(item.GetObject().mValue >= 0);

        std::stringstream ss{};
        ss << "#" << item.GetObject().mName;
        if (item.IsStackable() || item.IsChargeBased() || item.IsNumberBased())
            ss << " ("  << +item.mCondition << ")";
        else if (item.IsConditionBased())
            ss << " ("  << +item.mCondition << "%)";

        ss << "\n#" << ToShopString(mSellPrice);

        // First calculate text dims, trim the textbox to that size,
        // then add the text again, centered
        const auto& [textDims, _] = mDescription.AddText(font, ss.str());
        mDescription.SetDimensions(textDims);
        mDescription.AddText(font, ss.str(), true);

        const auto& dims = GetPositionInfo().mDimensions;
        auto diff = dims - textDims;
        if (diff.x < 0) diff.x = 0;
        else diff.x = diff.x / 2;

        mDescription.SetPosition(
            glm::vec2{diff.x, diff.y} + glm::vec2{4, 2});
    }

private:
    void AddChildren()
    {
        ClearChildren();
        AddItem(true);
        AddChildBack(&mDescription);
    }
    
    BAK::Royals mSellPrice;
    TextBox mDescription;
};

}
