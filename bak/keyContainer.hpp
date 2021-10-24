#pragma once

#include "bak/IContainer.hpp"

#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include <ostream>

namespace BAK {

class KeyContainer : public IContainer
{
public:
    KeyContainer(
        Inventory&&);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanAddItem(const InventoryItem& item) override
    {
        return item.GetObject().mType == ItemType::Key;
    }

    bool GiveItem(const InventoryItem&) override
    {
        // FIXME: Implement adding keys...
        return true;
    }

    bool RemoveItem(const InventoryItem& item) override
    {
        mInventory.RemoveItem(item);
        return true;
    }

    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const KeyContainer& i);

}
