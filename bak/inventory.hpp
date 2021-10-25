#pragma once

#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/strongType.hpp"
#include "com/logger.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace BAK {

using InventoryIndex = StrongType<unsigned, struct InventoryIndexTag>;

class Inventory
{
public:
    Inventory(
        unsigned capacity)
    :
        mCapacity{capacity},
        mItems{}
    {}

    Inventory(
        unsigned capacity,
        std::vector<InventoryItem>&& items)
    :
        mCapacity{capacity},
        mItems{std::move(items)}
    {}


    Inventory(Inventory&&) = default;
    Inventory& operator=(Inventory&&) = default;
    Inventory(const Inventory&) = default;
    Inventory& operator=(const Inventory&) = default;

    const auto& GetItems() const { return mItems; }
    auto& GetItems() { return mItems; }

    std::size_t GetNumberItems() const { return mItems.size(); }
    std::size_t GetSpaceUsed() const;
    

    const auto& GetAtIndex(InventoryIndex i) const
    {
        ASSERT(i.mValue < mItems.size());
        return mItems[i.mValue];
    }

    auto& GetAtIndex(InventoryIndex i)
    {
        ASSERT(i.mValue < mItems.size());
        return mItems[i.mValue];
    }

    auto FindItem(const InventoryItem& item) const
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                return elem.mItemIndex == item.mItemIndex;
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

    // Search for a stackable item prioritising incomplete stacks
    auto FindStack(const InventoryItem& item) const
    {
        ASSERT(item.IsStackable());
        auto it = std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                const auto stackSize = elem.GetObject().mStackSize;
                return (elem.mItemIndex == item.mItemIndex)
                    && (elem.mCondition != elem.GetObject().mStackSize);
            });

        // If we didn't find an incomplete stack, return a complete one
        if (it == mItems.end())
            return FindItem(item);
        else
            return it;
    }

    // Search for a stackable item prioritising incomplete stacks
    auto FindStack(const InventoryItem& item)
    {
        ASSERT(item.IsStackable());
        auto it = std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                const auto stackSize = elem.GetObject().mStackSize;
                return (elem.mItemIndex == item.mItemIndex)
                    && (elem.mCondition != elem.GetObject().mStackSize);
            });
        if (it == mItems.end())
        {
            return FindItem(item);
        }
        else
        {
            return it;
        }
    }

    auto FindEquipped(BAK::ItemType slot) const
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&slot](const auto& elem){
                return elem.GetObject().mType == slot
                    && elem.IsEquipped();
            });
    }

    auto FindEquipped(BAK::ItemType slot)
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&slot](const auto& elem){
                return elem.GetObject().mType == slot
                    && elem.IsEquipped();
            });
    }
    
    bool HasIncompleteStack(const InventoryItem& item) const;

    // Characters can be added up to the size of the inventory
    // accounting for the size of items
    std::size_t CanAddCharacter(const InventoryItem& item) const;
    // Containers can be added up to their capacity
    std::size_t CanAddContainer(const InventoryItem& item) const;

    bool HaveItem(const InventoryItem& item) const;

    // Adds the given item with no checks
    void AddItem(const InventoryItem& item);
    bool RemoveItem(const InventoryItem& item);
    bool RemoveItem(BAK::InventoryIndex item);

    void CheckPostConditions();
    
private:
    // result > 0 if can add item to inventory.
    // for stackable items result amount is how 
    // much of this item can be added to inventory.
    std::size_t CanAdd(bool fits, const InventoryItem& item) const;

    unsigned mCapacity;
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);
}
