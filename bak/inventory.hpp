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
    // FIXME: probably want this to return 0 if cant add and the amount
    // it can add if stackable and have an empty stack
    bool CanAdd(const InventoryItem& item) const;
    bool HaveWeaponEquipped() const;
    bool HaveItem(const InventoryItem& item) const;

    // Adds the given item with no checks
    void AddItem(const InventoryItem& item);
    bool RemoveItem(const InventoryItem& item);
    bool RemoveItem(BAK::InventoryIndex item);
    
private:
    unsigned mCapacity;
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);
}
