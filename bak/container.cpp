#include "bak/container.hpp"

namespace BAK {

Container::Container(
    unsigned address,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    ContainerType type,
    Target dialog,
    glm::vec<2, unsigned> location,
    Inventory&& inventory)
:
    mAddress{address},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mDialog{dialog},
    mLocation{location},
    mInventory{std::move(inventory)}
{}

std::ostream& operator<<(std::ostream& os, const Container& i)
{
    os << "Container { addr: " << std::hex << i.mAddress << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << ToString(i.mType) << std::dec
        << ", dialog:" << i.mDialog << ", loc: " << i.mLocation
        << ", inventory: [" << i.mInventory << "]}";
    return os;
}

GDSContainer::GDSContainer(
    BAK::HotspotRef gdsScene,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    ContainerType type,
    Inventory&& inventory)
:
    mGdsScene{gdsScene},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mInventory{std::move(inventory)}
{}

std::ostream& operator<<(std::ostream& os, const GDSContainer& i)
{
    os << "Container { addr: " << std::hex << i.mGdsScene << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << ToString(i.mType)
        << ", inventory: [" << i.mInventory << "]}";
    return os;
}


}
