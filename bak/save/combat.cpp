#include "bak/save/combat.hpp"

#include "bak/save/saveOffsets.hpp"
#include "bak/spells.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/state/encounter.hpp"

#include "com/logger.hpp"

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
        const auto unknown0 = fb.GetUint16LE();
        const auto monsterType = fb.GetUint16LE();
        const auto gridX = fb.GetUint8();
        const auto gridY = fb.GetUint8();
        const auto unknown1 = fb.GetUint16LE();
        const auto unknown2 = fb.GetUint8();
        const auto rest = fb.GetArray<13>();
        data.emplace_back(CombatantGridLocation{unknown0, MonsterIndex{monsterType}, glm::uvec2{gridX, gridY}, unknown1, unknown2, rest});
        logger.Spam() << "CGL #" << i << data.back() << "\n";
    }
    return data;
}

void Save(const std::vector<CombatantGridLocation>& cgls, FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sCombatantGridLocationsOffset);
    assert(cgls.size() == SaveOffsets::sCombatantGridLocationsCount);
    for (const auto& cgl : cgls)
    {
        fb.PutUint16LE(cgl.mUnknown0);
        fb.PutUint16LE(cgl.mMonster.mValue);
        fb.PutUint8(cgl.mGridPos.x);
        fb.PutUint8(cgl.mGridPos.y);
        fb.PutUint16LE(cgl.mUnknown1);
        fb.PutUint8(cgl.mUnknown2);
        for (auto val : cgl.mRest)
        {
            fb.PutUint8(val);
        }
    }
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
        const auto combatantState = CombatantWorldState{fb.GetUint8()};
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

void Save(const std::vector<CombatEntityList>& cels, FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sCombatEntityListOffset);
    assert(cels.size() == SaveOffsets::sCombatEntityListCount);
    constexpr unsigned maxCombatants = 7;
    for (const auto& cel : cels)
    {
        unsigned i = 0;
        for (const auto& combatant : cel.mCombatants)
        {
            fb.PutUint16LE(combatant.mValue);
            i++;
        }
        for (; i < maxCombatants; i++)
        {
            fb.PutUint16LE(0xffff);
        }
    }
}

void Save(const std::vector<CombatWorldLocation>& cwls, FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sCombatWorldLocationsOffset);
    assert(cwls.size() == SaveOffsets::sCombatWorldLocationsCount);
    for (const auto& cwl : cwls)
    {
        fb.PutUint32LE(cwl.mPosition.mPosition.x);
        fb.PutUint32LE(cwl.mPosition.mPosition.y);
        fb.PutUint16LE(cwl.mPosition.mHeading << 8);
        fb.PutUint8(cwl.mImageIndex);
        fb.PutUint8(std::to_underlying(cwl.mState));
    }
}

}

