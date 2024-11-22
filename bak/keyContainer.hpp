#pragma once

#include "bak/IContainer.hpp"

#include "bak/inventory.hpp"

#include <ostream>

namespace BAK {

class KeyContainer : public IContainer
{
public:
    KeyContainer(Inventory&&);
 
    Inventory& GetInventory() override;
    const Inventory& GetInventory() const override;

    bool CanAddItem(const InventoryItem& item) const override;
    bool GiveItem(const InventoryItem& item) override;
    bool RemoveItem(const InventoryItem& item) override;
    ContainerType GetContainerType() const override;

    ShopStats& GetShop() override;
    const ShopStats& GetShop() const override;
    LockStats& GetLock() override;

    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const KeyContainer& i);

}
