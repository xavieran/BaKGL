#include "bak/encounter/zone.hpp"
#include "bak/encounter/encounter.hpp"

#include "com/assert.hpp"

#include "bak/fileBufferFactory.hpp"

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
    ASSERT(i < mZones.size());
    return mZones[i];
}

void ZoneFactory::Load()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        sFilename);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto zone = fb.GetUint8();
        const auto tile = glm::vec<2, unsigned>{
            fb.GetUint8(), fb.GetUint8()};
        const auto cell = glm::vec<2, std::uint8_t>{
            fb.GetUint8(), fb.GetUint8()};
        const auto heading = static_cast<std::uint16_t>(
            fb.GetUint16LE() / 0xffu);
        const auto dialog = fb.GetUint32LE();
        ASSERT(fb.GetUint32LE() == 0);
        ASSERT(fb.GetUint16LE() == 0);

        const auto loc = MakeGamePositionFromTileAndCell(tile, cell);

        mZones.emplace_back(
            Zone{
                ZoneNumber{zone},
                GamePositionAndHeading{loc, heading},
                KeyTarget{dialog}});
        Logging::LogSpam("ZoneFactory") << "ZoneTransition: " << i << " " << mZones.back() << "\n";
    }
}

}

