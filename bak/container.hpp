#pragma once

#include "bak/IContainer.hpp"

#include "bak/dialog.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <ostream>

namespace BAK {

class Container : public IContainer
{
public:
    Container(
        unsigned address,
        unsigned number,
        unsigned numberItems,
        unsigned capacity,
        unsigned type,
        Target dialog,
        glm::vec<2, unsigned> location,
        std::vector<InventoryItem>&& items);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    Target mDialog;
    glm::vec<2, unsigned> mLocation;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const Container& i);

}
