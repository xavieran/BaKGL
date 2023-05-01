#pragma once

#include "bak/encounter/gdsEntry.hpp"

#include "com/assert.hpp"

#include "bak/fileBufferFactory.hpp"

namespace BAK::Encounter {

template <typename S>
GDSEntryFactory<S>::GDSEntryFactory()
:
    mGDSEntrys{}
{
    Load();
}

template <typename S>
GDSEntry GDSEntryFactory<S>::Get(unsigned i, glm::vec<2, unsigned> tile) const
{
    ASSERT(i < mGDSEntrys.size());
    const auto& rawGds = mGDSEntrys[i];
    return GDSEntry{
        rawGds.mHotspot,
        rawGds.mEntryDialog,
        rawGds.mExitDialog,
        GamePositionAndHeading{
            MakeGamePositionFromTileAndOffset(
                tile,
                rawGds.mExitOffset),
            rawGds.mExitHeading},
        rawGds.mWalkToDest
    };

}

template <typename S>
void GDSEntryFactory<S>::Load()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        sFilename);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto gdsNumber = fb.GetUint8();
        const auto gdsChar   = MakeHotspotChar(fb.GetUint8());
        fb.Skip(2);
        const auto entry = KeyTarget{fb.GetUint32LE()};
        const auto exit  = KeyTarget{fb.GetUint32LE()};
        const auto xoff  = fb.GetUint8();
        const auto yoff  = fb.GetUint8();
        // ... ???
        const auto heading = fb.GetUint16LE() >> 8;
        const auto walkToDest = fb.GetUint8();
        ASSERT(fb.GetUint16LE() == 0);

        mGDSEntrys.emplace_back(
            HotspotRef{gdsNumber, gdsChar},
            entry,
            exit,
            glm::vec<2, unsigned>(xoff, yoff),
            heading,
            walkToDest == 1);
    }
}

}
