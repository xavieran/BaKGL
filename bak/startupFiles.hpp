#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include <array>

namespace BAK
{

struct ChapterStartLocation
{
    MapLocation mMapLocation;
    Location mLocation;
    Time mTimeElapsed;
};

std::ostream& operator<<(std::ostream&, const ChapterStartLocation&);

ChapterStartLocation LoadChapterStartLocation(Chapter);
void LoadFilter();
void LoadDetect();
void LoadZoneDat(ZoneNumber);
void LoadZoneDefDat(ZoneNumber);

using ZoneMap = std::array<std::uint8_t, 0x190>;

// This is used to say whether there is a tile adjacent
// for the purposes of loading surrounding tiles
ZoneMap LoadZoneMap(ZoneNumber);
unsigned GetMapDatTileValue(
    unsigned x,
    unsigned y,
    const ZoneMap& mapDat);

void LoadStartDat();
}
