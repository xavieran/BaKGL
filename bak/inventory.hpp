#pragma once

#include "bak/objectInfo.hpp"
#include "bak/types.hpp"

#include <string>
#include <vector>

namespace BAK {


class InventoryItem
{
public:
    InventoryItem(
        const GameObject& object,
        ItemIndex itemIndex,
        std::uint8_t condition,
        std::uint8_t status,
        std::uint8_t modifiers);

    const GameObject& mObject;
    ItemIndex mItemIndex;
    std::uint8_t mCondition;
    std::uint8_t mStatus;
    std::uint8_t mModifiers;
};

std::ostream& operator<<(std::ostream&, const InventoryItem&);

class Inventory
{
public:
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);
}
