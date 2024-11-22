#pragma once

#include "bak/types.hpp"

#include "gui/inventory/inventorySlot.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"

#include <glm/glm.hpp>

namespace BAK {
class InventoryItem;
}

namespace Gui {
class Icons;
class Font;

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
        bool available,
        std::function<void()>&& showItemDescription);
    
    bool GetAvailable() const;
    void UpdateDescription(
        const Font& font,
        const BAK::InventoryItem& item);
private:
    void AddChildren();
    
    bool mAvailable;
    BAK::Royals mSellPrice;
    TextBox mDescription;
};

}
