#include "bak/encounter/zone.hpp"
#include "bak/encounter/encounter.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

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
        const auto tileX = fb.GetUint8();
        const auto tileY = fb.GetUint8();
        const auto xOff = fb.GetUint8();
        const auto yOff = fb.GetUint8();
        const auto heading = fb.GetUint16LE();
        const auto dialog = fb.GetUint32LE();
        assert(fb.GetUint32LE() == 0);
        assert(fb.GetUint16LE() == 0);

        const auto xLoc = TileOffsetToWorldLocation(tileX, xOff);
        const auto yLoc = TileOffsetToWorldLocation(tileY, yOff);
        mZones.emplace_back(
            zone,
            glm::vec<2, unsigned>{xLoc, yLoc},
            heading,
            KeyTarget{dialog});
    }
}

}

