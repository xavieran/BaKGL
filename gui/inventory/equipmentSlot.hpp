#pragma once

#include "gui/inventory/inventorySlot.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>


namespace Gui {

class EquipmentSlot : public Widget
{
public:
    EquipmentSlot(
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

    void PropagateUp(const DragEvent& event)
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

    template <typename ...Args>
    void AddItem(Args&&... args)
    {
        ClearChildren();
        mItem.emplace(std::forward<Args>(args)...);
        AddChildBack(&(*mItem));
    }

    void ClearItem()
    {
        ClearChildren();
        AddChildBack(&mBlank);
    }

    bool HasItem() const
    {
        return bool{mItem};
    }

    InventorySlot& GetInventorySlot()
    {
        return *mItem;
    }

private:
    std::optional<DraggableItem> mItem;
    Widget mBlank;
};


}
