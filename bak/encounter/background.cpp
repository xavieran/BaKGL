#include "bak/encounter/background.hpp"

#include "xbak/FileBuffer.h"

#include <cassert>

namespace BAK {


BackgroundFactory::BackgroundFactory()
:
    mBackgrounds{}
{
    Load();
}

const Background& BackgroundFactory::Get(unsigned i) const
{
    assert(i < mBackgrounds.size());
    return mBackgrounds[i];
}

void BackgroundFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
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
        const auto heading = fb.GetUint16LE();
        const auto walkToDest = fb.GetUint8();
        assert(fb.GetUint16LE() == 0);

        mBackgrounds.emplace_back(
            HotspotRef{gdsNumber, gdsChar},
            entry,
            exit,
            glm::vec<2, unsigned>(xoff, yoff),
            heading,
            walkToDest == 1);
    }
}

}
