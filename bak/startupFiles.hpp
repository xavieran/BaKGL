#pragma once

#include "bak/types.hpp"

namespace BAK
{

void LoadChapter(Chapter);
void LoadFilter();
void LoadDetect();
void LoadZoneDat();
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
