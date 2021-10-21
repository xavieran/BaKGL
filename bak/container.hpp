#pragma once

#include "bak/dialog.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <ostream>

namespace BAK {

class Container
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
 
    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    Target mDialog;
    glm::vec<2, unsigned> mLocation;
    Inventory mItems;
};

std::ostream& operator<<(std::ostream& os, const Container& i);

}
