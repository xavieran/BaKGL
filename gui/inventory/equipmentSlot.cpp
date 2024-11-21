#include "gui/inventory/equipmentSlot.hpp"

#include "gui/inventory/inventorySlot.hpp"
#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

EquipmentSlot::EquipmentSlot(
    glm::vec2 pos,
    glm::vec2 dims,
    const Icons& mIcons,
    int icon)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{},
        true
    },
    mBlank{
        ImageTag{},
        std::get<Graphics::SpriteSheetIndex>(
            mIcons.GetInventoryIcon(icon)),
        std::get<Graphics::TextureIndex>(
            mIcons.GetInventoryIcon(icon)),
        glm::vec2{0},
        dims,
        true
    }
{
    ClearItem();
}

void EquipmentSlot::PropagateUp(const DragEvent& event)
{
    // Display blank slot image when you lift the equipment off the slot
    evaluate_if<DragStarted>(event, [&](const auto&){
            AddChildFront(&mBlank);
        });

    evaluate_if<DragEnded>(event, [&](const auto&){
            RemoveChild(&mBlank);
        });

    Widget::PropagateUp(event);
}

void EquipmentSlot::ClearItem()
{
    ClearChildren();
    AddChildBack(&mBlank);
}

bool EquipmentSlot::HasItem() const
{
    return bool{mItem};
}

InventorySlot& EquipmentSlot::GetInventorySlot()
{
    return *mItem;
}

}
