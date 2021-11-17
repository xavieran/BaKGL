#include "bak/inventoryItem.hpp"

#include "com/assert.hpp"
#include "com/ostream.hpp"

#include <iostream>

namespace BAK {

bool CheckItemStatus(std::uint8_t status, ItemStatus flag)
{
    return CheckBitSet(status, flag);
}

std::uint8_t SetItemStatus(std::uint8_t status, ItemStatus flag, bool state)
{
    return SetBit(status, flag, state);
}

InventoryItem::InventoryItem(
    GameObject const* object,
    ItemIndex itemIndex,
    unsigned condition,
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
        +i.mCondition << " status: " << +i.mStatus << " mods: [" << i.GetModifiers() 
        << "] IsEquipped: " << i.IsEquipped() << "\n";
    return os;
}

}
