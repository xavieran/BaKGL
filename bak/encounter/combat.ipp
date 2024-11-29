#pragma once

#include "bak/dialog.hpp"
#include "bak/encounter/combat.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "bak/fileBufferFactory.hpp"

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

    const auto count = fb.GetUint32LE();
    logger.Spam() << "Combats: " << count <<"\n";
    for (unsigned i = 0; i < count; i++)
    {
        logger.Spam() << "Combat #" << i << " @" 
            << std::hex << fb.Tell() << std::dec << "\n";
        fb.Skip(3);
        const auto combatIndex = fb.GetUint32LE();
        const auto entryDialog = fb.GetUint32LE();
        const auto scoutDialog = fb.GetUint32LE();

        const auto zero = fb.GetUint32LE();
        assert(zero == 0);

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
            std::uint32_t minX = fb.GetUint32LE();
            std::uint32_t maxX = fb.GetUint32LE();
            fb.Skip(8);
            //fb.DumpAndSkip(8);
            std::uint32_t minY = fb.GetUint32LE();
            std::uint32_t maxY = fb.GetUint32LE();
            fb.Skip(10);
            //fb.DumpAndSkip(8);
            //fb.DumpAndSkip(2);
            logger.Spam() << "Combatant #" << i << " " << monsterIndex
                << " mvTp: " << movementType << " pos: " << pos << "\n";
            combatants.emplace_back(monsterIndex, movementType, pos, glm::ivec2{minX, minY}, glm::ivec2{maxX, maxY});
        }

        constexpr unsigned maxCombatants = 7;
        for (unsigned i = 0; i < (maxCombatants - numEnemies); i++)
        {
            fb.Skip(48);
        }

        auto unknown = fb.GetUint16LE();
        const bool isAmbush = fb.GetUint16LE() == 0x1;

        mCombats.emplace_back(
            combatIndex,
            KeyTarget{entryDialog},
            KeyTarget{scoutDialog},
            trap,
            north,
            west,
            south,
            east,
            combatants,
            unknown,
            isAmbush);

        logger.Spam() << "Index: " << i << " COMBAT #" << combatIndex << " ";
        logger.Spam() << mCombats.back() << "\n";
    }
}

}
