#pragma once

#include "gui/inventory/inventorySlot.hpp"

#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class IContainer;
class InventoryItem;
}

namespace Gui {
class Icons;
class Font;

class ContainerDisplay :
    public Widget
{
public:
    ContainerDisplay(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font,
        std::function<void(const BAK::InventoryItem&)>&& showDescription);

    void SetContainer(BAK::IContainer* container);
    void RefreshGui();
private:

    void ShowItemDescription(const BAK::InventoryItem& item);
    void UpdateInventoryContents();
    void AddChildren();

private:
    const Font& mFont;
    const Icons& mIcons;

    std::vector<DraggableItem> mInventoryItems;

    BAK::IContainer* mContainer;

    std::function<void(const BAK::InventoryItem&)> mShowDescription;
    const Logging::Logger& mLogger;
};

}
