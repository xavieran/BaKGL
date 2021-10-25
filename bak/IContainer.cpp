#include "IContainer.hpp"

namespace BAK {

std::string_view ToString(ContainerType type)
{
    switch (type)
    {
    case ContainerType::Bag: return "Bag";
    case ContainerType::Gravestone: return "Gravestone";
    case ContainerType::Building: return "Building";
    case ContainerType::Shop: return "Shop";
    case ContainerType::Inn: return "Inn";
    case ContainerType::TimirianyaHut: return "TimirianyaHut";
    case ContainerType::Combat: return "Combat";
    case ContainerType::Chest: return "Chest";
    case ContainerType::FairyChest: return "FairyChest";
    case ContainerType::EventChest: return "EventChest";
    default: return "UnknownContainerType";
    }
}

}
