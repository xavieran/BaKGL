#include "bak/encounter/trap.hpp"

#include "bak/dialog.hpp"
#include "com/logger.hpp"
#include "xbak/FileBuffer.h"

#include <cassert>

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const Trap& trap)
{
    os << "Trap { #" << trap.mCombatIndex
        << " Entry: " << trap.mEntryDialog 
        << "}";
    return os;
}


TrapFactory::TrapFactory()
:
    mTraps{}
{
    Load();
}

const Trap& TrapFactory::Get(unsigned i) const
{
    assert(i < mTraps.size());
    return mTraps[i];
}

void TrapFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        sFilename);

    auto store = DialogStore{};
    const auto logger = Logging::LogState::GetLogger("Trap");

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);

        const auto trapIndex = fb.GetUint32LE();
        const auto entryDialog = fb.GetUint32LE();
        // prety sure you cant scout a trap??
        const auto scoutDialog = fb.GetUint32LE();
        fb.Skip(395);


        logger.Spam() << "Index: " << i << " Trap #" << trapIndex << " ";
        try
        {
            logger.Spam() << store.GetFirstText(store.GetSnippet(KeyTarget{entryDialog})) << "\n";
        }
        catch (const std::runtime_error& e)
        {
            logger.Spam() << e.what() << "\n";
        }

        mTraps.emplace_back(
            trapIndex,
            KeyTarget{entryDialog});
    }
}

}
