#include "bak/fixedObject.hpp"

#include "bak/coordinates.hpp"

#include "com/ostream.hpp"

namespace BAK {

FixedObject::FixedObject(
    ContainerHeader header,
    Target dialogKey,
    std::optional<HotspotRef> hotspotRef,
    std::optional<glm::vec<2, unsigned>> encounterOff)
:
    mHeader{header},
    mDialogKey{dialogKey},
    mHotspotRef{hotspotRef},
    mEncounterPos{std::invoke([&]() -> std::optional<GamePosition> {
        if (encounterOff)
        {
            ASSERT(std::holds_alternative<ContainerWorldLocation>(header.mLocation));
            const auto& loc = std::get<ContainerWorldLocation>(header.mLocation);
            return MakeGamePositionFromTileAndOffset(
                GetTile(loc.mLocation),
                *encounterOff);
        }
        else
        {
            return std::optional<GamePosition>{};
        }
    })}
{}

std::ostream& operator<<(std::ostream& os, const FixedObject& obj)
{
    os << "FixedObject {" << obj.mHeader << " diag: " << obj.mDialogKey 
        << " hs: " << obj.mHotspotRef << "}" 
        << " encounterPos: " << obj.mEncounterPos << "}";
    return os;
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
std::vector<FixedObject> LoadFixedObjects(
    unsigned targetZone)
{
    std::vector<FixedObject> fixedObjects;

    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    auto fb = FileBufferFactory::CreateFileBuffer("OBJFIXED.DAT");

    fb.DumpAndSkip(2);

    unsigned z = 1;
    while (fb.GetBytesLeft() > 0)
    {
        logger.Debug() << "Z: " << z++ << " @" << std::hex << fb.Tell() << std::dec << std::endl;
        auto objects = fb.GetUint16LE();
        logger.Debug() << "Objects: " << objects<< std::endl;
        for (unsigned i = 0; i < objects; i++)
        {
            logger.Debug() << "Obj no: " << i << std::endl;
            auto header = ContainerHeader(
                ContainerWorldLocationTag{},
                fb);

            std::uint32_t dialogKey = 0;
            std::optional<glm::vec<2, unsigned>> encounterOff{};
            auto hotspotRef = std::optional<HotspotRef>{};

            if (header.mContainerType == 0x0)
            {
                // No dialog
            }
            else if (header.mContainerType == 0x2
                || header.mContainerType == 0x21)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
            }
            else if (header.mContainerType == 0x8)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
                const auto hasEncounter = fb.GetUint8();
                const auto xOff = fb.GetUint8();
                const auto yOff = fb.GetUint8();
                if (hasEncounter != 0)
                    encounterOff = glm::vec<2, unsigned>{xOff, yOff};
            }
            else if (header.mContainerType == 0xa)
            {
                fb.DumpAndSkip(2);
                dialogKey = fb.GetUint32LE();
                fb.DumpAndSkip(4);
                hotspotRef = HotspotRef{
                    fb.GetUint8(),
                    static_cast<char>(
                        fb.GetUint8() + 0x40)};
                if (hotspotRef == HotspotRef{0, 0x40})
                    hotspotRef.reset();
                const auto hasEncounter = fb.GetUint8();
                const auto xOff = fb.GetUint8();
                const auto yOff = fb.GetUint8();
                if (hasEncounter != 0)
                    encounterOff = glm::vec<2, unsigned>{xOff, yOff};
            }
            else
            {
                ASSERT(false);
            }

            logger.Debug() << header << " dialog: " << std::hex
                << dialogKey << " hs: " << hotspotRef << std::dec 
                << " encounterOff: " << encounterOff << std::endl;

            if (targetZone == header.GetZone().mValue)
            {
                fixedObjects.emplace_back(
                    header,
                    KeyTarget{dialogKey},
                    hotspotRef,
                    encounterOff);
            }
        }
    }

    return fixedObjects;
}



}
