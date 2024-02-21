#include "bak/container.hpp"

#include "bak/inventory.hpp"

#include "com/ostream.hpp"

#include <optional>

namespace BAK {

std::ostream& operator<<(std::ostream& os, const ContainerWorldLocation& loc)
{
    os << "CWL { Z: " << loc.mZone << " from: " << loc.GetFrom() 
        << " to: " << loc.GetTo()
        << " model: " << +loc.mModel << " unk: " << std::hex << +loc.mUnknown << std::dec
        << " loc: " << loc.mLocation << "}";
    return os;
}

unsigned ContainerWorldLocation::GetFrom() const
{
    return (mChapterRange >> 4) & 0xf;
}

unsigned ContainerWorldLocation::GetTo() const
{
    return mChapterRange & 0xf;
}

std::ostream& operator<<(std::ostream& os, const ContainerGDSLocation& loc)
{
    os << "CGL { arr: " << std::hex << loc.mUnknown << std::dec << " loc: " << loc.mLocation << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ContainerCombatLocation& loc)
{
    os << "CCL { arr: " << std::hex << loc.mUnknown << std::dec << " combat: " << loc.mCombat << " person: " << loc.mCombatant << "}";
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
    os << "ContainerHeader @" <<  std::hex << header.mAddress << std::dec 
        << " {" << header.mLocation << " LocType: "
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
                0,
                0,
                0,
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
    const auto chapterRange = fb.GetUint8();
    const auto model = fb.GetUint8();
    const auto unknown = fb.GetUint8();
    const auto x = fb.GetUint32LE();
    const auto y = fb.GetUint32LE();

    mLocation = ContainerWorldLocation{
        zone,
        chapterRange,
        model,
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

ContainerHeader::ContainerHeader(ContainerCombatLocationTag, FileBuffer& fb)
{
    mAddress = fb.Tell();

    mLocation = ContainerCombatLocation{
        fb.GetArray<4>(),
        fb.GetUint32LE(),
        fb.GetUint32LE()};

    mLocationType = fb.GetUint8();
    ASSERT(mLocationType == 7);
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

unsigned ContainerHeader::GetCombatNumber() const
{
    ASSERT(std::holds_alternative<ContainerCombatLocation>(mLocation));
    return std::get<ContainerCombatLocation>(mLocation).mCombat;
}

unsigned ContainerHeader::GetCombatantNumber() const
{
    ASSERT(std::holds_alternative<ContainerCombatLocation>(mLocation));
    return std::get<ContainerCombatLocation>(mLocation).mCombatant;
}

unsigned ContainerHeader::GetModel() const
{
    assert(std::holds_alternative<ContainerWorldLocation>(mLocation));
    return std::get<ContainerWorldLocation>(mLocation).mModel;
}

bool ContainerHeader::PresentInChapter(Chapter chapter) const
{
    if (std::holds_alternative<ContainerWorldLocation>(mLocation))
    {
        auto& location = std::get<ContainerWorldLocation>(mLocation);
        return (chapter.mValue <= location.GetTo())
            && (chapter.mValue >= location.GetFrom());
    }
    // FIXME: Double check if this applies to other container types
    return true;
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



}
