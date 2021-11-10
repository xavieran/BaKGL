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
std::uint8_t SetItemStatus(std::uint8_t status, ItemStatus flag, bool state);

class InventoryItem
{
public:
    InventoryItem(
        GameObject const* object,
        ItemIndex itemIndex,
        unsigned condition,
        std::uint8_t status,
        std::uint8_t modifiers);

    const GameObject& GetObject() const { ASSERT(mObject); return *mObject; }

    auto GetQuantity() const { return mCondition; }
    auto GetCondition() const { return mCondition; }

    bool IsEquipped() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Equipped);
    }

    bool IsMoney() const
    {
        return mItemIndex == ItemIndex{0x35}
            || mItemIndex == ItemIndex{0x36};
    }

    bool IsKey() const
    {
        return GetObject().mType == ItemType::Key;
    }

    void SetEquipped(bool state)
    {
        mStatus = SetItemStatus(mStatus, ItemStatus::Equipped, state);
    }

    bool IsConditionBased() const
    {
        return (0x1000 & GetObject().mFlags) == 0x1000;
    }

    bool IsChargeBased() const
    {
        return (0x2000 & GetObject().mFlags) == 0x2000;
    }

    bool IsStackable() const
    {
        return (0x0800 & GetObject().mFlags) == 0x0800;
    }

    GameObject const* mObject;
    ItemIndex mItemIndex;
    unsigned mCondition;
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
