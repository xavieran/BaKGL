#pragma once

#include "gui/inventory/inventorySlot.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {
class Icons;

class EquipmentSlot : public Widget
{
public:
    EquipmentSlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& mIcons,
        int icon);
    
    template <typename ...Args>
    void AddItem(Args&&... args)
    {
        ClearChildren();
        mItem.emplace(std::forward<Args>(args)...);
        AddChildBack(&(*mItem));
    }

    void PropagateUp(const DragEvent& event);
    void ClearItem();
    bool HasItem() const;
    InventorySlot& GetInventorySlot();

private:
    std::optional<DraggableItem> mItem;
    Widget mBlank;
};


}
