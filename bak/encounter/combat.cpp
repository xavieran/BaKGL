#include "bak/encounter/combat.hpp"

#include "bak/dialog.hpp"
#include "com/logger.hpp"
#include "xbak/FileBuffer.h"

#include <cassert>

namespace BAK::Encounters {

CombatFactory::CombatFactory()
:
    mCombats{}
{
    Load();
}

const Combat& CombatFactory::Get(unsigned i) const
{
    assert(i < mCombats.size());
    return mCombats[i];
}

void CombatFactory::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        sFilename);

    auto store = DialogStore{};
    const auto logger = Logging::LogState::GetLogger("Combat");

    const auto count = fb.GetUint32LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(3);
        const auto combatIndex = fb.GetUint32LE();
        const auto entryDialog = fb.GetUint32LE();
        const auto scoutDialog = fb.GetUint32LE();
        fb.Skip(385);


        logger.Debug() << "Index: " << i << " COMBAT #" << combatIndex << " ";
        try
        {
            logger.Debug() << store.GetFirstText(store.GetSnippet(KeyTarget{entryDialog})) << "\n";
        }
        catch (const std::runtime_error& e)
        {
            // Only Key 133 is missing... on Combat #417
            logger.Debug() << e.what() << "\n";
        }

        mCombats.emplace_back(
            combatIndex,
            KeyTarget{entryDialog},
            KeyTarget{scoutDialog});
    }
}

}
