#pragma once

#include "dialogTarget.hpp"

#include "xbak/FileBuffer.h"

#include <glm/glm.hpp>

/*
DEF_TOWN.DAT
0d00 14    town tag    entry dg  exit dg xo yo transition
0000 0127 0001 0000 00 68e31600 69e31600 10 14 0040 01
0000 0127 0002 0000 00 74e31600 75e31600 0b 08 00a0 01
0000 0127 0005 0000 00 99e31600 9ae31600 0a 20 0000 01
0000 0127 000c 0000 00 dee31600 dfe31600 15 1c 0000 01
0000 0127 0003 0000 00 86e31600 87e31600 1b 06 0060 01
0000 0127 000b 0000 00 d5e31600 d6e31600 12 09 00c0 01
0000 0127 0006 0000 00 a1e31600 a2e31600 13 1a 00a0 01
0000 0127 0009 0000 00 bee31600 bfe31600 06 15 0040 01
0000 0117 0008 0000 00 b5e31600 b6e31600 23 1d 00c0 01
0000 0107 0007 0000 00 a8e31600 a9e31600 0c 24 0000 01
0000 0108 0004 0000 00 90e31600 91e31600 15 22 0000 01
0000 0108 000a 0000 00 c9e31600 cae31600 0f 12 0040 01
0000 00ff 030c 0000 00 dee31600 dfe31600 09 15 0080 00
0000                                   

*/

namespace BAK {

struct Town
{
    std::uint32_t mTownTag;
    KeyTarget mEntryDialog;
    KeyTarget mExitDialog;

    glm::vec<2, int> mDestOffset;
    std::uint32_t mTransitionStyle;
};

std::vector<Town> LoadTowns()
{
    std::vector<Town> towns;

    auto fb = FileBufferFactory::CreateFileBuffer("DEF_TOWN.DAT");

    const unsigned nTowns = fb.GetUint16LE();

    for (unsigned i = 0; i < nTowns; i++)
    {
        fb.DumpAndSkip(4);
        const unsigned tag = fb.GetUint16LE();
        fb.DumpAndSkip(3);
        const auto entryDialog = KeyTarget{fb.GetUint32LE()};
        const auto exitDialog  = KeyTarget{fb.GetUint32LE()};
        const int xOff = fb.GetUint8();
        const int yOff = fb.GetUint8();
        fb.DumpAndSkip(1);
        const int transition = fb.GetUint8();

        towns.emplace_back(
            tag,
            entryDialog,
            exitDialog,
            glm::vec<2, int>{xOff, yOff},
            transition);
    }

    return towns;
}

}
