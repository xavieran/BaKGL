#include "bak/save/combat.hpp"

#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/state/encounter.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

std::vector<CombatEntityList> LoadCombatEntityLists(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatEntityLists");
    fb.Seek(SaveOffsets::sCombatEntityListOffset);

    logger.Spam() << "Combat Entity Lists Start @"
        << std::hex << SaveOffsets::sCombatEntityListOffset << std::dec << std::endl;

    std::vector<CombatEntityList> data{};
    for (int i = 0; i < SaveOffsets::sCombatEntityListCount; i++)
    {
        auto& list = data.emplace_back();
        std::stringstream ss{};
        ss << " Combat #" << i;
        constexpr unsigned maxCombatants = 7;
        auto sep = ' ';
        for (unsigned i = 0; i < maxCombatants; i++)
        {
            auto combatant = fb.GetUint16LE();
            if (combatant != 0xffff)
            {
                ss << sep << combatant;
                list.mCombatants.emplace_back(CombatantIndex{combatant});
            }
            sep = ',';
        }
        logger.Spam() << ss.str() << std::endl;
    }
    logger.Spam() << "Combat Entity Lists End @"
        << std::hex << fb.Tell() << std::dec << std::endl;
    return data;
}

CombatEntityList LoadCombatEntityList(FileBuffer& fb, CombatIndex index)
{
    constexpr unsigned maxCombatants = 7;
    fb.Seek(SaveOffsets::sCombatEntityListOffset + index.mValue * maxCombatants * 2);
    auto list = CombatEntityList{};

    for (unsigned i = 0; i < maxCombatants; i++)
    {
        auto combatant = fb.GetUint16LE();
        if (combatant != 0xffff)
        {
            list.mCombatants.emplace_back(CombatantIndex{combatant});
        }
    }
    return list;
}

std::vector<Skills> LoadCombatStats(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatStats");
    unsigned combatStatsStart = SaveOffsets::sCombatStatsOffset;
    fb.Seek(combatStatsStart);
    logger.Spam() << "Combat Stats Start @"
        << std::hex << combatStatsStart << std::dec << std::endl;

    std::vector<Skills> data{};
    for (unsigned i = 0; i < SaveOffsets::sCombatStatsCount; i++)
    {
        logger.Spam() << "Combat #" << std::dec << i
            << " " << std::hex << fb.Tell() << std::endl;
        logger.Spam() << std::hex << fb.GetUint16LE() << std::endl << std::dec;
        auto spells = Spells(fb.GetArray<6>());

        auto skills = LoadSkills(fb);
        logger.Spam() << skills << "\n";
        fb.Skip(7);
        data.emplace_back(std::move(skills));
    }
    logger.Spam() << "Combat Stats End @"
        << std::hex << fb.Tell() << std::dec << std::endl;
    return data;
}

std::vector<CombatantGridLocation> LoadCombatantGridLocations(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatantGridLocations");
    std::vector<CombatantGridLocation> data{};
    const auto initial = 0;
    logger.Spam() << "Loading Combat Grid Locations" << std::endl;
    fb.Seek(SaveOffsets::sCombatantGridLocationsOffset + (initial * 22));
    for (unsigned i = 0; i < SaveOffsets::sCombatantGridLocationsCount; i++)
    {
        fb.Skip(2);
        const auto monsterType = fb.GetUint16LE();
        const auto gridX = fb.GetUint8();
        const auto gridY = fb.GetUint8();
        fb.Skip(16);
        data.emplace_back(CombatantGridLocation{MonsterIndex{monsterType}, glm::uvec2{gridX, gridY}});
        logger.Spam() << "CGL #" << i << data.back() << "\n";
    }
    return data;
}

CombatantGridLocation LoadCombatantGridLocation(FileBuffer& fb, CombatIndex index)
{
    fb.Seek(SaveOffsets::sCombatantGridLocationsOffset + (index.mValue * 22));
    fb.Skip(2);
    const auto monsterType = fb.GetUint16LE();
    const auto gridX = fb.GetUint8();
    const auto gridY = fb.GetUint8();
    fb.Skip(16);
    return CombatantGridLocation{MonsterIndex{monsterType}, glm::uvec2{gridX, gridY}};
}

std::vector<CombatWorldLocation> LoadCombatWorldLocations(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatWorldLocations");
    std::vector<CombatWorldLocation> data{};
    fb.Seek(SaveOffsets::sCombatWorldLocationsOffset);
    for (unsigned k = 0; k < SaveOffsets::sCombatWorldLocationsCount; k++)
    {
        const auto x = fb.GetUint32LE();
        const auto y = fb.GetUint32LE();
        const auto heading = static_cast<std::uint16_t>(fb.GetUint16LE() >> 8);
        const auto combatantPosition = GamePositionAndHeading{{x, y}, heading};
        const auto imageIndex = fb.GetUint8();
        const auto combatantState = fb.GetUint8();
        data.emplace_back(CombatWorldLocation{combatantPosition, imageIndex, combatantState});
        logger.Spam() << "CWL #" << k << " " << data.back() << "\n";
    }
    return data;
}

std::vector<Time> LoadCombatClickedTimes(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatClickedTimes");
    logger.Spam() << "Loading Combat Clicked Times" << std::endl;
    std::vector<Time> times{};
    for (unsigned i = 0; i < 100; i++)
    {
        auto time = State::GetCombatClickedTime(fb, i);
        times.emplace_back(time);
        if (time.mTime > 0)
        {
            logger.Spam() << "Combat #" << i << " time: " << time << "\n";
        }
    }
    return times;
}

}
