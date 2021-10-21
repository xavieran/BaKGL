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

    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const KeyContainer& i);

}
