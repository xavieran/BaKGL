#include "bak/gameData.hpp"
#include "bak/save.hpp"
#include "bak/spells.hpp"

#include "bak/state/encounter.hpp"
#include "bak/state/skill.hpp"

#include "bak/container.hpp"
#include "bak/inventory.hpp"

#include "bak/timeExpiringState.hpp"
#include "com/bits.hpp"
#include "com/ostream.hpp"

#include <variant>

namespace BAK {

GameData::GameData(const std::string& save)
:
    mBuffer{FileBufferFactory::Get().CreateSaveBuffer(save)},
    mLogger{Logging::LogState::GetLogger("GameData")},
    mName{LoadSaveName(mBuffer)},
    mObjects{},
    mChapter{LoadChapter()},
    mLocation{LoadLocation()},
    mTime{LoadWorldTime()},
    mParty{LoadParty()}
{

    mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
    //mLogger.Info() << mParty << "\n";
    mLogger.Info() << mTime << std::hex << " " << mTime.GetTime().mTime  << std::dec << "\n";
    //LoadContainers(0x1);
    //LoadContainers(0x2);
    //LoadContainers(0x3);
    //LoadContainers(0x4);
    //LoadContainers(0x5);
    //LoadContainers(0x6);
    //LoadContainers(0x7);
    //LoadContainers(0x8);
    //LoadContainers(0x9);
    //LoadContainers(0xa);
    //LoadContainers(0xb);
    //LoadContainers(0xc);
    //mLogger.Debug() << "Loaded Z12 Cont: " << std::hex 
    //    << mBuffer.Tell() << std::dec << "\n";
    //LoadShops();
    //LoadChapterOffsetP();
    //LoadCombatEntityLists();
    //LoadCombatStats(0x914b, 1698);
    //LoadCombatGridLocations();
    //LoadCombatWorldLocations();
    //LoadCombatClickedTimes();
    LoadTimeExpiringState();
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
        mLogger.Info() << storage.back() << "\n";
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
    mLogger.Debug() << "Party: " << party << "\n";
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
        mLogger.Debug() << "Name: " << name << "@" 
            << std::hex << mBuffer.Tell() << std::dec << "\n";

        auto characterNameOffset = mBuffer.GetArray<2>();
        auto spells = Spells{mBuffer.GetArray<6>()};

        auto skills = Skills{};

        for (unsigned i = 0; i < Skills::sSkills; i++)
        {
            const auto max        = mBuffer.GetUint8();
            const auto trueSkill  = mBuffer.GetUint8();
            const auto current    = mBuffer.GetUint8();
            const auto experience = mBuffer.GetUint8();
            const auto modifier   = mBuffer.GetSint8();

            const auto pos = mBuffer.Tell();

            const auto selected = State::ReadSkillSelected(mBuffer, character, i);
            const auto unseenIprovement = State::ReadSkillUnseenImprovement(mBuffer, character, i);

            skills.SetSkill(static_cast<SkillType>(i), Skill{
                max,
                trueSkill,
                current,
                experience,
                modifier,
                selected,
                unseenIprovement
            });

            mBuffer.Seek(pos);
        }

        skills.SetSelectedSkillPool(skills.CalculateSelectedSkillPool());

        //bool characterIndex = mBuffer.GetUint8() != 0;
        auto unknown2 = mBuffer.GetArray<7>();
        mLogger.Info() << " Finished loading : " << name << std::hex << mBuffer.Tell() << std::dec << "\n";
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

    mLogger.Debug() << "Active Characters: " << active << "\n";

    return active;
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
        zone,
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

    mLogger.Debug() << "Chapter Offsets Start @" 
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
        mLogger.Debug() << ss.str() << std::endl;
    }

    mLogger.Debug() << "Chapter Offsets End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
}

void GameData::LoadCombatEntityLists()
{
    mBuffer.Seek(sCombatEntityListOffset);

    mLogger.Info() << "Combat Entity Lists Start @" 
        << std::hex << sCombatEntityListOffset << std::dec << std::endl;

    for (int i = 0; i < sCombatEntityListCount; i++)
    {
        std::stringstream ss{};
        ss << " Combat #" << i;
        constexpr unsigned maxCombatants = 7;
        auto sep = ' ';
        for (unsigned i = 0; i < maxCombatants; i++)
        {
            auto combatant = mBuffer.GetUint16LE();
            if (combatant != 0xffff)
                ss << sep << combatant;
            sep = ',';
        }
        mLogger.Info() << ss.str() << std::endl;
    }

    mLogger.Info() << "Combat Entity Lists End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
}

void GameData::LoadCombatStats(unsigned offset, unsigned num)
{
    unsigned combatStatsStart = offset;
    mBuffer.Seek(combatStatsStart);
    mLogger.Info() << "Combat Stats Start @" 
        << std::hex << combatStatsStart << std::dec << std::endl;

    // ends at 3070a
    for (unsigned i = 0; i < num; i++)
    {
        mLogger.Info() << "Combat #" << std::dec << i 
            << " " << std::hex << mBuffer.Tell() << std::endl;
        mLogger.Info() << std::hex << mBuffer.GetUint16LE() << std::endl << std::dec;
        auto spells = Spells(mBuffer.GetArray<6>());

        std::stringstream ss{""};
        for (const auto& stat : {
            "Health", "Stamina", "Speed", "Strength", 
            "Defense", "Crossbow", "Melee", "Cast",
            "Assess", "Armor", "Weapon", "Bard",
            "Haggle", "Lockpick", "Scout", "Stealth"})
        {
            ss << std::dec << stat << ": " << +mBuffer.GetUint8() << " " 
                << +mBuffer.GetUint8() << " " << +mBuffer.GetUint8() << " ";
            mBuffer.Skip(2);
        }
        mBuffer.Skip(7); // Conditions?
        mLogger.Info() << ss.str() << std::endl;
    }
    mLogger.Info() << "Combat Stats End @" 
        << std::hex << mBuffer.Tell() << std::dec << std::endl;
}

void GameData::LoadCombatGridLocations()
{
    const auto initial = 0;
    mLogger.Info() << "Loading Combat Grid Locations" << std::endl;
    mBuffer.Seek(sCombatGridLocationsOffset + (initial * 22));
    for (unsigned i = 0; i < sCombatGridLocationsCount; i++)
    {
        mBuffer.Skip(2);
        const auto monsterType = mBuffer.GetUint16LE();
        const auto gridX = mBuffer.GetUint8();
        const auto gridY = mBuffer.GetUint8();
        mBuffer.Skip(16);

        mLogger.Info() << "Combat #" << i << " monster: " << monsterType <<
            " grid: " << glm::uvec2{gridX, gridY} << "\n";
    }
}

void GameData::LoadCombatWorldLocations()
{
    mBuffer.Seek(sCombatWorldLocationsOffset);
    for (unsigned k = 0; k < sCombatWorldLocationsCount; k++)
    {
        const auto x = mBuffer.GetUint32LE();
        const auto y = mBuffer.GetUint32LE();
        const auto heading = static_cast<std::uint16_t>(mBuffer.GetUint16LE() >> 8);
        const auto combatantPosition = GamePositionAndHeading{{x, y}, heading};
        const auto unknownFlag = mBuffer.GetUint8();
        // 0 - invisible?
        // 1 - invisible?
        // 2 - moving
        // 3 - moving
        // 4 - dead
        const auto combatantState = mBuffer.GetUint8();
        mLogger.Info() << "Combatant: " << k << " Position: " << combatantPosition << 
            " unknown: " << + unknownFlag <<
            " state: " << +combatantState << std::endl;
    }
}

std::vector<GenericContainer> GameData::LoadCombatInventories()
{
    mLogger.Debug() << "Loading Combat Inventories" << std::endl;
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
        mLogger.Debug() << "CobmatInventory #" << i << " @" << std::hex << loc << std::dec << " " << containers.back() << "\n";
    }

    return containers;
}

void GameData::LoadCombatClickedTimes()
{
    mLogger.Debug() << "Loading Combat Clicked Times" << std::endl;
    for (unsigned i = 0; i < 100; i++)
    {
        auto time = State::GetCombatClickedTime(mBuffer, i);
        if (time.mTime > 0)
        {
            mLogger.Debug() << "Combat #" << i << " time: " << time << "\n";
        }
    }
}

}
