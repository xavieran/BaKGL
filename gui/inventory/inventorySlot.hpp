#pragma once

#include "bak/inventoryItem.hpp"
#include "bak/types.hpp"

#include "gui/inventory/modifiers.hpp"

#include "gui/core/dragEndpoint.hpp"
#include "gui/core/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class InventoryItem;
}

namespace Gui {
class Font;
class Icons;

class InventorySlot;

using DraggableItem = Draggable<InventorySlot>;

template <typename Base>
using ItemEndpoint = DragEndpoint<
    Base,
    InventorySlot>;

class InventorySlot :
    public Widget
{
public:
    InventorySlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const Icons& icons,
        BAK::InventoryIndex itemIndex,
        const BAK::InventoryItem& item,
        std::function<void()>&& useItemDirectly,
        std::function<void()>&& showItemDescription);
    
    BAK::InventoryIndex GetItemIndex() const;
    const BAK::InventoryItem& GetItem() const;
    bool OnMouseEvent(const MouseEvent& event) override;
    bool LeftMousePressed(glm::vec2 click);
    bool RightMousePressed(glm::vec2 click);
    void UpdateSelected();
    void UpdateQuantity(
        const Font& font,
        const BAK::InventoryItem& item);
    bool IsSelected() const;
    void ResetSelected();
protected:
    void AddItem(bool snapToTop);
private:
    void AddChildren();
    
    const BAK::InventoryIndex mItemIndex;
    const BAK::InventoryItem& mItemRef;
    std::function<void()> mUseItemDirectly;
    std::function<void()> mShowItemDescription;
    bool mIsSelected;

    TextBox mQuantity;
    Widget mItem;
    Modifiers mModifiers;
};

}
