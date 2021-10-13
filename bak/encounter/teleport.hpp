#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/types.hpp"

#include <optional>

/*
TargetZone
01 0B 0B 21 10 00 20  46 00  01 00 
01 0F 10 0F 08 00 00 46 00 02 00 
02 10 12 18 04 00 20 46 00 03 00 
02 12 0C 12 18 00 00 46 00 04 00 
03 0F 0B 08 16 00 20 46 00 05 00 
03 0C 0A 11 14 00 00 46 00 06 00 
04 0B 0C 17 10 00 40 46 00 07 00 
04 0A 14 0D 1D 00 00 46 00 08 00 
05 0B 0B 1C 0A 00 E0 46 00 09 00 
05 13 0E 0E 0B 00 20 46 00 0A 00 
02 13 0A 0C 07 00 00 02 00 04 00 
03 0D 0A 1A 05 00 00 03 00 03 00 
01 0F 0B 23 14 00 40 00 00 00 00 
02 10 12 1D 25 00 80 00 00 00 00 
07 0A 0E 03 15 00 A0 00 00 00 00 
02 14 0E 22 11 00 40 00 00 00 00 
03 0A 0D 02 09 00 C0 00 00 00 00 
07 0D 0A 1D 03 00 00 00 00 00 00 
02 13 0A 0B 08 00 20 02 00 05 00 
Zone TileX TileY Xoff YOff Heading
0B 0A 0A 26 16 00 00 00 00 00 00  // Sewer???
02 13 0A 0B 08 00 20 02 00 02 00 
0B 0B 0B 0A 0A 00 80 00 00 00 00 
04 0B 0D 05 16 00 C0 28 00 09 00 
0C 0D 0A 18 15 00 00 00 00 00 00 
03 0C 0D 15 1C 00 80 0C 00 01 00 
0A 0E 0D 09 15 00 00 00 00 00 00 
02 11 0D 0A 20 00 80 05 00 02 00 
0A 0A 10 15 02 00 00 00 00 00 00 
0B 0A 10 25 12 00 40 00 00 00 00 
06 0B 0B 11 15 00 A0 00 00 00 00 
FF 00 00 00 00 00 00 02 00 02 00 
FF 00 00 00 00 00 00 06 00 01 00 
00 00 00 00 00 00 00 00 00 00 00 
06 0E 11 15 22 00 80 04 00 01 00 
0C 0A 10 0D 03 00 00 00 00 00 00 
0A 0A 10 10 26 00 80 00 00 00 00 
02 11 0D 0A 20 00 80 05 00 01 00 
FF 00 00 00 00 00 00 28 00 02 00 
0A 0E 0E 01 0C 00 A0 00 00 00 00 
09 0F 0E 13 1D 00 80 00 00 00 00

*/

namespace BAK::Encounter {

class Teleport
{
public:
    std::optional<ZoneNumber> mTargetZone;
    GamePositionAndHeading mTargetLocation;
    std::optional<HotspotRef> mTargetGDSScene;
};

std::ostream& operator<<(std::ostream& os, const Teleport&);

class TeleportFactory
{
public:
    static constexpr auto sFilename = "TELEPORT.DAT";

    TeleportFactory();

    const Teleport& Get(unsigned i) const;

private:
    void Load();
    std::vector<Teleport> mTeleports;
};

}
