#include "bak/encounter/zone.hpp"
#include "bak/encounter/encounter.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const Zone& zone)
{
    os << "Zone { " << zone.mTargetZone
        << " worldPosition: " << zone.mTargetLocation
        << " dialog: " << std::hex << zone.mDialog << std::dec << "}";
    return os;
}

ZoneFactory::ZoneFactory()
:
    mZones{}
{
    Load();
}

const Zone& ZoneFactory::Get(unsigned i) const
{
    assert(i < mZones.size());
    return mZones[i];
}

void ZoneFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        sFilename);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto zone = fb.GetUint8();
        const auto tile = glm::vec<2, unsigned>{
            fb.GetUint8(), fb.GetUint8()};
        const auto offset = glm::vec<2, std::uint8_t>{
            fb.GetUint8(), fb.GetUint8()};
        const auto heading = fb.GetUint16LE();
        const auto dialog = fb.GetUint32LE();
        assert(fb.GetUint32LE() == 0);
        assert(fb.GetUint16LE() == 0);

        const auto loc = MakeGamePositionFromTileAndOffset(tile, offset);

        mZones.emplace_back(
            zone,
            GamePositionAndHeading{loc, heading},
            KeyTarget{dialog});
    }
}

}

