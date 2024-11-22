#pragma once

#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"

#include <iostream>
#include <optional>
#include <vector>

namespace BAK {

class FileBuffer;
enum class ItemType;
enum class SkillType;

class Inventory
{
public:
    Inventory(
        unsigned capacity);

    Inventory(
        unsigned capacity,
        std::vector<InventoryItem>&& items);

    Inventory(Inventory&&) noexcept                 = default;
    Inventory& operator=(Inventory&&) noexcept      = default;
    Inventory(const Inventory&) noexcept            = default;
    Inventory& operator=(const Inventory&) noexcept = default;

    const std::vector<InventoryItem>& GetItems() const;
    std::vector<InventoryItem>& GetItems();

    std::size_t GetCapacity() const;
    std::size_t GetNumberItems() const;
    std::size_t GetSpaceUsed() const;

    const InventoryItem& GetAtIndex(InventoryIndex i) const;
    InventoryItem& GetAtIndex(InventoryIndex i);

    std::vector<InventoryItem>::const_iterator FindItem(const InventoryItem& item) const;
    std::vector<InventoryItem>::iterator FindItem(const InventoryItem& item);

    std::optional<InventoryIndex> GetIndexFromIt(std::vector<InventoryItem>::iterator it);
    std::optional<InventoryIndex> GetIndexFromIt(std::vector<InventoryItem>::const_iterator it) const;

    // Search for a stackable item prioritising incomplete stacks
    std::vector<InventoryItem>::const_iterator FindStack(const InventoryItem& item) const;
    // Search for a stackable item prioritising incomplete stacks
    std::vector<InventoryItem>::iterator FindStack(const InventoryItem& item);

    std::vector<InventoryItem>::const_iterator FindEquipped(BAK::ItemType slot) const;
    std::vector<InventoryItem>::iterator FindEquipped(BAK::ItemType slot);

    std::vector<InventoryItem>::const_iterator FindItemType(BAK::ItemType slot) const;
    std::vector<InventoryItem>::iterator FindItemType(BAK::ItemType slot);

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

    unsigned CalculateModifiers(SkillType skill) const;

    void CopyFrom(Inventory& other);
    
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
