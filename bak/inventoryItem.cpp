#include "bak/inventoryItem.hpp"

#include "com/assert.hpp"

#include <iostream>

namespace BAK {

bool CheckItemStatus(std::uint8_t status, ItemStatus flag)
{
    const auto shifted = 1 << static_cast<std::uint8_t>(flag);
    return (status & shifted) == shifted;
}

std::uint8_t SetItemStatus(std::uint8_t status, ItemStatus flag, bool state)
{
    if (state)
        return status | (1 << static_cast<std::uint8_t>(flag));
    else
        return status & (~(1 << static_cast<std::uint8_t>(flag)));
}

InventoryItem::InventoryItem(
    GameObject const* object,
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
    os << i.GetObject().mName << " #" << i.mItemIndex << " pct/qty: " << 
        +i.mCondition << " status: " << +i.mStatus << " mods: " << +i.mModifiers;
    return os;
}

}
