#pragma once

#include "bak/inventoryItem.hpp"
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
    const auto& GetAtIndex(unsigned i) const
    {
        ASSERT(i < mItems.size());
        return mItems[i];
    }
    auto& GetAtIndex(unsigned i)
    {
        ASSERT(i < mItems.size());
        return mItems[i];
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
