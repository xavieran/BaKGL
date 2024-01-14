#include "bak/gameData.hpp"
#include "bak/save.hpp"
#include "bak/spells.hpp"

#include "bak/container.hpp"
#include "bak/inventory.hpp"

#include "com/bits.hpp"
#include "com/ostream.hpp"

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
    mLogger.Info() << mTime << std::hex << " " << mTime.mTime.mTime << std::dec << "\n";
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
    LoadShops();
    LoadCombatEntityLists();
    LoadCombatStats(0x914b, 1698);
    LoadCombatGridLocations();
    LoadCombatWorldLocations();
}


std::pair<unsigned, unsigned> GameData::CalculateComplexEventOffset(unsigned eventPtr) const
{
    const auto source = (eventPtr + 0x2540) & 0xffff;
    const auto byteOffset = source / 10;
    const auto bitOffset = source % 10 != 0
        ? (source % 10) - 1
        : 0;
        
    mLogger.Debug() << __FUNCTION__ << std::hex << " " << eventPtr << " ("
        << byteOffset + sGameComplexEventRecordOffset << ", " 
        << bitOffset << ")\n" << std::dec;
    return std::make_pair(
        byteOffset + sGameComplexEventRecordOffset,
        bitOffset);
}

std::pair<unsigned, unsigned> GameData::CalculateEventOffset(unsigned eventPtr) const
{
    const unsigned startOffset = sGameEventRecordOffset;
    const unsigned bitOffset = eventPtr & 0xf;
    const unsigned byteOffset = (0xfffe & (eventPtr >> 3)) + startOffset;
    mLogger.Debug() << __FUNCTION__ << std::hex << " " << eventPtr << " ("
        << byteOffset << ", " << bitOffset << ")\n" << std::dec;
    return std::make_pair(byteOffset, bitOffset);
}

void GameData::SetBitValueAt(unsigned byteOffset, unsigned bitOffset, unsigned value)
{
    mBuffer.Seek(byteOffset);
    const auto originalData = mBuffer.GetUint16LE();
    const auto data = SetBit(originalData, bitOffset, value != 0);

    mBuffer.Seek(byteOffset);
    mBuffer.PutUint16LE(data);

    mLogger.Debug() << __FUNCTION__ << std::hex << 
        " " << byteOffset << " " << bitOffset 
        << " original[" << +originalData << "] new[" << +data  <<"]\n" << std::dec;
}

void GameData::SetEventFlag(unsigned eventPtr, unsigned value)
{
    mLogger.Debug() << __FUNCTION__ << " " << std::hex << eventPtr 
        << " to: " << value << std::dec << "\n";
    if (eventPtr >= 0xdac0)
    {
        const auto [byteOffset, bitOffset] = CalculateComplexEventOffset(eventPtr);
        SetBitValueAt(byteOffset, bitOffset, value);
    }
    else
    {
        const auto [byteOffset, bitOffset] = CalculateEventOffset(eventPtr);
        SetBitValueAt(byteOffset, bitOffset, value);
    }
}

void GameData::SetEventFlagTrue (unsigned eventPtr)
{
    SetEventFlag(eventPtr, 1);
}

void GameData::SetEventFlagFalse(unsigned eventPtr)
{
    SetEventFlag(eventPtr, 0);
}

void GameData::SetEventDialogAction(const SetFlag& setFlag)
{
    if (setFlag.mEventPointer >= 0xdac0
        && setFlag.mEventPointer % 10 == 0)
    {
        const auto offset = std::get<0>(CalculateComplexEventOffset(setFlag.mEventPointer));
        mBuffer.Seek(offset);
        const auto data = mBuffer.GetUint8();
        const auto newData = ((data & setFlag.mEventMask) 
            | setFlag.mEventData)
            ^ setFlag.mAlwaysZero;
        mBuffer.Seek(offset);

        mLogger.Debug() << __FUNCTION__ << std::hex << 
            " " << setFlag << " offset: " << offset 
            << " data[" << +data << "] new[" << +newData <<"]\n" << std::dec;
        mBuffer.PutUint8(newData);
    }
    else
    {
        if (setFlag.mEventPointer != 0)
            SetEventFlag(setFlag.mEventPointer, setFlag.mEventValue);

        // TREAT THIS AS UINT16_T !!! EVENT MASK + EVENT DATA
        if (setFlag.mEventMask != 0)
            SetEventFlag(setFlag.mEventMask, setFlag.mEventValue);

        if (setFlag.mAlwaysZero != 0)
            SetEventFlag(setFlag.mAlwaysZero, setFlag.mEventValue);
    }
}

unsigned GameData::ReadBitValueAt(unsigned byteOffset, unsigned bitOffset) const
{
    mBuffer.Seek(byteOffset);
    const unsigned eventData = mBuffer.GetUint16LE();
    const unsigned bitValue = eventData >> bitOffset;
    mLogger.Debug() << __FUNCTION__ << std::hex << 
        " " << byteOffset << " " << bitOffset 
        << " [" << +bitValue << "]\n" << std::dec;
    return bitValue;
}

unsigned GameData::ReadEvent(unsigned eventPtr) const
{
    if (eventPtr >= 0xdac0)
    {
        const auto [byteOffset, bitOffset] = CalculateComplexEventOffset(eventPtr);
        return ReadBitValueAt(byteOffset, bitOffset);
    }
    else
    {
        const auto [byteOffset, bitOffset] = CalculateEventOffset(eventPtr);
        return ReadBitValueAt(byteOffset, bitOffset);
    }
}

bool GameData::ReadEventBool(unsigned eventPtr) const
{
    return (ReadEvent(eventPtr) & 0x1) == 1;
}

bool GameData::ReadSkillSelected(unsigned character, unsigned skill) const
{
    constexpr auto maxSkills = 0x11;
    return ReadEventBool(
        sSkillSelectedEventFlag
        + (character * maxSkills)
        + skill);
}

bool GameData::ReadSkillUnseenImprovement(unsigned character, unsigned skill) const
{
    constexpr auto maxSkills = 0x11;
    return ReadEventBool(
        sSkillImprovementEventFlag
        + (character * maxSkills)
        + skill);
}

std::uint8_t GameData::ReadSelectedSkillPool(unsigned character) const
{
    mBuffer.Seek(sCharacterSelectedSkillPool + (1 << character));
    return mBuffer.GetUint8();
}

void GameData::SetSelectedSkillPool(unsigned character, std::uint8_t value)
{
    mBuffer.Seek(sCharacterSelectedSkillPool + (1 << character));
    return mBuffer.PutUint8(value);
}

void GameData::ClearUnseenImprovements(unsigned character)
{
    constexpr auto maxSkills = 0x11;
    for (unsigned i = 0; i < maxSkills; i++)
    {
        const auto flag = 
            sSkillImprovementEventFlag
            + (character * maxSkills)
            + i;

        SetEventFlagFalse(flag);
    }
}

bool GameData::ReadTempleSeen(unsigned temple) const
{
    return ReadEventBool(sTempleSeenFlag + temple);
}

void GameData::SetTempleSeen(unsigned temple)
{
    SetEventFlagTrue(sTempleSeenFlag + temple);
}

void GameData::SetTimeExpiringState(
    unsigned number,
    unsigned eventPtr,
    unsigned flag,
    Time time)
{
    // DialogAction value is always 4, flag is always 0x40
    // ItemExpiring value is always 1, flag is always 0x80
    // 0x80 adds 
    unsigned index = 0;
    mBuffer.Seek(sTimeExpiringEventRecordOffset + index);
    mBuffer.PutUint8(number);
    mBuffer.PutUint8(flag);
    mBuffer.PutUint16LE(eventPtr);
    mBuffer.PutUint32LE(time.mTime);
}

// Called by checkBlockTriggered, checkTownTriggered, checkBackgroundTriggered, checkZoneTriggered,
// doEnableEncounter, doDialogEncounter, doDisableEncounter, doSoundEncounter
bool GameData::CheckActive(
    const Encounter::Encounter& encounter,
    ZoneNumber zone) const
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlagOffset(
        zone,
        encounter.GetTileIndex(),
        encounterIndex);
    const bool encounterFlag1450 = ReadEventBool(
        CalculateRecentEncounterStateFlag(encounterIndex));
    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? (ReadEventBool(encounter.mSaveAddress) == 1)
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? ReadEventBool(encounter.mSaveAddress2)
        : false;
    return !(alreadyEncountered
        || encounterFlag1450
        || eventFlag1
        || eventFlag2);
}

bool GameData::CheckCombatActive(
    const Encounter::Encounter& encounter,
    ZoneNumber zone) const
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlagOffset(
        zone,
        encounter.GetTileIndex(),
        encounterIndex);

    constexpr auto combatIndex = 0;
    // If this flag is not set then this combat hasn't been seen
    const bool encounterFlag1464 = !CheckCombatEncounterStateFlag(combatIndex);

    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? (ReadEventBool(encounter.mSaveAddress) == 1)
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? ReadEventBool(encounter.mSaveAddress2)
        : false;
    return !(alreadyEncountered
        || encounterFlag1464
        || eventFlag1
        || eventFlag2);
}

void GameData::SetPostDialogEventFlags(
    const Encounter::Encounter& encounter,
    ZoneNumber zone)
{
    const auto tileIndex = encounter.GetTileIndex();
    const auto encounterIndex = encounter.GetIndex().mValue;

    if (encounter.mSaveAddress3 != 0)
    {
        SetEventFlagTrue(encounter.mSaveAddress3);
    }

    // Unknown 3 flag is associated with events like the sleeping glade and 
    // timirianya danger zone (effectively, always encounter this encounter)
    if (encounter.mUnknown3 == 0)
    {
        // Inhibit for this chapter
        if (encounter.mUnknown2 != 0)
        {
            SetEventFlagTrue(
                CalculateUniqueEncounterStateFlagOffset(
                    zone,
                    tileIndex,
                    encounterIndex));
        }

        // Inhibit for this tile
        SetEventFlagTrue(
            CalculateRecentEncounterStateFlag(
                encounterIndex));
    }

}

// Background and Town
void GameData::SetPostGDSEventFlags(
    const Encounter::Encounter& encounter)
{
    if (encounter.mSaveAddress3 != 0)
        SetEventFlagTrue(encounter.mSaveAddress3);
}

// Used by Block, Disable, Enable, Sound, Zone
void GameData::SetPostEnableOrDisableEventFlags(
    const Encounter::Encounter& encounter,
    ZoneNumber zone)
{
    if (encounter.mSaveAddress3 != 0)
    {
        SetEventFlagTrue(encounter.mSaveAddress3);
    }

    if (encounter.mUnknown2 != 0)
    {
        SetEventFlagTrue(
            CalculateUniqueEncounterStateFlagOffset(
                zone,
                encounter.GetTileIndex(),
                encounter.GetIndex().mValue));
    }
}


// For each encounter in every zone there is a unique enabled/disabled flag.
// This is reset every time a new chapter is loaded (I think);
unsigned GameData::CalculateUniqueEncounterStateFlagOffset(
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) const
{
    constexpr auto encounterStateOffset = 0x190;
    constexpr auto maxEncountersPerTile = 0xa;
    const auto zoneOffset = (zone.mValue - 1) * encounterStateOffset;
    const auto tileOffset = tileIndex * maxEncountersPerTile;
    const auto offset = zoneOffset + tileOffset + encounterIndex;
    return offset + encounterStateOffset;
}

bool GameData::CheckUniqueEncounterStateFlagOffset(
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) const
{
    return ReadEventBool(
        CalculateUniqueEncounterStateFlagOffset(
            zone,
            tileIndex,
            encounterIndex));
}

unsigned GameData::CalculateCombatEncounterStateFlag(
    unsigned combatIndex) const
{
    constexpr auto combatEncounterFlag = 0x1464;
    return combatIndex + combatEncounterFlag;
}

bool GameData::CheckCombatEncounterStateFlag(
    unsigned combatIndex) const
{
    constexpr auto alwaysTriggeredIndex = 0x3e8;
    if (combatIndex >= alwaysTriggeredIndex)
        return true;
    else
        return ReadEventBool(
            CalculateCombatEncounterStateFlag(combatIndex));
}



// 1450 is "recently encountered this encounter"
// should be cleared when we move to a new tile
// (or it will inhibit the events of the new tile)
unsigned GameData::CalculateRecentEncounterStateFlag(
    std::uint8_t encounterIndex) const
{
    // Refer readEncounterEventState1450 in IDA
    // These get cleared when we load a new tile
    constexpr auto offset = 0x1450;
    return offset + encounterIndex;
}

bool GameData::ReadConversationItemClicked(unsigned eventPtr) const
{
    return ReadEventBool(sConversationChoiceMarkedFlag + eventPtr);
}

void GameData::SetConversationItemClicked(unsigned eventPtr)
{
    return SetEventFlagTrue(sConversationChoiceMarkedFlag + eventPtr);
}

bool GameData::CheckConversationOptionInhibited(unsigned eventPtr)
{
    return ReadEventBool(sConversationOptionInhibitedFlag + eventPtr);
}

void GameData::SetLockHasBeenSeen(unsigned lockIndex)
{
    SetEventFlagTrue(sLockHasBeenSeenFlag + lockIndex);
}

bool GameData::CheckLockHasBeenSeen(unsigned lockIndex)
{
    return ReadEventBool(sLockHasBeenSeenFlag + lockIndex);
}

void GameData::ClearTileRecentEncounters()
{
    for (unsigned i = 0; i < 10; i++)
    {
        SetEventFlagFalse(CalculateRecentEncounterStateFlag(i));
    }
}

/* ************* LOAD Game STATE ***************** */
Party GameData::LoadParty()
{
    auto characters = LoadCharacters();
    auto active = LoadActiveCharacters();
    auto gold = LoadGold();
    auto keys = LoadCharacterInventory(sPartyKeyInventoryOffset);
    return Party{
        gold,
        std::move(keys),
        characters,
        active};
}

    
std::vector<Character> GameData::LoadCharacters()
{
    unsigned characters = sCharacterCount;

    std::vector<Character> chars;
    auto spellInfo = SpellInfo{};

    for (unsigned character = 0; character < characters; character++)
    {
        mBuffer.Seek(GetCharacterNameOffset(character));
        auto name = mBuffer.GetString(sCharacterNameLength);

        mBuffer.Seek(GetCharacterSkillOffset(character));
        mLogger.Debug() << "Name: " << name << "@" 
            << std::hex << mBuffer.Tell() << std::dec << "\n";

        auto characterNameOffset = mBuffer.GetArray<2>();
        auto spells = Spells{mBuffer.GetArray<6>()};
        for (const auto& spell : spellInfo.GetSpells())
        {
            if (spell.HasSpell(spells))
            {
                mLogger.Debug() << "  " << spell.mName << "\n";
            }
        }

        auto skills = Skills{};

        for (unsigned i = 0; i < Skills::sSkills; i++)
        {
            const auto max        = mBuffer.GetUint8();
            const auto trueSkill  = mBuffer.GetUint8();
            const auto current    = mBuffer.GetUint8();
            const auto experience = mBuffer.GetUint8();
            const auto modifier   = mBuffer.GetSint8();

            const auto pos = mBuffer.Tell();

            const auto selected = ReadSkillSelected(character, i);
            const auto unseenIprovement = ReadSkillUnseenImprovement(character, i);

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
    return mBuffer.GetUint16LE();
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

    return active;
}

Location GameData::LoadLocation()
{
    mBuffer.Seek(sLocationOffset);

    unsigned zone = mBuffer.GetUint8();
    ASSERT(zone < 12);
    mLogger.Info() << "LOADED: Zone:" << zone << std::endl;

    unsigned xtile = mBuffer.GetUint8();
    unsigned ytile = mBuffer.GetUint8();
    unsigned xpos = mBuffer.GetUint32LE();
    unsigned ypos = mBuffer.GetUint32LE();

    //mBuffer.DumpAndSkip(5);
    mBuffer.Skip(5);
    std::uint16_t heading = mBuffer.GetUint8();
    mBuffer.Skip(1);

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
    mLogger.Debug() << " Items: " << +itemCount << " cap: " << capacity << "\n";
    return LoadInventory(mBuffer, itemCount, capacity);
}

LockStats GameData::LoadLock()
{
    const auto lockFlag = mBuffer.GetUint8();
    const auto picklock = mBuffer.GetUint8();
    const auto fairyChestIndex = mBuffer.GetUint8();
    const auto damage = mBuffer.GetUint8();
    return LockStats{lockFlag, picklock, fairyChestIndex, damage};
}

ShopStats GameData::LoadShop()
{
    const auto templeNumber = mBuffer.GetUint8();
    const auto sellFactor = mBuffer.GetUint8();
    const auto maxDiscount = mBuffer.GetUint8();
    const auto buyFactor = mBuffer.GetUint8();
    const auto haggle1 = mBuffer.GetUint8();
    const auto haggle2 = mBuffer.GetUint8();
    const auto bardingSkill = mBuffer.GetUint8();
    const auto bardingReward = mBuffer.GetUint8();
    const auto bardingMaxReward = mBuffer.GetUint8();
    const auto unknown = mBuffer.GetArray<3>();
    const auto repairTypes = mBuffer.GetUint8();
    const auto repairFactor = mBuffer.GetUint8();
    const auto categories = mBuffer.GetUint16LE();

    return ShopStats{
        templeNumber,
        sellFactor,
        maxDiscount,
        buyFactor,
        haggle1,
        haggle2,
        bardingSkill,
        bardingReward,
        bardingMaxReward,
        unknown,
        repairTypes,
        repairFactor,
        categories
    };
}

std::vector<GenericContainer> GameData::LoadShops()
{
    mBuffer.Seek(sShopsOffset);
    auto shops = std::vector<GenericContainer>{};

    for (unsigned i = 0; i < sShopsCount; i++)
    {
        const unsigned address = mBuffer.Tell();
        mLogger.Debug() << " Container: " << i
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerGDSLocationTag>(mBuffer);
        shops.emplace_back(std::move(container));
        mLogger.Debug() << shops.back() << "\n";
    }

    return shops;
}

std::vector<GenericContainer> GameData::LoadContainers(unsigned zone)
{
    const auto& mLogger = Logging::LogState::GetLogger("GameData");
    mLogger.Debug() << "Loading containers for Z: " << zone << "\n";
    std::vector<GenericContainer> containers{};

    ASSERT(zone < sZoneContainerOffsets.size());
    const auto [offset, count] = sZoneContainerOffsets[zone];
    mBuffer.Seek(offset);

    for (unsigned j = 0; j < count; j++)
    {
        const unsigned address = mBuffer.Tell();
        mLogger.Debug() << " Container: " << j
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerWorldLocationTag>(mBuffer);
        containers.emplace_back(std::move(container));
        mLogger.Debug() << containers.back() << "\n";
    }

    return containers;
}

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
        // These are spells
        //mBuffer.DumpAndSkip(6);
        mBuffer.Skip(6);

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
        mBuffer.Skip(7);
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
    mLogger.Info() << "Loading Combat Inventories" << std::endl;
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
        mLogger.Info() << "CobmatInventory #" << i << " @" << std::hex << loc << std::dec << " " << containers.back() << "\n";
    }

    return containers;
}

}
