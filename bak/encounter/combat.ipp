#pragma once

#include "bak/dialog.hpp"
#include "bak/encounter/combat.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "bak/fileBuffer.hpp"

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
    auto fb = FileBufferFactory::Get().CreateDataBuffer(
        isTrap ? sTrapFilename : sCombatFilename);

    const auto logger = Logging::LogState::GetLogger("Combat");

    const auto count = fb.GetUint16LE();
    logger.Debug() << "Combats: " << count <<"\n";
    for (unsigned i = 0; i < count; i++)
    {
        logger.Debug() << "Combat #" << i << " @" 
            << std::hex << fb.Tell() << std::dec << "\n";
        fb.Skip(5);
        const auto combatIndex = fb.GetUint32LE();
        const auto entryDialog = fb.GetUint32LE();
        const auto scoutDialog = fb.GetUint32LE();

        const auto zero = fb.GetUint32LE();

        const auto GetPosAndHeading = [&fb]{
            const auto x = fb.GetUint32LE();
            const auto y = fb.GetUint32LE();
            const auto heading = static_cast<std::uint16_t>(fb.GetUint16LE() >> 8);
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
        auto combatants = std::vector<CombatantData>{};

        for (unsigned i = 0; i < numEnemies; i++)
        {
            const auto monsterIndex = fb.GetUint16LE();
            const auto movementType = fb.GetUint16LE();
            const auto pos = GetPosAndHeading();
            combatants.emplace_back(monsterIndex, movementType, pos);
            fb.Skip(48 - 14);
        }

        constexpr unsigned maxCombatants = 7;
        for (unsigned i = 0; i < (maxCombatants - numEnemies); i++)
        {
            fb.Skip(48);
        }
        fb.Skip(2);

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

        logger.Spam() << "Index: " << i << " COMBAT #" << combatIndex << " ";
        logger.Spam() << mCombats.back() << "\n";
    }
}

}
