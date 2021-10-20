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

    bool IsEquipped() const
    {
        // FIXME: Not correct
        return mStatus && (1 << 6) == (1 << 6);
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


class Inventory
{
public:
    static constexpr auto sMaxInventorySize = 20;

    Inventory()
    :
        mItems{}
    {}

    Inventory(Inventory&&) = default;
    Inventory& operator=(Inventory&&) = default;
    Inventory(const Inventory&) = default;
    Inventory& operator=(const Inventory&) = default;

    Inventory(std::vector<InventoryItem>&& items)
    :
        mItems{std::move(items)}
    {}

    const auto& GetItems() const { return mItems; }

    auto FindIncompleteStack(const InventoryItem& item)
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                const auto stackSize = elem.GetObject().mStackSize;
                return (elem.mItemIndex == item.mItemIndex)
                    && (elem.mCondition != elem.GetObject().mStackSize);
            });
    }

    auto FindItem(const InventoryItem& item)
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                return elem.mItemIndex == item.mItemIndex;
            });
    }
    
    bool HasIncompleteStack(const InventoryItem& item) const;
    // FIXME: probably want this to return 0 if cant add and the amount
    // it can add if stackable and have an empty stack
    bool CanAdd(const InventoryItem& item) const;
    bool HaveWeaponEquipped() const;
    bool HaveItem(const InventoryItem& item) const;

    // Adds the given item with no checks
    void AddItem(const InventoryItem& item);
    bool RemoveItem(const InventoryItem& item);
    bool RemoveItem(unsigned item);
    
private:
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);
}
