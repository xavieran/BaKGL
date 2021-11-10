#pragma once

#include "bak/IContainer.hpp"

#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include <ostream>

namespace BAK {

class KeyContainer : public IContainer
{
public:
    KeyContainer(Inventory&&);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanAddItem(const InventoryItem& item) const override
    {
        return item.GetObject().mType == ItemType::Key;
    }

    bool GiveItem(const InventoryItem& item) override
    {
        ASSERT(item.GetQuantity() == 1);
        ASSERT(item.GetObject().mType == ItemType::Key);
        auto it = mInventory.FindItem(item);
        if (it != mInventory.GetItems().end())
            it->mCondition++;
        else
            mInventory.AddItem(item);
        return true;
    }

    bool RemoveItem(const InventoryItem& item) override
    {
        auto it = mInventory.FindItem(item);
        if (it != mInventory.GetItems().end())
        {
            ASSERT(it->GetQuantity() > 0);
            it->mCondition--;
            if (it->GetQuantity() == 0)
            {
                mInventory.RemoveItem(
                    InventoryIndex{static_cast<unsigned>(
                        std::distance(
                            mInventory.GetItems().begin(),it))});
            }
        }
        else
        {
            ASSERT(it != mInventory.GetItems().end());
        }
        return true;
    }

    ContainerType GetContainerType() const override
    {
        return ContainerType::Key;
    }

    const ShopStats& GetShopData() const override { ASSERT(false); }
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const KeyContainer& i);

}
