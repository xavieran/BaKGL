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
        << " Capacity: " << +header.mCapacity << " Flags: " << std::hex
        << +header.mFlags << std::dec << "}";
    return os;
}

ContainerHeader::ContainerHeader()
    :
        mAddress{0},
        mLocation{
            ContainerWorldLocation{
                ZoneNumber{0},
                std::array<std::uint8_t, 3>{},
                glm::uvec2{0, 0}}},
        mLocationType{0},
        mItems{0},
        mCapacity{0},
        mFlags{0}
{
}

ContainerHeader::ContainerHeader(ContainerWorldLocationTag, FileBuffer& fb)
{
    mAddress = fb.Tell();

    const auto zone = ZoneNumber{fb.GetUint8()};
    const auto unknown = fb.GetArray<3>();
    const auto x = fb.GetUint32LE();
    const auto y = fb.GetUint32LE();

    mLocation = ContainerWorldLocation{
        zone,
        unknown,
        GamePosition{x, y}};

    mLocationType = fb.GetUint8();
    mItems        = fb.GetUint8();
    mCapacity     = fb.GetUint8();
    mFlags        = fb.GetUint8();
}

ContainerHeader::ContainerHeader(ContainerGDSLocationTag, FileBuffer& fb)
{
    mAddress = fb.Tell();

    mLocation = ContainerGDSLocation{
        fb.GetArray<4>(),
        HotspotRef{
            static_cast<std::uint8_t>(fb.GetUint32LE()),
            MakeHotspotChar(static_cast<char>(fb.GetUint32LE()))}};

    mLocationType = fb.GetUint8();
    mItems        = fb.GetUint8();
    mCapacity     = fb.GetUint8();
    mFlags        = fb.GetUint8();
}

ZoneNumber ContainerHeader::GetZone() const
{
    ASSERT(std::holds_alternative<ContainerWorldLocation>(mLocation));
    return std::get<ContainerWorldLocation>(mLocation).mZone;
}

GamePosition ContainerHeader::GetPosition() const
{
    ASSERT(std::holds_alternative<ContainerWorldLocation>(mLocation));
    return std::get<ContainerWorldLocation>(mLocation).mLocation;
}

HotspotRef ContainerHeader::GetHotspotRef() const
{
    ASSERT(std::holds_alternative<ContainerGDSLocation>(mLocation));
    return std::get<ContainerGDSLocation>(mLocation).mLocation;
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
    os << "ContainerDialog { cv: " 
        << std::hex << +ce.mContextVar
        << " order: " << +ce.mDialogOrder
        << " dialog: " << ce.mDialog << std::dec << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GenericContainer& gc)
{
    os << "GenericContainer{ " << gc.GetHeader();
    if (gc.HasLock()) os << gc.GetLock();
    if (gc.HasDialog()) os << gc.GetDialog();
    if (gc.HasShop()) os << gc.GetShop();
    if (gc.HasEncounter()) os << gc.GetEncounter();
    if (gc.HasLastAccessed()) os << std::hex << " LastAccessed: " << gc.GetLastAccessed() << std::dec;
    if (gc.HasInventory()) os << " " << gc.GetInventory();
    os << "}";
    return os;
}

GenericContainer LoadGenericContainer(FileBuffer& fb, bool isGdsLocation)
{
    auto header = isGdsLocation
        ? ContainerHeader{ContainerGDSLocationTag{}, fb}
        : ContainerHeader{ContainerWorldLocationTag{}, fb};

    auto lockData = std::optional<LockStats>{};
    auto dialog = std::optional<ContainerDialog>{};
    auto shopData = std::optional<ShopStats>{};
    auto encounter = std::optional<ContainerEncounter>{};
    auto lastAccessed = std::optional<Time>{};

    auto inventory = LoadInventory(fb, header.mItems, header.mCapacity);

    if (header.mFlags == 0x21)
    {
        // This is not actually correct interpretation
        // for this property type
        const auto contextVar = fb.GetUint8();
        const auto dialogOrder = fb.GetUint8();
        const auto dialogKey = KeyTarget{fb.GetUint32LE()};
        dialog = ContainerDialog{contextVar, dialogOrder, dialogKey};
    }
    else
    {
        if (header.HasLock())
        {
            lockData = LoadLock(fb);
        }
        if (header.HasDialog())
        {
            const auto contextVar = fb.GetUint8();
            const auto dialogOrder = fb.GetUint8();
            const auto dialogKey = KeyTarget{fb.GetUint32LE()};
            dialog = ContainerDialog{contextVar, dialogOrder, dialogKey};
        }
        if (header.HasShop())
        {
            shopData = LoadShop(fb);
        }
        if (header.HasEncounter())
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
            auto encounterPos = std::optional<glm::uvec2>{};
            const auto hasEncounter = fb.GetUint8();
            const auto xOff = fb.GetUint8();
            const auto yOff = fb.GetUint8();
            if (hasEncounter != 0)
            {
                const auto encounterOff = glm::uvec2{xOff, yOff};
                encounterPos = MakeGamePositionFromTileAndOffset(
                    GetTile(header.GetPosition()),
                    encounterOff);
            }

            encounter = ContainerEncounter{
                requireEventFlag,
                setEventFlag,
                hotspotRef,
                encounterPos};
        }
        if (header.HasTime())
        {
            lastAccessed = Time{fb.GetUint32LE()};
        }
    }

    return GenericContainer{
        header,
        lockData,
        dialog,
        shopData,
        encounter,
        lastAccessed,
        std::move(inventory)};
}

}
