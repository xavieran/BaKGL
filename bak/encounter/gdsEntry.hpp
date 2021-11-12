#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/hotspotRef.hpp"

#include "graphics/glm.hpp"

namespace BAK::Encounter {

/*
 03 00 00 00
         |hotspot   | entry    |  exit       offset?
 01 27 00 28 0B 00 00 D0 5C 15 00 D1 5C 15 00 0B 04 00 A0 01 00 00
 01 27 00 28 09 00 00 C6 5C 15 00 C7 5C 15 00 05 16 00 40 01 00 00
 01 00 01 28 0D 00 00 22 40 2C 00 00 00 00 00 23 0C 00 40 00 00 00
*/

// This is a GDS scene ala towns
struct RawGDSEntry
{
    RawGDSEntry(
        HotspotRef hotspot,
        KeyTarget entryDialog,
        KeyTarget exitDialog,
        glm::vec<2, unsigned> exitOffset,
        std::uint16_t exitHeading,
        // Whether to animate travelling into the town
        bool walkToDest)
    :
        mHotspot{hotspot},
        mEntryDialog{entryDialog},
        mExitDialog{exitDialog},
        mExitOffset{exitOffset},
        mExitHeading{exitHeading},
        mWalkToDest{walkToDest}
    {}

    HotspotRef mHotspot;
    KeyTarget mEntryDialog;
    KeyTarget mExitDialog;
    glm::vec<2, unsigned> mExitOffset;
    std::uint16_t mExitHeading;
    // Whether to animate travelling into the town
    bool mWalkToDest;
};

struct GDSEntry
{
    HotspotRef mHotspot;
    KeyTarget mEntryDialog;
    KeyTarget mExitDialog;
    GamePositionAndHeading mExitPosition;
    // Whether to animate travelling into the town
    bool mWalkToDest;
};

std::ostream& operator<<(std::ostream& os, const GDSEntry&);

template <typename SourceFile>
class GDSEntryFactory
{
public:
    static constexpr auto sFilename = SourceFile::file;

    GDSEntryFactory();

    GDSEntry Get(unsigned i, glm::vec<2, unsigned> tile) const;

private:
    void Load();

    std::vector<RawGDSEntry> mGDSEntrys;
};

}
