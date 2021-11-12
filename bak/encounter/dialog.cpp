#include "bak/encounter/dialog.hpp"

#include "com/assert.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const Dialog& dialog)
{
    os << "Dialog { " << std::hex << dialog.mDialog << std::dec << "}";
    return os;
}

DialogFactory::DialogFactory()
:
    mDialogs{}
{
    Load();
}

const Dialog& DialogFactory::Get(unsigned i) const
{
    ASSERT(i < mDialogs.size());
    return mDialogs[i];
}

void DialogFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        sFilename);

    const auto count = fb.GetUint16LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(5);
        const auto target = fb.GetUint32LE();
        mDialogs.emplace_back(KeyTarget{target});
    }
}

}
