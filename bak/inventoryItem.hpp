#pragma once

#include "bak/objectInfo.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace BAK {

enum class ItemStatus : std::uint8_t
{
    Equipped   = 6,
    Repairable = 7,
};

bool CheckItemStatus(std::uint8_t status, ItemStatus flag);

class InventoryItem
{
public:
    InventoryItem(
        GameObject const* object,
        ItemIndex itemIndex,
        std::uint8_t condition,
        std::uint8_t status,
        std::uint8_t modifiers);

    const GameObject& GetObject() const { ASSERT(mObject); return *mObject; }

    bool IsEquipped() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Equipped);
    }

    bool IsStackable() const
    {
        return (0x0800 & GetObject().mFlags) == 0x0800;
    }

    GameObject const* mObject;
    ItemIndex mItemIndex;
    std::uint8_t mCondition;
    std::uint8_t mStatus;
    std::uint8_t mModifiers;


};

std::ostream& operator<<(std::ostream&, const InventoryItem&);

class InventoryItemFactory
{
public:
    static InventoryItem MakeItem(
        ItemIndex itemIndex,
        std::uint8_t quantity,
        std::uint8_t status,
        std::uint8_t modifiers)
    {
        static ObjectIndex objects{};

        return InventoryItem{
            &objects.GetObject(itemIndex),
            itemIndex,
            quantity,
            status,
            modifiers};
    }

    static InventoryItem MakeItem(
        ItemIndex itemIndex,
        std::uint8_t quantity)
    {
        return MakeItem(
            itemIndex,
            quantity,
            0,
            0);
    }
};

}
