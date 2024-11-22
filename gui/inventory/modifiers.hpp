#pragma once

#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class InventoryItem;
}

namespace Gui {
class Icons;

class Modifiers :
   public Widget
{
public:
    Modifiers(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const BAK::InventoryItem& item);
    
    void UpdateModifiers();
private:
    void AddChildren();
    
    const Icons& mIcons;
    const BAK::InventoryItem& mItemRef;
    std::vector<Widget> mModifiers;
};

}
