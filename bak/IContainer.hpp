#pragma once

#include "inventory.hpp"
#include "inventoryItem.hpp"

namespace BAK {

class IContainer
{
public:
    virtual const Inventory& GetInventory() const = 0;
    virtual Inventory& GetInventory() = 0;
};

}
