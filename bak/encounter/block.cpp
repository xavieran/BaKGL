#include "bak/encounter/block.hpp"

#include "xbak/FileBuffer.h"

#include <cassert>

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const Block& block)
{
    os << "Block { " << std::hex << block.mDialog << std::dec << "}";
    return os;
}

BlockFactory::BlockFactory()
:
    mBlocks{}
{
    Load();
}


const Block& BlockFactory::Get(unsigned i) const
{
    assert(i < mBlocks.size());
    return mBlocks[i];
}

void BlockFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        sFilename);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto target = fb.GetUint32LE();
        assert(fb.GetUint16LE() == 0);
        mBlocks.emplace_back(KeyTarget{target});
    }
}

}
