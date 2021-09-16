#include "bak/inventory.hpp"

namespace BAK {


InventoryItem::InventoryItem(
    const GameObject& object,
    ItemIndex itemIndex,
    std::uint8_t condition,
    std::uint8_t status,
    std::uint8_t modifiers)
:
    mObject{object},
    mItemIndex{itemIndex},
    mCondition{condition},
    mStatus{status},
    mModifiers{modifiers}
{}

std::ostream& operator<<(std::ostream& os, const InventoryItem& i)
{
    os << i.mObject.mName << " #" << i.mItemIndex << " pct/qty: " << 
        +i.mCondition << " status: " << +i.mStatus << " mods: " << +i.mModifiers;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inventory)
{
    for (unsigned i = 0; i < inventory.mItems.size(); i++)
    {
        os << i << " >> " << inventory.mItems[i] << "\n";
    }
    return os;
}

}
