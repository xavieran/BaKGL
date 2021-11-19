#pragma once

#include "bak/inventory.hpp"
#include "bak/lock.hpp"
#include "bak/shop.hpp"

#include <string_view>

namespace BAK {

enum class ContainerType
{
    Bag           =  0x0,
    CT1           =  0x1,
    Gravestone    =  0x2,
    Building      =  0x3,
    Shop          =  0x4,
    Inn           =  0x6,
    TimirianyaHut =  0x8,
    Combat        =  0xa, //(0x8 + 0x2)
    Chest         = 0x10,
    FairyChest    = 0x11,
    EventChest    = 0x19,
    Hole          = 0x21,

    Key = 0xfe,
    Inv = 0xff
};

enum class ClickableProperty
{
    HasLock      = 1,
    HasDialog    = 2,
    HasShop      = 4,
    HasEncounter = 8,
    HasTime      = 0x10,
};

std::string_view ToString(ContainerType);

class IContainer
{
public:
    virtual const Inventory& GetInventory() const = 0;
    virtual Inventory& GetInventory() = 0;
    virtual bool CanAddItem(const InventoryItem&) const = 0;
    virtual bool GiveItem(const InventoryItem&) = 0;
    virtual bool RemoveItem(const InventoryItem&) = 0;
    virtual ContainerType GetContainerType() const = 0;
    virtual ShopStats& GetShopData() = 0;
    virtual LockStats& GetLockData() = 0;
    bool IsShop() const;
};

}
