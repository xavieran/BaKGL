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
        ASSERT(item.IsKey());
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
            it->SetQuantity(it->GetQuantity() - 1);
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
            return false;
        }

        return true;
    }

    ContainerType GetContainerType() const override
    {
        return ContainerType::Key;
    }

    ShopStats& GetShop() override { ASSERT(false); return *reinterpret_cast<ShopStats*>(this);}
    const ShopStats& GetShop() const override { ASSERT(false); return *reinterpret_cast<const ShopStats*>(this);}
    LockStats& GetLock() override { ASSERT(false); return *reinterpret_cast<LockStats*>(this); }
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const KeyContainer& i);

}
