#pragma once

#include "inventory.hpp"
#include "inventoryItem.hpp"

#include <ostream>
#include <string_view>

namespace BAK {

struct Lock
{
    unsigned mLockFlag;
    unsigned mRequiredPicklockSkill;
    unsigned mFairyChestIndex;
    unsigned mTrapDamage;
};

struct Shop
{
    std::uint8_t mTempleNumber;
    std::uint8_t mSellFactor;
    std::uint8_t mMaxDiscount;
    std::uint8_t mBuyFactor;
    std::uint16_t mHaggle;
    std::uint8_t mBardingSkill;
    std::uint8_t mBardingReward;
    std::uint8_t mBardingMaxReward;
    std::array<std::uint8_t, 3> mUnknown;
    std::uint8_t mRepairTypes;
    std::uint8_t mRepairFactor;
    std::uint16_t mCategories;
};

std::ostream& operator<<(std::ostream&, const Shop&);

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
    virtual const Shop& GetShopData() const = 0;
    bool IsShop() const;
};

}
