#pragma once

#include "bak/objectInfo.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace BAK {

enum class ItemStatus
{
    Equipped   = 6,
    Repairable = 7,
};

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

    GameObject const* mObject;
    ItemIndex mItemIndex;
    std::uint8_t mCondition;
    std::uint8_t mStatus;
    std::uint8_t mModifiers;

    bool IsEquipped() const
    {
        // FIXME: Not correct
        return mStatus && (1 << 6) == (1 << 6);
    }
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


class Inventory
{
public:
    static constexpr auto sMaxInventorySize = 20;

    bool CanAdd(const GameObject& object) const
    {
        const auto currentQuantity = std::accumulate(
            mItems.begin(), mItems.end(),
            0,
            [](const auto sum, const auto& elem) -> unsigned {
                if (!elem.IsEquipped())
                    return sum + elem.GetObject().mImageSize;
                return sum;
            });
        return (currentQuantity + object.mImageSize) < sMaxInventorySize;
    }

    bool HaveWeaponEquipped() const
    {
        for (const auto& item : mItems)
        {
            const auto itemType = item.GetObject().mType;
            if ((itemType == BAK::ItemType::Sword
                || itemType == BAK::ItemType::Staff)
                && item.IsEquipped())
                return true;
        }
        return false;
    }

    void AddItem(InventoryItem&& item)
    {
        mItems.emplace_back(std::move(item));
    }

    const auto& GetItems() const { return mItems; }
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);
}
