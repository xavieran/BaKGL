#include "IContainer.hpp"

#include "com/ostream.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const Shop& shop)
{
    os << std::dec << "Shop { templeNumber: " << +shop.mTempleNumber
        << " sellFactor: " << +shop.mSellFactor
        << " maxDiscount: " << +shop.mMaxDiscount
        << " buyFactor: " << +shop.mBuyFactor
        << " haggle: " << std::hex << shop.mHaggle << std::dec
        << " bardingSkill: " << +shop.mBardingSkill
        << " bardingReward: " << +shop.mBardingReward
        << " bardingMaxReward: " << +shop.mBardingMaxReward
        << " unknown: " << std::hex << shop.mUnknown << std::dec
        << " repairTypes: " << +shop.mRepairTypes
        << " repairFactor: " << +shop.mRepairFactor
        << " categories: " << std::hex << shop.mCategories << std::dec << "}";

    return os;
}

std::string_view ToString(ContainerType type)
{
    switch (type)
    {
    case ContainerType::Bag: return "Bag";
    case ContainerType::CT1: return "CT1";
    case ContainerType::Gravestone: return "Gravestone";
    case ContainerType::Building: return "Building";
    case ContainerType::Shop: return "Shop";
    case ContainerType::Inn: return "Inn";
    case ContainerType::TimirianyaHut: return "TimirianyaHut";
    case ContainerType::Combat: return "Combat";
    case ContainerType::Chest: return "Chest";
    case ContainerType::FairyChest: return "FairyChest";
    case ContainerType::EventChest: return "EventChest";
    case ContainerType::Hole: return "Hole";
    case ContainerType::Key: return "Key";
    case ContainerType::Inv: return "Inv";
    default: return "UnknownContainerType";
    }
}

bool IContainer::IsShop() const
{
    return GetContainerType() == BAK::ContainerType::Shop
        || GetContainerType() == BAK::ContainerType::Inn;
}

}
