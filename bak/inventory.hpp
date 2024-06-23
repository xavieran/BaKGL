#pragma once

#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"
#include "bak/skills.hpp"

#include "com/assert.hpp"
#include "com/strongType.hpp"

#include "bak/fileBufferFactory.hpp"

#include <iostream>
#include <optional>
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

    Inventory(Inventory&&) noexcept                 = default;
    Inventory& operator=(Inventory&&) noexcept      = default;
    Inventory(const Inventory&) noexcept            = default;
    Inventory& operator=(const Inventory&) noexcept = default;

    const auto& GetItems() const { return mItems; }
    auto& GetItems() { return mItems; }

    std::size_t GetCapacity() const { return mCapacity; }
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
                return elem.GetItemIndex() == item.GetItemIndex();
            });
    }

    auto FindItem(const InventoryItem& item)
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&item](const auto& elem){
                return elem.GetItemIndex() == item.GetItemIndex();
            });
    }

    std::optional<InventoryIndex> GetIndexFromIt(std::vector<InventoryItem>::iterator it)
    {
        if (it == mItems.end())
            return std::nullopt;
        else
            return std::make_optional(
                static_cast<InventoryIndex>(std::distance(mItems.begin(), it)));
    }

    std::optional<InventoryIndex> GetIndexFromIt(std::vector<InventoryItem>::const_iterator it) const
    {
        if (it == mItems.end())
            return std::nullopt;
        else
            return std::make_optional(
                static_cast<InventoryIndex>(std::distance(mItems.cbegin(), it)));
    }

    // Search for a stackable item prioritising incomplete stacks
    auto FindStack(const InventoryItem& item) const
    {
        // This is unpleasant, the dream would be to use
        // std::views::zip_view (C++23) with std::views::filter... 
        ASSERT(item.IsStackable() || item.IsChargeBased());
        auto items = std::vector<
            std::pair<
                std::size_t,
                std::reference_wrapper<const InventoryItem>>>{};

        for (std::size_t i = 0; i < mItems.size(); i++)
        {
            if (mItems[i].GetItemIndex() == item.GetItemIndex())
                items.emplace_back(i, std::ref(mItems[i]));
        }

        auto it = std::min_element(items.begin(), items.end(),
            [](const auto& l, const auto& r){
                return (std::get<1>(l).get().GetQuantity() < std::get<1>(r).get().GetQuantity());
            });

        // If we didn't find an incomplete stack, return a complete one
        if (it == items.end())
            return FindItem(item);
        else
            return std::next(mItems.begin(), it->first);
    }

    // Search for a stackable item prioritising incomplete stacks
    auto FindStack(const InventoryItem& item)
    {
        // Is there a better way?
        auto cit = static_cast<const Inventory*>(this)->FindStack(item);
        return std::next(
            mItems.begin(),
            std::distance(mItems.cbegin(), cit));
    }

    auto FindEquipped(BAK::ItemType slot) const
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&slot](const auto& elem){
                return elem.IsItemType(slot) && elem.IsEquipped();
            });
    }

    auto FindEquipped(BAK::ItemType slot)
    {
        return std::find_if(
            mItems.begin(), mItems.end(),
            [&slot](const auto& elem){
                return elem.IsItemType(slot) && elem.IsEquipped();
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
    bool RemoveItem(BAK::InventoryIndex);
    bool RemoveItem(BAK::InventoryIndex, unsigned quantity);
    bool ReplaceItem(BAK::InventoryIndex, BAK::InventoryItem);

    void CheckPostConditions();

    unsigned CalculateModifiers(SkillType skill) const
    {
        unsigned mods = 0;
        for (const auto& item : mItems)
        {
            if ((item.GetObject().mModifierMask 
                & (1 << static_cast<unsigned>(skill))) != 0)
            {
                mods += item.GetObject().mModifier;
            }
        }
        return mods;
    }

    void CopyFrom(Inventory& other)
    {
        for (const auto& item : other.GetItems())
        {
            auto newItem = item;
            newItem.SetActivated(false);
            newItem.SetEquipped(false);
            AddItem(item);
        }
    }
    
private:
    // result > 0 if can add item to inventory.
    // for stackable items result amount is how 
    // much of this item can be added to inventory.
    std::size_t CanAdd(bool fits, const InventoryItem& item) const;

    unsigned mCapacity;
    std::vector<InventoryItem> mItems;
};

std::ostream& operator<<(std::ostream&, const Inventory&);

Inventory LoadInventory(
    FileBuffer& fb,
    unsigned itemCount,
    unsigned capacity);

}
