#include "bak/gameData.hpp"

#include "bak/combat.hpp"
#include "bak/constants.hpp"
#include "bak/container.hpp"
#include "bak/dialogAction.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/inventory.hpp"
#include "bak/money.hpp"
#include "bak/resourceNames.hpp"
#include "bak/save.hpp"
#include "bak/spells.hpp"
#include "bak/timeExpiringState.hpp"
#include "bak/types.hpp"

#include "bak/state/encounter.hpp"
#include "bak/state/skill.hpp"

#include "com/bits.hpp"
#include "com/ostream.hpp"

#include <variant>

namespace BAK {

GameData::GameData(const std::string& save)
:
    mBuffer{FileBufferFactory::Get().CreateSaveBuffer(save)},
    mLogger{Logging::LogState::GetLogger("GameData")},
    mName{LoadSaveName(mBuffer)},
    mChapter{LoadChapter()},
    mMapLocation{LoadMapLocation()},
    mLocation{LoadLocation()},
    mTime{LoadWorldTime()},
    mParty{LoadParty()}
{
    mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
    LoadChapterOffsetP();
    //LoadCombatStats(0x914b, 1698);
    //LoadCombatClickedTimes();
}

std::vector<TimeExpiringState> GameData::LoadTimeExpiringState()
{
    // DialogAction value is always 4, flag is always 0x40
    // ItemExpiring value is always 1, flag is always 0x80
    // 0x80 adds 
    mBuffer.Seek(sTimeExpiringEventRecordOffset);
    auto storage = std::vector<TimeExpiringState>{};
    auto stateCount = mBuffer.GetUint16LE();
    for (unsigned i = 0; i < stateCount; i++)
    {
        auto type = mBuffer.GetUint8();
        auto flag = mBuffer.GetUint8();
        auto data= mBuffer.GetUint16LE();
        auto time = Time{mBuffer.GetUint32LE()};
        storage.emplace_back(TimeExpiringState{ExpiringStateType{type}, flag, data, time});
        mLogger.Spam() << storage.back() << "\n";
    }
    return storage;
}

SpellState GameData::LoadSpells()
{
    mBuffer.Seek(sActiveSpells);
    return SpellState{mBuffer.GetUint16LE()};
}

std::vector<SkillAffector> GameData::GetCharacterSkillAffectors(
    CharIndex character)
{
    mBuffer.Seek(GetCharacterAffectorsOffset(character.mValue));
    std::vector<SkillAffector> affectors{};
    for (unsigned i = 0; i < 8; i++)
    {
        // Strength Drain Spell does this...
        // 00 01 08 00 07 00 D8 70 02 00 B0 E1 04
        const auto type = mBuffer.GetUint16LE();
        if (type == 0)
        {
            mBuffer.Skip(12);
            continue;
        }
        const auto skill = ToSkill(static_cast<BAK::SkillTypeMask>(mBuffer.GetUint16LE()));
        const auto adjust = mBuffer.GetSint16LE();
        const auto startTime = Time{mBuffer.GetUint32LE()};
        const auto endTime = Time{mBuffer.GetUint32LE()};
        affectors.emplace_back(SkillAffector{type, skill, adjust, startTime, endTime});
    }
    return affectors;
}

/* ************* LOAD Game STATE ***************** */
Party GameData::LoadParty()
{
    auto characters = LoadCharacters();
    auto activeCharacters = LoadActiveCharacters();
    auto gold = LoadGold();
    auto keys = LoadCharacterInventory(sPartyKeyInventoryOffset);
    auto party = Party{
        gold,
        std::move(keys),
        characters,
        activeCharacters};
    return party;
}
    
std::vector<Character> GameData::LoadCharacters()
{
    unsigned characters = sCharacterCount;

    std::vector<Character> chars;

    for (unsigned character = 0; character < characters; character++)
    {
        mBuffer.Seek(GetCharacterNameOffset(character));
        auto name = mBuffer.GetString(sCharacterNameLength);

        mBuffer.Seek(GetCharacterSkillOffset(character));
        mLogger.Spam() << "Name: " << name << "@" 
            << std::hex << mBuffer.Tell() << std::dec << "\n";

        auto characterNameOffset = mBuffer.GetArray<2>();
        auto spells = Spells{mBuffer.GetArray<6>()};

        auto skills = LoadSkills(mBuffer);

        const auto pos = mBuffer.Tell();
        for (unsigned i = 0; i < Skills::sSkills; i++)
        {
            const auto selected = State::ReadSkillSelected(mBuffer, character, i);
            const auto unseenImprovement = State::ReadSkillUnseenImprovement(mBuffer, character, i);

            skills.GetSkill(static_cast<SkillType>(i)).mSelected = selected;
            skills.GetSkill(static_cast<SkillType>(i)).mUnseenImprovement = unseenImprovement;
        }
        mBuffer.Seek(pos);

        skills.SetSelectedSkillPool(skills.CalculateSelectedSkillPool());

        //bool characterIndex = mBuffer.GetUint8() != 0;
        auto unknown2 = mBuffer.GetArray<7>();
        mLogger.Spam() << " Finished loading : " << name << std::hex << mBuffer.Tell() << std::dec << "\n";
        // Load inventory
        auto inventory = LoadCharacterInventory(
            GetCharacterInventoryOffset(character));

        auto conditions = LoadConditions(character);

        chars.emplace_back(
            character,
            name,
            skills,
            spells,
            characterNameOffset,
            unknown2,
            conditions,
            std::move(inventory));

        auto affectors = GetCharacterSkillAffectors(CharIndex{character});
        for (const auto& affector : affectors)
        {
            chars.back().AddSkillAffector(affector);
        }
    }
    
    return chars;
}

Conditions GameData::LoadConditions(unsigned character)
{
    ASSERT(character < sCharacterCount);
    mBuffer.Seek(GetCharacterConditionOffset(character));

    auto conditions = Conditions{};
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
        conditions.mConditions[i] = mBuffer.GetUint8();
    return conditions;
}

unsigned GameData::LoadChapter()
{
    mBuffer.Seek(sChapterOffset);
    auto chapter = mBuffer.GetUint16LE();
    // next 4 uint16's might have something to do
    // with location displayed when chapter loaded
    mBuffer.Seek(0x64);
    auto chapterAgain = mBuffer.GetUint16LE();
    assert(chapter == chapterAgain);
    return chapterAgain;
}

Royals GameData::LoadGold()
{
    mBuffer.Seek(sGoldOffset);
    return Royals{mBuffer.GetUint32LE()};
}

std::vector<CharIndex> GameData::LoadActiveCharacters()
{
    mBuffer.Seek(sActiveCharactersOffset);
    const auto activeCharacters = mBuffer.GetUint8();

    auto active = std::vector<CharIndex>{};
    for (unsigned i = 0; i < activeCharacters; i++)
    {
        const auto c = mBuffer.GetUint8();
        active.emplace_back(c);
    }

    mLogger.Spam() << "Active Characters: " << active << "\n";

    return active;
}

MapLocation GameData::LoadMapLocation()
{
    mBuffer.Seek(sMapPositionOffset);
    auto posX = mBuffer.GetUint16LE();
    auto posY = mBuffer.GetUint16LE();
    auto heading = mBuffer.GetUint16LE();
    auto mapLocation = MapLocation{{posX, posY}, heading};
    mLogger.Info() << mapLocation << "\n";
    return mapLocation;
}
 
Location GameData::LoadLocation()
{
    mBuffer.Seek(sLocationOffset);

    unsigned zone = mBuffer.GetUint8();
    ASSERT(zone <= 12);
    mLogger.Info() << "LOADED: Zone:" << zone << std::endl;

    unsigned xtile = mBuffer.GetUint8();
    unsigned ytile = mBuffer.GetUint8();
    unsigned xpos = mBuffer.GetUint32LE();
    unsigned ypos = mBuffer.GetUint32LE();

    mLogger.Info() << "Unknown: " << mBuffer.GetArray<5>() << "\n";
    std::uint16_t heading = mBuffer.GetUint16LE();

    mLogger.Info() << "Tile: " << xtile << "," << ytile << std::endl;
    mLogger.Info() << "Pos: " << xpos << "," << ypos << std::endl;
    mLogger.Info() << "Heading: " << heading << std::endl;
    
    return Location{
        ZoneNumber{zone},
        {xtile, ytile},
        GamePositionAndHeading{
            GamePosition{xpos, ypos},
            heading}
    };
}

WorldClock GameData::LoadWorldTime()
{
    mBuffer.Seek(sTimeOffset);
    return WorldClock{
        Time{mBuffer.GetUint32LE()},
        Time{mBuffer.GetUint32LE()}};
}

Inventory GameData::LoadCharacterInventory(unsigned offset)
{
    mBuffer.Seek(offset);

    const auto itemCount = mBuffer.GetUint8();
    const auto capacity = mBuffer.GetUint16LE();
    mLogger.Spam() << " Items: " << +itemCount << " cap: " << capacity << "\n";
    return LoadInventory(mBuffer, itemCount, capacity);
}

std::vector<GenericContainer> GameData::LoadShops()
{
    mBuffer.Seek(sShopsOffset);
    auto shops = std::vector<GenericContainer>{};

    for (unsigned i = 0; i < sShopsCount; i++)
    {
        const unsigned address = mBuffer.Tell();
        mLogger.Spam() << " Container: " << i
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerGDSLocationTag>(mBuffer);
        shops.emplace_back(std::move(container));
        mLogger.Spam() << shops.back() << "\n";
    }

    return shops;
}

std::vector<GenericContainer> GameData::LoadContainers(unsigned zone)
{
    const auto& mLogger = Logging::LogState::GetLogger("GameData");
    mLogger.Spam() << "Loading containers for Z: " << zone << "\n";
    std::vector<GenericContainer> containers{};

    ASSERT(zone < sZoneContainerOffsets.size());
    const auto [offset, count] = sZoneContainerOffsets[zone];
    mBuffer.Seek(offset);

    for (unsigned j = 0; j < count; j++)
    {
        const unsigned address = mBuffer.Tell();
        mLogger.Spam() << " Container: " << j
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerWorldLocationTag>(mBuffer);
        containers.emplace_back(std::move(container));
        mLogger.Spam() << containers.back() << "\n";
    }

    return containers;
}

// add a memroy breakpoint on this?
void GameData::LoadChapterOffsetP()
{
    // I have no idea what these mean
    constexpr unsigned chapterOffsetsStart = 0x11a3;
    mBuffer.Seek(chapterOffsetsStart);

    mLogger.Spam() << "Chapter Offsets Start @" 
        << std::hex << chapterOffsetsStart << std::dec << std::endl;

    for (unsigned i = 0; i < 10; i++)
    {
        std::stringstream ss{};
        ss << "Chapter #" << i << " : " << mBuffer.GetUint16LE();
        for (unsigned i = 0; i < 5; i++)
        {
            unsigned addr = mBuffer.GetUint32LE();
            ss << " a: " << std::hex << addr << std::dec;
        }
        mLogger.Spam() << ss.str() << std::endl;
    }

    mLogger.Spam() << "Chapter Offsets End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
}

std::vector<CombatEntityList> GameData::LoadCombatEntityLists()
{
    mBuffer.Seek(sCombatEntityListOffset);

    mLogger.Spam() << "Combat Entity Lists Start @" 
        << std::hex << sCombatEntityListOffset << std::dec << std::endl;

    std::vector<CombatEntityList> data{};
    for (int i = 0; i < sCombatEntityListCount; i++)
    {
        auto& list = data.emplace_back();
        std::stringstream ss{};
        ss << " Combat #" << i;
        constexpr unsigned maxCombatants = 7;
        auto sep = ' ';
        for (unsigned i = 0; i < maxCombatants; i++)
        {
            auto combatant = mBuffer.GetUint16LE();
            if (combatant != 0xffff)
            {
                ss << sep << combatant;
                list.mCombatants.emplace_back(CombatantIndex{combatant});
            }
            sep = ',';
        }
        mLogger.Spam() << ss.str() << std::endl;
    }
    mLogger.Spam() << "Combat Entity Lists End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
    return data;
}

CombatEntityList GameData::LoadCombatEntityList(CombatIndex index)
{
    constexpr unsigned maxCombatants = 7;
    mBuffer.Seek(sCombatEntityListOffset + index.mValue * maxCombatants * 2);
    auto list = CombatEntityList{};

    for (unsigned i = 0; i < maxCombatants; i++)
    {
        auto combatant = mBuffer.GetUint16LE();
        if (combatant != 0xffff)
        {
            list.mCombatants.emplace_back(CombatantIndex{combatant});
        }
    }
    return list;
}

std::vector<Skills> GameData::LoadCombatStats()
{
    unsigned combatStatsStart = sCombatStatsOffset;
    mBuffer.Seek(combatStatsStart);
    mLogger.Spam() << "Combat Stats Start @" 
        << std::hex << combatStatsStart << std::dec << std::endl;

    std::vector<Skills> data{};
    // ends at 3070a
    for (unsigned i = 0; i < sCombatStatsCount; i++)
    {
        mLogger.Spam() << "Combat #" << std::dec << i 
            << " " << std::hex << mBuffer.Tell() << std::endl;
        mLogger.Spam() << std::hex << mBuffer.GetUint16LE() << std::endl << std::dec;
        auto spells = Spells(mBuffer.GetArray<6>());

        auto skills = LoadSkills(mBuffer);
        mLogger.Spam() << skills << "\n";
        mBuffer.Skip(7); // Conditions?
        data.emplace_back(std::move(skills));
    }
    mLogger.Spam() << "Combat Stats End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
    return data;
}

std::vector<CombatantGridLocation> GameData::LoadCombatantGridLocations()
{
    std::vector<CombatantGridLocation> data{};
    const auto initial = 0;
    mLogger.Spam() << "Loading Combat Grid Locations" << std::endl;
    mBuffer.Seek(sCombatantGridLocationsOffset + (initial * 22));
    for (unsigned i = 0; i < sCombatantGridLocationsCount; i++)
    {
        mBuffer.Skip(2);
        const auto monsterType = mBuffer.GetUint16LE();
        const auto gridX = mBuffer.GetUint8();
        const auto gridY = mBuffer.GetUint8();
        mBuffer.Skip(16);
        data.emplace_back(CombatantGridLocation{MonsterIndex{monsterType}, glm::uvec2{gridX, gridY}});
        mLogger.Spam() << "CGL #" << i << data.back() << "\n";
    }
    return data;
}

CombatantGridLocation GameData::LoadCombatantGridLocation(CombatIndex index)
{
    mBuffer.Seek(sCombatantGridLocationsOffset + (index.mValue * 22));
    mBuffer.Skip(2);
    const auto monsterType = mBuffer.GetUint16LE();
    const auto gridX = mBuffer.GetUint8();
    const auto gridY = mBuffer.GetUint8();
    mBuffer.Skip(16);
    return CombatantGridLocation{MonsterIndex{monsterType}, glm::uvec2{gridX, gridY}};
}

std::vector<CombatWorldLocation> GameData::LoadCombatWorldLocations()
{
    std::vector<CombatWorldLocation> data{};
    mBuffer.Seek(sCombatWorldLocationsOffset);
    for (unsigned k = 0; k < sCombatWorldLocationsCount; k++)
    {
        const auto x = mBuffer.GetUint32LE();
        const auto y = mBuffer.GetUint32LE();
        const auto heading = static_cast<std::uint16_t>(mBuffer.GetUint16LE() >> 8);
        const auto combatantPosition = GamePositionAndHeading{{x, y}, heading};
        const auto unknownFlag = mBuffer.GetUint8();
        const auto combatantState = mBuffer.GetUint8();
        data.emplace_back(CombatWorldLocation{combatantPosition, unknownFlag, combatantState});
        mLogger.Spam() << "CWL #" << k << " " << data.back() << "\n";
    }
    return data;
}

CombatWorldLocation GameData::LoadCombatWorldLocation(std::uint8_t tileIndex, std::uint8_t encounterIndex, std::uint8_t combatantRelativeIndex)
{
    std::vector<CombatWorldLocation> data{};
    static constexpr auto dataSize = 12;
    const auto offset = sCombatWorldLocationsOffset + (tileIndex * 35 * dataSize) + (encounterIndex * 7 * dataSize) + combatantRelativeIndex * dataSize;
    mBuffer.Seek(offset);
    const auto x = mBuffer.GetUint32LE();
    const auto y = mBuffer.GetUint32LE();
    const auto heading = static_cast<std::uint16_t>(mBuffer.GetUint16LE() >> 8);
    const auto combatantPosition = GamePositionAndHeading{{x, y}, heading};
    const auto unknownFlag = mBuffer.GetUint8();
    const auto combatantState = mBuffer.GetUint8();
    return CombatWorldLocation{combatantPosition, unknownFlag, combatantState};
}

std::vector<GenericContainer> GameData::LoadCombatInventories()
{
    mLogger.Spam() << "Loading Combat Inventories" << std::endl;
    mBuffer.Seek(sCombatInventoryOffset);
    std::vector<GenericContainer> containers{};

    // There are more combat inventories than there are
    // combatants for some reason. We should look them
    // up by combat and combatant number rather than by
    // index.
    for (unsigned i = 0; i < sCombatInventoryCount; i++)
    {
        auto loc = mBuffer.Tell();
        auto container = LoadGenericContainer<ContainerCombatLocationTag>(mBuffer);
        containers.emplace_back(std::move(container));
        mLogger.Spam() << "CobmatInventory #" << i << " @" << std::hex << loc << std::dec << " " << containers.back() << "\n";
    }

    return containers;
}

std::vector<Time> GameData::LoadCombatClickedTimes()
{
    mLogger.Spam() << "Loading Combat Clicked Times" << std::endl;
    std::vector<Time> times{};
    for (unsigned i = 0; i < 100; i++)
    {
        auto time = State::GetCombatClickedTime(mBuffer, i);
        times.emplace_back(time);
        if (time.mTime > 0)
        {
            mLogger.Spam() << "Combat #" << i << " time: " << time << "\n";
        }
    }
    return times;
}

}
