#pragma once

#include "bak/dialogTarget.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>


/* DEF_ZONE.DAT
2700 0000 
        |zone| tile?| x/y off? | heading? | 
0127 00 | 0a | 0b0a | 1127     | 0040     | f732 2900 0000 0000 0000
0127 00 | 01 | 0a0d | 0911     | 00c0     | fe32 2900 0000 0000 0000
0127 00 | 02 | 0a12 | 031e     | 00a0     | e132 2900 0000 0000 0000
0107 00 | 05 | 0a0b | 0c0d     | 00c0     | e332 2900 0000 0000 0000
0107 00 | 01 | 0f11 | 2214     | 0060     | ec32 2900 0000 0000 0000
0008 00 | 06 | 0b0a | 1503     | 0000     | e432 2900 0000 0000 0000
0100 01 | 0c | 0b0e | 120c     | 0000     | 1833 2900 0000 0000 0000
0127 00 | 02 | 0f12 | 1c25     | 0080     | e232 2900 0000 0000 0000
0127 00 | 01 | 0f0a | 2213     | 0020     | e632 2900 0000 0000 0000
0127 00 | 01 | 0a0b | 1b02     | 0020     | e532 2900 0000 0000 0000
0127 00 | 03 | 0a0a | 0411     | 00c0     | e732 2900 0000 0000 0000
0127 00 | 02 | 130a | 230e     | 0040     | e832 2900 0000 0000 0000
00fe 03 | 0a | 0a10 | 1026     | 0080     | f832 2900 0000 0000 0000
0127 00 | 04 | 0e0a | 1a04     | 0000     | e932 2900 0000 0000 0000
0127 00 | 03 | 1712 | 1325     | 0080     | ea32 2900 0000 0000 0000
0107 00 | 05 | 1710 | 1302     | 0000     | eb32 2900 0000 0000 0000
0107 00 | 04 | 0a14 | 0925     | 0080     | ed32 2900 0000 0000 0000
0110 00 | 06 | 150f | 2517     | 0060     | ee32 2900 0000 0000 0000
0110 00 | 05 | 1612 | 0924     | 00a0     | f232 2900 0000 0000 0000
0117 00 | 0c | 0d0f | 1801     | 0000     | f932 2900 0000 0000 0000
0107 00 | 07 | 0d0f | 1423     | 0060     | f032 2900 0000 0000 0000
0107 00 | 05 | 0f0a | 1902     | 0000     | f432 2900 0000 0000 0000
0008 00 | 06 | 110c | 1b07     | 0000     | ef32 2900 0000 0000 0000
0100 01 | 0a | 0d0e | 1607     | 0080     | 1933 2900 0000 0000 0000
0009 00 | 05 | 0f0e | 0d25     | 0060     | f332 2900 0000 0000 0000
0120 00 | 0a | 0a0e | 0416     | 00c0     | fb32 2900 0000 0000 0000
0120 00 | 08 | 0f0a | 1e0e     | 0040     | ff32 2900 0000 0000 0000
0126 00 | 0a | 0b0e | 2514     | 0040     | 0033 2900 0000 0000 0000
0126 00 | 0a | 0a0a | 161b     | 00c0     | 0133 2900 0000 0000 0000
00ff 03 | 02 | 1111 | 191e     | 0080     | 0233 2900 0000 0000 0000
0180 00 | 09 | 0d0c | 091d     | 00c0     | 0333 2900 0000 0000 0000
0180 00 | 0a | 0e0a | 0717     | 0000     | fd32 2900 0000 0000 0000
0120 00 | 0b | 0b0d | 0a19     | 0000     | 0633 2900 0000 0000 0000
0120 00 | 0b | 0a0a | 1e21     | 0000     | 0733 2900 0000 0000 0000
0108 00 | 0c | 0a10 | 2606     | 0000     | 0833 2900 0000 0000 0000
0108 00 | 0c | 0b0b | 0e09     | 0000     | 0933 2900 0000 0000 0000
0120 00 | 0c | 0d0d | 1c01     | 0000     | 24fe 1c00 0000 0000 0000
0120 00 | 08 | 0a0d | 1116     | 0080     | 0a33 2900 0000 0000 0000
0117 00 | 05 | 1610 | 111e     | 0080     | 0b33 2900 0000 0000 0000 
*/

namespace BAK {

class Zone
{
public:
    std::uint32_t mZoneTag;
    unsigned mTargetZone;
    glm::vec<2, unsigned> mTargetTile;
    glm::vec<2, unsigned> mTargetTileOffset;
    double mTargetHeading;
    KeyTarget mDialog;
};

std::vector<Zone> LoadZones()
{
    std::vector<Zone> zones;

    auto fb = FileBufferFactory::CreateFileBuffer("DEF_ZONE.DAT");

    const unsigned nZones = fb.GetUint16LE();
	fb.DumpAndSkip(2);

    for (unsigned i = 0; i < nZones; i++)
    {
        const unsigned tag = fb.GetUint16LE();
        fb.DumpAndSkip(1);
        const auto targetZone = fb.GetUint8();
        const auto tileX = fb.GetUint8();
        const auto tileY = fb.GetUint8();
        const auto tileXOff = fb.GetUint8();
        const auto tileYOff = fb.GetUint8();
        const auto heading = fb.GetUint16LE();
        const auto dialog = KeyTarget{fb.GetUint32LE()};
		fb.DumpAndSkip(6);

        zones.emplace_back(
            tag,
            targetZone,
            glm::vec<2, unsigned>{tileX, tileY},
            glm::vec<2, unsigned>{tileXOff, tileYOff},
            heading,
            dialog);
    }

    return zones;
}

}
