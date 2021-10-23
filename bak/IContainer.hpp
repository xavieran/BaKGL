#pragma once

#include "inventory.hpp"
#include "inventoryItem.hpp"

namespace BAK {

enum class ContainerType
{
    Bag           =  0x0,
    Gravestone    =  0x2,
    Building      =  0x3,
    Shop          =  0x4,
    Inn           =  0x6,
    TimirianyaHut =  0x8, // ?
    Combat        = 0x10,
    Chest         = 0x16,
    FairyChest    = 0x17,
    EventChest    = 0x25
};

class IContainer
{
public:
    virtual const Inventory& GetInventory() const = 0;
    virtual Inventory& GetInventory() = 0;
};

}
