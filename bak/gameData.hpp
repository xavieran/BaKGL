#pragma once

#include <glm/glm.hpp>

#include "bak/constants.hpp"
#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "bak/container.hpp"
#include "bak/dialogAction.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/money.hpp"
#include "bak/party.hpp"
#include "bak/resourceNames.hpp"
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

#include <vector>
#include <memory>

namespace BAK {

struct Location
{
    unsigned mZone;
    glm::vec<2, unsigned> mTile;
    GamePositionAndHeading mLocation;
};

class GameData
{   
public:
/*
 *
 * Locklear, Gorath, Owyn, Pug, James, Patrus
 *
 * Characters Start @0xdb
 * 0x9fd -> Indicates unseen character stat improvement (bit flags)
 * * 0x011a0 - Character Inventory Offsets Start???
 * 0x01383 - Combat Entity List Start
 * 0x039cb - Combat Entity List Start
 *
 * 0x04fb0 - Combat locations
 * Note: This is only populated if loaded by a tile
 * 0x52f0 - combat 1 location start
 * c8 a4 00 00 5a 2b 00 00 00 80 00 03 00
 * X loc       yloc       rotation State (dead/alive/invislbe?)
 * 0x0913e - Events End??
 * @10 bytes each => 1678 combat locs?
 *
 * 0x0914b - Combat Stats Start 
 * 0x3070a - Combat Stats End
 *
 * 0x31340 - Combats Start
 * 
 * 0x3a7f0 - Character Inventory Start
 * 0x3aaf0 - Character Invenorty Start ???
 *
 * 0x3b621 - Start of Containers
 * 0x44c37 - End of Containers ???
 * 0x44cc9 - Start of Shops
 * 0x46043 End of shops
 * 0x46053 - Combat Inventory Start
 * 0x51720 - Combat Inventory End (1773)
 */

 /* 
  * Combat starts 31340 - 12 2 06 - 12 sprite 2,6 grid location
  * Combat Zone 1 1
  * Inventory #3 Combat #1 Person #0 0x460b7
  * Combat Stats #3 0x9268 (bbe0)? 
  */

    // Offset refers to the raw offset in the save file
    // "Flag" refers to the flag used in the code that is 
    // processed by "CalculateEventOffset" or "CalculateComplexEventOffset"
    // to generate a raw offset
    
    static constexpr auto sCharacterCount = 6;
    static constexpr auto sChapterOffset = 0x5a; // -> 5c
    static constexpr auto sGoldOffset = 0x66; // -> 6a
    static constexpr auto sTimeOffset = 0x6a; // -> 0x72
    static constexpr auto sLocationOffset = 0x76; // -> 0x88
    static constexpr auto sCharacterNameOffset    = 0x9f; // -> 0xdb
    static constexpr auto sCharacterSkillOffset   = 0xdb; // -> 0x315
    static constexpr auto sActiveCharactersOffset = 0x315; // -> 0x319
    static constexpr auto sCharacterStatusOffset  = 0x330; // -> 0x

    // Single bit indicators for event state tracking 
    // In the code this offset is 0x440a in the game -> diff of 0x3d28
    static constexpr auto sGameEventRecordOffset = 0x6e2; // -> 0xadc
    static constexpr auto sGameComplexEventRecordOffset = 0xb09; // -> 0xadc

    static constexpr auto sConversationChoiceMarkedFlag = 0x1d4c;
    static constexpr auto sConversationOptionInhibitedFlag = 0x1a2c;
    // Based on disassembly this may be the state of doors (open/closed)
    static constexpr auto sDoorFlag = 0x1b58;

    static constexpr auto sSkillSelectedEventFlag = 0x1856;
    static constexpr auto sSkillImprovementEventFlag= 0x18ce;

    static constexpr auto sCombatEntityListCount  = 700;
    static constexpr auto sCombatEntityListOffset = 0x1383;

    static constexpr auto sCharacterInventoryOffset = 0x3a804; // -> 3aa4b
    static constexpr auto sPartyKeyInventoryOffset = 0x3aaa4;

    static constexpr auto sZone1ContainerOffset = 0x3b631;
    static constexpr auto sZone2ContainerOffset = 0x3be55;
    static constexpr auto sZone3ContainerOffset = 0x3c55f;
    static constexpr auto sZone4ContainerOffset = 0x3d0b4;
    static constexpr auto sZone5ContainerOffset = 0x3dc07;
    static constexpr auto sZone6ContainerOffset = 0x3e708;
    static constexpr auto sZone7ContainerOffset = 0x3f8b2;
    static constexpr auto sZone8ContainerOffset = 0x40c97;
    static constexpr auto sZone9ContainerOffset = 0x416b7;
    static constexpr auto sZoneAContainerOffset = 0x42868;
    static constexpr auto sZoneBContainerOffset = 0x43012;
    static constexpr auto sZoneCContainerOffset = 0x4378f;
    
    static constexpr auto sShopsCount  = 98;
    static constexpr auto sShopsOffset = 0x443c9;

    static constexpr auto sCombatInventoryCount  = 1734;
    static constexpr auto sCombatInventoryOffset = 0x46053;

    GameData(const std::string& save)
    :
        mBuffer{FileBufferFactory::CreateFileBuffer(save)},
        mLogger{Logging::LogState::GetLogger("GameData")},
        mName{LoadSaveName()},
        mObjects{},
        mChapter{LoadChapter()},
        mLocation{LoadLocation()},
        mTime{LoadWorldTime()},
        mParty{LoadParty()}
    {

        mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
        //mLogger.Info() << mParty << "\n";
        mLogger.Info() << mTime << "\n";
        LoadContainers(0x1);
        LoadContainers(0x2);
        LoadContainers(0x3);
        LoadContainers(0x4);
        LoadContainers(0x5);
        LoadContainers(0x6);
        LoadContainers(0x7);
        LoadContainers(0x8);
        LoadContainers(0x9);
        LoadContainers(0xa);
        LoadContainers(0xb);
        LoadContainers(0xc);
        mLogger.Debug() << "Loaded Z12 Cont: " << std::hex 
            << mBuffer.Tell() << std::dec << "\n";
        LoadShops();
        //LoadCombatEntityLists();
        //LoadCombatInventories(
        //    sCombatInventoryOffset,
        //    sCombatInventoryCount);
        //LoadCombatStats(0x914b, 1698);
    }


    std::pair<unsigned, unsigned> CalculateComplexEventOffset(unsigned eventPtr) const
    {
        const auto source = (eventPtr + 0x2540) & 0xffff;
        const auto byteOffset = source / 10;
        const auto bitOffset = source % 10 != 0
            ? (source % 10) - 1
            : 0;
            
        mLogger.Spam() << __FUNCTION__ << std::hex << " " << eventPtr << " ("
            << byteOffset + sGameComplexEventRecordOffset << ", " 
            << bitOffset << ")\n" << std::dec;
        return std::make_pair(
            byteOffset + sGameComplexEventRecordOffset,
            bitOffset);
    }

    std::pair<unsigned, unsigned> CalculateEventOffset(unsigned eventPtr) const
    {
        const unsigned startOffset = sGameEventRecordOffset;
        const unsigned bitOffset = eventPtr & 0xf;
        const unsigned byteOffset = (0xfffe & (eventPtr >> 3)) + startOffset;
        mLogger.Spam() << __FUNCTION__ << std::hex << " " << eventPtr << " ("
            << byteOffset << ", " << bitOffset << ")\n" << std::dec;
        return std::make_pair(byteOffset, bitOffset);
    }

    void SetBitValueAt(unsigned byteOffset, unsigned bitOffset, unsigned value)
    {
        mBuffer.Seek(byteOffset);
        const auto originalData = mBuffer.GetUint16LE();
        auto data = originalData;
        if (value)
            data = data | (1 << bitOffset);
        else
            data = data & (~(1 << bitOffset));
  
        mBuffer.Seek(byteOffset);
        mBuffer.PutUint16LE(data);

        mLogger.Debug() << __FUNCTION__ << std::hex << 
            " " << byteOffset << " " << bitOffset 
            << " original[" << +originalData << "] new[" << +data  <<"]\n";
    }

    void SetEventFlag(unsigned eventPtr, unsigned value)
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

    void SetEventFlagTrue (unsigned eventPtr) { SetEventFlag(eventPtr, 1); }
    void SetEventFlagFalse(unsigned eventPtr) { SetEventFlag(eventPtr, 0); }

    void SetEventDialogAction(const SetFlag& setFlag)
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
                << " data[" << +data << "] new[" << +newData <<"]\n";
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

    unsigned ReadBitValueAt(unsigned byteOffset, unsigned bitOffset) const
    {
        mBuffer.Seek(byteOffset);
        const unsigned eventData = mBuffer.GetUint16LE();
        const unsigned bitValue = eventData >> bitOffset;
        mLogger.Spam() << __FUNCTION__ << std::hex << 
            " " << byteOffset << " " << bitOffset 
            << " [" << +bitValue << "]\n";
        return bitValue;
    }

    unsigned ReadEvent(unsigned eventPtr) const
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

    bool ReadEventBool(unsigned eventPtr) const
    {
        return (ReadEvent(eventPtr) & 0x1) == 1;
    }

    bool ReadSkillSelected(unsigned character, unsigned skill) const
    {
        constexpr auto maxSkills = 0x11;
        return ReadEventBool(
            sSkillSelectedEventFlag
            + (character * maxSkills)
            + skill);
    }

    bool ReadSkillUnseenImprovement(unsigned character, unsigned skill) const
    {
        constexpr auto maxSkills = 0x11;
        return ReadEventBool(
            sSkillImprovementEventFlag
            + (character * maxSkills)
            + skill);
    }

    void ClearUnseenImprovements(unsigned character)
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

    // Called by checkBlockTriggered, checkTownTriggered, checkBackgroundTriggered, checkZoneTriggered,
    // doEnableEncounter, doDialogEncounter, doDisableEncounter, doSoundEncounter
    bool CheckActive(
        const Encounter::Encounter& encounter,
        ZoneNumber zone)
    {
        const auto encounterIndex = encounter.GetIndex().mValue;
        const bool alreadyEncountered = ReadEventBool(
            CalculateUniqueEncounterStateFlagOffset(
                zone,
                encounter.GetTileIndex(),
                encounterIndex));
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

    void SetPostDialogEventFlags(const Encounter::Encounter& encounter)
    {
        constexpr auto zone = ZoneNumber{1};
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
            // FIXME use actual values
            if (encounter.mUnknown2 != 0) // Inhibit for this chapter
            {
                SetEventFlagTrue(CalculateUniqueEncounterStateFlagOffset(
                    zone, tileIndex, encounterIndex));
            }

            // Inhibit for this tile
            SetEventFlagTrue(CalculateRecentEncounterStateFlag(encounterIndex));
        }

    }
    
    // Background and Town
    void SetPostGDSEventFlags(const Encounter::Encounter& encounter)
    {
        if (encounter.mSaveAddress3 != 0)
            SetEventFlagTrue(encounter.mSaveAddress3);
    }

    // Used by Block, Disable, Enable, Sound, Zone
    void SetPostEnableOrDisableEventFlags(
        const Encounter::Encounter& encounter,
        ZoneNumber zone)
    {
        if (encounter.mSaveAddress3 != 0)
        {
            SetEventFlagTrue(encounter.mSaveAddress3);
        }
        // FIXME use actual values
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
    unsigned CalculateUniqueEncounterStateFlagOffset(
        ZoneNumber zone, 
        std::uint8_t tileIndex,
        std::uint8_t encounterIndex)
    {
        constexpr auto encounterStateOffset = 0x190;
        constexpr auto maxEncountersPerTile = 0xa;
        const auto zoneOffset = (zone.mValue - 1) * encounterStateOffset;
        const auto tileOffset = tileIndex * maxEncountersPerTile;
        const auto offset = zoneOffset + tileOffset + encounterIndex;
        return offset + encounterStateOffset;
    }

    // 1450 is "recently encountered this encounter"
    // should be cleared when we move to a new tile
    // (or it will inhibit the events of the new tile)
    unsigned CalculateRecentEncounterStateFlag(
        std::uint8_t encounterIndex)
    {
        // Refer readEncounterEventState1450 in IDA
        // These get cleared when we load a new tile
        constexpr auto offset = 0x1450;
        return offset + encounterIndex;
    }

    bool ReadConversationItemClicked(unsigned eventPtr) const
    {
        return ReadEventBool(sConversationChoiceMarkedFlag + eventPtr);
    }

    void SetConversationItemClicked(unsigned eventPtr)
    {
        return SetEventFlagTrue(sConversationChoiceMarkedFlag + eventPtr);
    }

    bool CheckConversationOptionInhibited(unsigned eventPtr)
    {
        return ReadEventBool(sConversationOptionInhibitedFlag + eventPtr);
    }

    void ClearTileRecentEncounters()
    {
        for (unsigned i = 0; i < 10; i++)
        {
            SetEventFlagFalse(CalculateRecentEncounterStateFlag(i));
        }
    }

    /* ************* LOAD Game STATE ***************** */
    Party LoadParty()
    {
        auto characters = LoadCharacters();
        auto active = LoadActiveCharacters();
        auto gold = LoadGold();
        auto keys = LoadInventory(sPartyKeyInventoryOffset);
        return Party{
            gold,
            std::move(keys),
            characters,
            active};
    }

        
    std::vector<Character> LoadCharacters()
    {
        unsigned characters = sCharacterCount;
        const auto nameOffset = sCharacterNameOffset;
        const auto nameLength = 10;
        const auto skillLength = 5 * 16 + 8 + 7;
        const auto skillOffset = sCharacterSkillOffset;
        const auto inventoryOffset = sCharacterInventoryOffset;
        const auto inventoryLength = 0x70;

        std::vector<Character> chars;

        for (unsigned character = 0; character < characters; character++)
        {
            mBuffer.Seek(nameOffset + nameLength * character);
            auto name = mBuffer.GetString(nameLength);

            mBuffer.Seek(skillOffset + skillLength * character);
            mLogger.Debug() << "Name: " << name << "@" 
                << std::hex << mBuffer.Tell() << std::dec << "\n";

            auto unknown = mBuffer.GetArray<2>();
            auto spells = mBuffer.GetArray<6>();

            auto skills = Skills{};

            for (unsigned i = 0; i < Skills::sSkills; i++)
            {
                const auto max        = mBuffer.GetUint8();
                const auto current    = mBuffer.GetUint8();
                const auto limit      = mBuffer.GetUint8();
                const auto experience = mBuffer.GetUint8();
                const auto modifier   = mBuffer.GetSint8();

                const auto pos = mBuffer.Tell();

                const auto selected = ReadSkillSelected(character, i);
                const auto unseenIprovement = ReadSkillUnseenImprovement(character, i);

                skills.mSkills[i] = Skill{
                    max,
                    current,
                    limit,
                    experience,
                    modifier,
                    selected,
                    unseenIprovement
                };

                mBuffer.Seek(pos);
            }

            auto unknown2 = mBuffer.GetArray<7>();
            mLogger.Info() << " Finished loading : " << name << std::hex << mBuffer.Tell() << std::dec << "\n";
            // Load inventory
            auto inventory = LoadInventory(
                inventoryOffset + inventoryLength * character);

            auto conditions = LoadConditions(character);

            chars.emplace_back(
                character,
                name,
                skills,
                spells,
                unknown,
                unknown2,
                conditions,
                std::move(inventory));
        }
        
        return chars;
    }

    Conditions LoadConditions(unsigned character)
    {
        assert(character < sCharacterCount);
        mBuffer.Seek(sCharacterStatusOffset 
            + character * Conditions::sNumConditions);

        auto conditions = Conditions{};
        for (unsigned i = 0; i < Conditions::sNumConditions; i++)
            conditions.mConditions[i] = mBuffer.GetUint8();
        return conditions;
    }

    unsigned LoadChapter()
    {
        mBuffer.Seek(sChapterOffset);
        return mBuffer.GetUint16LE();
    }

    Royals LoadGold()
    {
        mBuffer.Seek(sGoldOffset);
        return Royals{mBuffer.GetUint32LE()};
    }

    std::vector<CharIndex> LoadActiveCharacters()
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
    Location LoadLocation()
    {
        mBuffer.Seek(sLocationOffset);

        unsigned zone = mBuffer.GetUint8();
        assert(zone < 12);
        mLogger.Info() << "Zone:" << zone << std::endl;

        unsigned xtile = mBuffer.GetUint8();
        unsigned ytile = mBuffer.GetUint8();
        unsigned xpos = mBuffer.GetUint32LE();
        unsigned ypos = mBuffer.GetUint32LE();

        mBuffer.DumpAndSkip(5);
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

    WorldClock LoadWorldTime()
    {
        mBuffer.Seek(sTimeOffset);
        return WorldClock{
            Time{mBuffer.GetUint32LE()},
            Time{mBuffer.GetUint32LE()}};
    }


    Inventory LoadInventory(unsigned offset)
    {
        mBuffer.Seek(offset);

        const auto itemCount = mBuffer.GetUint8();
        const auto capacity = mBuffer.GetUint16LE();
        mLogger.Info() << " Items: " << +itemCount << " cap: " << capacity << "\n";
        return Inventory{capacity, LoadItems(itemCount, capacity)};
    }

    std::vector<InventoryItem> LoadItems(unsigned itemCount, unsigned capacity)
    {
        std::vector<InventoryItem> items{};
        unsigned i;
        for (i = 0; i < itemCount; i++)
        {
            const auto item = ItemIndex{mBuffer.GetUint8()};
            const auto& object = mObjects.GetObject(item);
            const auto condition = mBuffer.GetUint8();
            const auto status = mBuffer.GetUint8();
            const auto modifiers = mBuffer.GetUint8();

            items.emplace_back(
                InventoryItemFactory::MakeItem(
                    item,
                    condition,
                    status,
                    modifiers));
        }

        for (; i < capacity; i++)
            mBuffer.Skip(4);

        return items;
    }

    std::vector<GDSContainer> LoadShops()
    {
        mBuffer.Seek(sShopsOffset);
        auto shops = std::vector<GDSContainer>{};

        for (unsigned i = 0; i < sShopsCount; i++)
        {
            const auto header = ContainerHeader{
                ContainerGDSLocationTag{},
                mBuffer};
            //auto tileX = mBuffer.GetUint8();
            //auto tileY = mBuffer.GetUint8();
            //ASSERT(mBuffer.GetUint16LE() == 0);

            //const unsigned gdsPart1 = mBuffer.GetUint32LE();
            //const unsigned gdsPart2 = mBuffer.GetUint32LE();
            //const unsigned shopType = mBuffer.GetUint8();
            //const unsigned itemCount = mBuffer.GetUint8();
            //const unsigned capacity = mBuffer.GetUint8();
            //const auto containerType = static_cast<ContainerType>(mBuffer.GetUint8());
            mLogger.Debug() << "Shop #: " << i << " " << header << "\n";
            auto inventory = Inventory{
                header.mCapacity,
                LoadItems(header.mItems, header.mCapacity)};
            // Shop data...
            if (static_cast<ContainerType>(header.mContainerType) == ContainerType::Shop)
                mBuffer.DumpAndSkip(16);

            mLogger.Debug() << std::hex << mBuffer.Tell() << std::dec << "\n";
            shops.emplace_back(
                header.GetHotspotRef(),
                header.mLocationType,
                header.mItems,
                header.mCapacity,
                static_cast<ContainerType>(header.mContainerType),
                std::move(inventory));
        }

        return shops;
    }

    std::vector<Container> LoadContainers(unsigned zone)
    {
        const auto& logger = Logging::LogState::GetLogger("GameData");
        logger.Info() << "Loading containers for Z: " << zone << "\n";
        std::vector<Container> containers{};

        unsigned count = 0;
        // TombStone
        switch (zone)
        {
        case 1:
            mBuffer.Seek(0x3b621); // 36 items 1
            count = 36;
            break;
        case 2:
            mBuffer.Seek(0x3be55); // 25 2
            count = 25;
            break;
        case 3:
            mBuffer.Seek(0x3c55f); // 54 3
            count = 54;
            break;
        case 4:
            mBuffer.Seek(0x3d0b4); // 65 4
            count = 65;
            break;
        case 5:
            mBuffer.Seek(0x3dc07); // 63 5
            count = 63;
            break;
        case 6:
            mBuffer.Seek(0x3e708); // 131 6
            count = 131;
            break;
        case 7:
            mBuffer.Seek(0x3f8b2); // 115 7
            count = 115;
            break;
        case 8:
            mBuffer.Seek(0x40c97); // 67 8
            count = 67;
            break;
        case 9:
            mBuffer.Seek(0x416b7); // 110 9
            count = 110;
            break;
        case 10:
            mBuffer.Seek(0x42868); // 25 A
            count = 25;
            break;
        case 11:
            mBuffer.Seek(0x43012); // 30 B
            count = 30;
            break;
        case 12:
            mBuffer.Seek(0x4378f); // 60 C
            count = 60;
            break;
        default:
            throw std::runtime_error("Zone not supported");
        }

        for (unsigned j = 0; j < count; j++)
        {
            unsigned address = mBuffer.Tell();
            logger.Info() << " Container: " << j
                << " addr: " << std::hex << address << std::dec << std::endl;

            auto header = ContainerHeader(ContainerWorldLocationTag{}, mBuffer);
            ASSERT(header.mLocationType == 6 
                || header.mLocationType == 9
                || header.mCapacity > 0);
            const auto containerType = static_cast<ContainerType>(header.mContainerType);

            logger.Info() << header
                << " Tp: " << ToString(containerType) << std::endl;
            
            auto inventory = Inventory{
                header.mCapacity,
                LoadItems(header.mItems, header.mCapacity)};

            mBuffer.DumpAndSkip(1);
            auto picklockSkill  = mBuffer.GetUint8();
            auto dialog = Target{KeyTarget{mBuffer.GetUint32LE()}};

            logger.Info() << "Picklock: " << std::hex << +picklockSkill
                << " dialog: " << dialog << std::dec << "\n";

            if (   containerType != ContainerType::Gravestone
                && containerType != ContainerType::Inn
                && containerType != ContainerType::Combat)
                dialog = KeyTarget{0};

            if (containerType == ContainerType::Shop
                || containerType == ContainerType::Inn)
            {
                mBuffer.DumpAndSkip(16);
            }
            else if (containerType == ContainerType::Bag)
            {
                mBuffer.Skip(-6);
            }
            else if (containerType == ContainerType::Unknown)
            {
                mBuffer.Skip(-2);
            }
            else if (containerType == ContainerType::Building)
            {
                const auto postDialog = Target{KeyTarget{mBuffer.GetUint32LE()}};
                mLogger.Debug() << "PostLockDialog:  " << postDialog << "\n";
                dialog = postDialog;
            }
            else if (containerType == ContainerType::TimirianyaHut)
            {
                mBuffer.DumpAndSkip(3);
            }
            else if (containerType == ContainerType::Combat)
            {
                mBuffer.DumpAndSkip(9);
            }
            else if (containerType == ContainerType::Chest)
            {
                mBuffer.Skip(-2);
            }
            else if (containerType == ContainerType::FairyChest)
            {
                //if (header.mLocationType != 4 
                //    && header.mCapacity == 5)
                //{
                //    mLogger.Debug() << "Large Fairy Chest\n";
                //    mBuffer.DumpAndSkip(3 * 8 + 1);
                //}
                mBuffer.DumpAndSkip(2);
            }
            else if (containerType == ContainerType::EventChest)
            {
                mBuffer.DumpAndSkip(11);
            }

            containers.emplace_back(
                address,
                header.mLocationType,
                header.mItems,
                header.mCapacity,
                static_cast<ContainerType>(header.mContainerType),
                dialog,
                header.GetPosition(),
                std::move(inventory));
            logger.Info() << "Items: \n" << containers.back().GetInventory() << "\n";


            std::cout << std::endl;
        }

        return containers;
    }

    void LoadChapterOffsetP()
    {
        // I have no idea what these mean
        constexpr unsigned chapterOffsetsStart = 0x11a3;
        mBuffer.Seek(chapterOffsetsStart);

        mLogger.Info() << "Chapter Offsets Start @" 
            << std::hex << chapterOffsetsStart << std::dec << std::endl;

        for (unsigned i = 0; i < 10; i++)
        {
            std::stringstream ss{};
            ss << "Chapter #" << i << " : " << mBuffer.GetUint16LE();
            for (unsigned i = 0; i < 5; i++)
            {
                unsigned addr = mBuffer.GetUint32LE();
                ss << " a: " << std::hex << addr;
            }
            mLogger.Info() << ss.str() << std::endl;
        }

        mLogger.Info() << "Chapter Offsets End @" 
            << std::hex << mBuffer.Tell() << std::dec << std::endl;
    }

    void LoadCombatEntityLists()
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

    void LoadCombatStats(unsigned offset, unsigned num)
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
            mBuffer.Dump(6);
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
            mBuffer.Dump(7);
            mBuffer.Skip(7);
            mLogger.Info() << ss.str() << std::endl;
        }
        mLogger.Info() << "Combat Stats End @" 
            << std::hex << mBuffer.Tell() << std::dec << std::endl;
    }

    void LoadCombatInventories(unsigned offset, unsigned number)
    {
        mLogger.Info() << "Loading Combat Inventories" << std::endl;
        auto combatInventoryLocation = offset;
        auto numberCombatInventories = number;
        mBuffer.Seek(combatInventoryLocation);
        for (unsigned i = 0; i < numberCombatInventories; i++)
        {
            mBuffer.Dump(13);
            auto x = mBuffer.Tell();
            //assert(mBuffer.GetUint8() == 0x64);
            mBuffer.GetUint8(); // always 0x64 for combats
            mBuffer.GetUint8();// == 0x0a);
            mBuffer.Skip(2);
            auto combatantNo = mBuffer.GetUint16LE();
            mBuffer.Skip(2);
            auto combatNo = mBuffer.GetUint16LE();
            mBuffer.Skip(2);

            int unknown = mBuffer.GetUint8();
            //unsigned items = mBuffer.GetUint8();
            //unsigned capacity = mBuffer.GetUint16LE();

            mLogger.Info() << "CombatInventory #" << i << " "
                << std::hex << x << std::dec << " CBT: " << +combatNo 
                << " PER: " << +combatantNo << " Unk: " << unknown << std::endl;
            const auto inventory = LoadInventory(mBuffer.Tell());
            mLogger.Info() << inventory << "\n";
        }
    }

    std::string LoadSaveName()
    {
        mBuffer.Seek(0);
        return mBuffer.GetString(30);
    }

    mutable FileBuffer mBuffer;
    Logging::Logger mLogger;

    const std::string mName;
    ObjectIndex mObjects;
    Chapter mChapter;
    Location mLocation;
    WorldClock mTime;
    Party mParty;
};

}
