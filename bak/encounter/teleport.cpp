#include "bak/encounter/teleport.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const Teleport& tele)
{
    os << "Teleport { " << tele.mTargetZone
        << " worldPosition: " << tele.mTargetLocation
        << " GDS: " << tele.mTargetGDSScene << "}";
    return os;
}

TeleportFactory::TeleportFactory()
:
    mTeleports{}
{
    Load();
}

const Teleport& TeleportFactory::Get(unsigned i) const
{
    ASSERT(i < mTeleports.size());
    return mTeleports[i];
}

void TeleportFactory::Load()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        sFilename);

    unsigned i = 0;
    while (fb.GetBytesLeft() != 0)
    {
        const auto possibleZone = fb.GetUint8();
        const auto tile = glm::vec<2, unsigned>{
            fb.GetUint8(), fb.GetUint8()};
        const auto offset = glm::vec<2, std::uint8_t>{
            fb.GetUint8(), fb.GetUint8()};
        const auto heading = static_cast<std::uint16_t>(
            fb.GetUint16LE() / 0xffu);
        const auto hotspotNum = static_cast<std::uint8_t>(fb.GetUint16LE());
        const auto hotspotChar = MakeHotspotChar(static_cast<std::uint8_t>(fb.GetUint16LE()));

        std::optional<ZoneNumber> zone{};
        if (possibleZone != 0xff)
            zone = ZoneNumber{possibleZone};

        const auto loc = MakeGamePositionFromTileAndOffset(tile, offset);

        std::optional<HotspotRef> hotspotRef{};
        if (hotspotNum != 0)
            hotspotRef = HotspotRef{hotspotNum, hotspotChar};

        mTeleports.emplace_back(
            zone,
            GamePositionAndHeading{loc, heading},
            hotspotRef);

        Logging::LogDebug("TeleportFactory") << "TeleportTransition: " << i++ << " " << mTeleports.back() << "\n";
    }
}

}

