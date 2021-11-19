#include "bak/container.hpp"

#include "bak/inventory.hpp"

#include "com/ostream.hpp"

#include <optional>

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

std::ostream& operator<<(std::ostream& os, const ContainerEncounter& ce)
{
    os << "ContainerEncounter { require: " << std::hex << ce.mRequireEventFlag
        << " set: " << ce.mSetEventFlag << std::dec << " hotspot: " << 
        ce.mHotspotRef << " pos: " << ce.mEncounterPos << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ContainerDialog& ce)
{
    os << "ContainerDialog { unknown: " << std::hex << ce.mUnknown
        << " dialog: " << ce.mDialog << std::dec << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GenericContainer& gc)
{
    os << "GenericContainer{ " << gc.mHeader;
    if (gc.mLock) os << gc.mLock;
    if (gc.mDialog) os << gc.mDialog;
    if (gc.mShop) os << gc.mShop;
    if (gc.mEncounter) os << gc.mEncounter;
    if (gc.mLastAccessed) os << std::hex << " LastAccessed: " << gc.mLastAccessed << std::dec;
    os << "}";
    return os;
}

GenericContainer LoadGenericContainer(FileBuffer& fb)
{
    auto header = ContainerHeader{
        ContainerWorldLocationTag{},
        fb};

    auto lockData = std::optional<LockStats>{};
    auto dialog = std::optional<ContainerDialog>{};
    auto shopData = std::optional<ShopStats>{};
    auto encounter = std::optional<ContainerEncounter>{};
    auto lastAccessed = std::optional<Time>{};

    auto inventory = LoadItems(fb, header.mItems, header.mCapacity);

    if ((header.mContainerType & 0x1) != 0)
    {
        lockData = LoadLock(fb);
    }
    if ((header.mContainerType & 0x2) != 0)
    {
        const auto unknown = fb.GetUint16LE();
        const auto diag = KeyTarget{fb.GetUint32LE()};
        dialog = ContainerDialog{unknown, diag};
    }
    if ((header.mContainerType & 0x4) != 0)
    {
        shopData = LoadShop(fb);
    }
    if ((header.mContainerType & 0x8) != 0)
    {
        const auto requireEventFlag = fb.GetUint16LE();
        const auto setEventFlag = fb.GetUint16LE();
        auto hotspotRef = std::optional<HotspotRef>{};
        hotspotRef = HotspotRef{
            fb.GetUint8(),
            static_cast<char>(
                fb.GetUint8() + 0x40)};
        if (hotspotRef->mGdsNumber == 0)
            hotspotRef.reset();
        auto encounterOff = std::optional<glm::uvec2>{};
        const auto hasEncounter = fb.GetUint8();
        const auto xOff = fb.GetUint8();
        const auto yOff = fb.GetUint8();
        if (hasEncounter != 0)
            encounterOff = glm::uvec2{xOff, yOff};

        encounter = ContainerEncounter{
            requireEventFlag,
            setEventFlag,
            hotspotRef,
            encounterOff};
    }
    if ((header.mContainerType & 0x10) != 0)
    {
        lastAccessed = Time{fb.GetUint32LE()};
    }

    return GenericContainer{
        header,
        lockData,
        dialog,
        shopData,
        encounter,
        lastAccessed};
}

Container::Container(
    unsigned address,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    ContainerType type,
    Target dialog,
    glm::vec<2, unsigned> location,
    std::optional<ShopStats> shopData,
    LockStats lockData,
    Inventory&& inventory)
:
    mAddress{address},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mDialog{dialog},
    mLocation{location},
    mShopData{shopData},
    mLockData{lockData},
    mInventory{std::move(inventory)}
{}

std::ostream& operator<<(std::ostream& os, const Container& i)
{
    os << "Container { addr: " << std::hex << i.mAddress << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << ToString(i.mType) << std::dec
        << ", dialog:" << i.mDialog << ", loc: " << i.mLocation 
        << " shop: " << i.mShopData << " lock: " << i.mLockData
        << ", inventory: [" << i.mInventory << "]}";
    return os;
}

GDSContainer::GDSContainer(
    BAK::HotspotRef gdsScene,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    ContainerType type,
    std::optional<ShopStats> shopData,
    LockStats lockData,
    Inventory&& inventory)
:
    mGdsScene{gdsScene},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mShopData{shopData},
    mLockData{lockData},
    mInventory{std::move(inventory)}
{}

std::ostream& operator<<(std::ostream& os, const GDSContainer& i)
{
    os << "Container { addr: " << std::hex << i.mGdsScene << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << ToString(i.mType)
        << " shop: " << i.mShopData << " lock: " << i.mLockData
        << ", inventory: [" << i.mInventory << "]}";
    return os;
}


}
