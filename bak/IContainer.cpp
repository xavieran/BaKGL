#include "IContainer.hpp"

#include "com/ostream.hpp"

namespace BAK {

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

bool IContainer::HasLock() const
{
    return CheckBitSet(
        static_cast<std::uint8_t>(GetContainerType()),
        0x0);
}


}
