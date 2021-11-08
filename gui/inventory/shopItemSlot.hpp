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

using DraggableItem = Draggable<InventorySlot>;

template <typename Base>
using ItemEndpoint = DragEndpoint<
    Base,
    DraggableItem>;

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
        mDescription{
            glm::vec2{0, 0},
            glm::vec2{98, 50}
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
        ss << "#" << item.GetObject().mName << " (" 
            << +item.mCondition << (item.IsStackable() ? ")" : "%)");
        ss << "\n#" << ToString(BAK::Royals{static_cast<unsigned>(item.GetObject().mValue)});
        Logging::LogDebug(__FUNCTION__) << "Desc: " << ss.str() << std::endl;

        const auto& [textDims, _] = mDescription.AddText(
            font,
            ss.str(),
            true,
            false);

        const auto& dims = GetPositionInfo().mDimensions;
        mDescription.SetPosition(
            dims - textDims 
            + glm::vec2{4, 2});
    }

private:
    void AddChildren()
    {
        ClearChildren();
        AddItem();
        AddChildBack(&mDescription);
    }
    
    TextBox mDescription;
};

}
