#include "bak/container.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const ContainerWorldLocation& loc)
{
    os << "CWL { Z: " << loc.mZone << ", arr: " << std::hex << loc.mUnknown << std::dec
        << " loc: " << loc.mLocation << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ContainerGDSLocation& loc)
{
    os << "CGL { arr: " << std::hex << loc.mUnknown << std::dec << " loc: " << loc.mLocation << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ContainerLocation& loc)
{
    return std::visit(
        [&](const auto& i) -> std::ostream& {
            return (os << i);
        }, loc);
}

std::ostream& operator<<(std::ostream& os, const ContainerHeader& header)
{
    os << "ContainerHeader {" << header.mLocation << " LocType: "
        << +header.mLocationType << " Items: " << +header.mItems
        << " Capacity: " << +header.mCapacity << " ContainerType: "
        << +header.mContainerType << "}";
    return os;
}

ContainerHeader::ContainerHeader(ContainerWorldLocationTag, FileBuffer& fb)
{
    const auto zone = ZoneNumber{fb.GetUint8()};
    const auto unknown = fb.GetArray<3>();
    const auto x = fb.GetUint32LE();
    const auto y = fb.GetUint32LE();
    mLocation = ContainerWorldLocation{
        zone,
        unknown,
        GamePosition{x, y}};

    mLocationType  = fb.GetUint8();
    mItems         = fb.GetUint8();
    mCapacity      = fb.GetUint8();
    mContainerType = fb.GetUint8();
}

ContainerHeader::ContainerHeader(ContainerGDSLocationTag, FileBuffer& fb)
{
    mLocation = ContainerGDSLocation{
        fb.GetArray<4>(),
        HotspotRef{
            static_cast<std::uint8_t>(fb.GetUint32LE()),
            MakeHotspotChar(static_cast<char>(fb.GetUint32LE()))}};

    mLocationType  = fb.GetUint8();
    mItems         = fb.GetUint8();
    mCapacity      = fb.GetUint8();
    mContainerType = fb.GetUint8();
}


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
