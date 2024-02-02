#include "bak/encounter/dialog.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

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
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        sFilename);

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto target = fb.GetUint32LE();
        mDialogs.emplace_back(KeyTarget{target});
        fb.Skip(2);
    }
}

}
