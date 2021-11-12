#pragma once

#include "bak/dialog.hpp"
#include "bak/encounter/combat.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

namespace BAK::Encounter {

template <bool isTrap>
GenericCombatFactory<isTrap>::GenericCombatFactory()
:
    mCombats{}
{
    Load();
}

template <bool isTrap>
const Combat& GenericCombatFactory<isTrap>::Get(unsigned i) const
{
    ASSERT(i < mCombats.size());
    return mCombats[i];
}

template <bool isTrap>
void GenericCombatFactory<isTrap>::Load()
{
    auto fb = FileBufferFactory::CreateFileBuffer(
        isTrap ? sTrapFilename : sCombatFilename);

    auto store = DialogStore{};
    const auto logger = Logging::LogState::GetLogger("Combat");

    const auto count = fb.GetUint16LE();
    for (unsigned i = 0; i < count; i++)
    {
        fb.Skip(5);
        const auto combatIndex = fb.GetUint32LE();
        const auto entryDialog = fb.GetUint32LE();
        const auto scoutDialog = fb.GetUint32LE();

        const auto zero = fb.GetUint32LE();

        const auto GetPosAndHeading = [&fb]{
            const auto x = fb.GetUint32LE();
            const auto y = fb.GetUint32LE();
            const auto heading = fb.GetUint16LE();
            return GamePositionAndHeading{{x, y}, heading};
        };

        
        const auto trap = std::invoke([&]() -> std::optional<GamePositionAndHeading> 
        {
            if (isTrap)
                return GetPosAndHeading();
            else
                return std::optional<GamePositionAndHeading>{};
        });

        const auto north = GetPosAndHeading();
        const auto west  = GetPosAndHeading();
        const auto south = GetPosAndHeading();
        const auto east  = GetPosAndHeading();

        const auto numEnemies = fb.GetUint8();
        auto combatants = std::vector<unsigned>{};

        for (unsigned i = 0; i < numEnemies; i++)
        {
            combatants.emplace_back(fb.GetUint8());
            fb.Skip(47);
        }
        constexpr unsigned maxCombatants = 7;
        for (unsigned i = 0; i < (maxCombatants - numEnemies); i++)
        {
            fb.Skip(48);
        }
        fb.Skip(2);
    

        logger.Spam() << "Index: " << i << " COMBAT #" << combatIndex << " ";
        try
        {
            logger.Spam() << store.GetFirstText(store.GetSnippet(KeyTarget{entryDialog})) << "\n";
        }
        catch (const std::runtime_error& e)
        {
            // Only Key 133 is missing... on Combat #417
            logger.Debug() << e.what() << "\n";
        }

        mCombats.emplace_back(
            combatIndex,
            KeyTarget{entryDialog},
            KeyTarget{scoutDialog},
            trap,
            north,
            west,
            south,
            east,
            combatants);
    }
}

}
