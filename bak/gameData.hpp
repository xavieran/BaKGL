#pragma once

#include <glm/glm.hpp>

#include "bak/constants.hpp"
#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "bak/container.hpp"
#include "bak/encounter.hpp"
#include "bak/party.hpp"
#include "bak/resourceNames.hpp"
#include "bak/skills.hpp"
#include "bak/worldClock.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

#include <vector>
#include <memory>

namespace BAK {

struct Location
{
    unsigned mZone;
    int mHeading;
    glm::vec<2, int> mPosition;
    glm::vec<2, int> mTile;
};

class GameData
{   
public:
    using Chapter = unsigned;
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
 * 0x46053 - Combat Inventory Start
 * 0x51720 - Combat Inventory End (1773)
 */

 /* 
  * Combat starts 31340 - 12 2 06 - 12 sprite 2,6 grid location
  * Combat Zone 1 1
  * Inventory #3 Combat #1 Person #0 0x460b7
  * Combat Stats #3 0x9268 (bbe0)? 
  * DEF_COMB.DAT
  * --                Combat Number (This must index to somewhere else?)
  * b1 01 00 01 07 00 01 00 00 00 14 
  * 01b1 = 433 (Number of combats in file?)
  * 14 = Dialog index for this combat
  *
  *  Number of Combats
  * B1 01 00 00 
  *
  * 01 - visible or not? Chapter??
  * 07 ??
  * 01 combat index
  * 01 07 00 01 
  *
  * 14 - Dialog index
  * 00 00 00 14  00 00 00 00
  * 00 00 00 00  00 00 00 DF  AC 00 00 BB  64 00 00 00
  * 80 43 77 00  00 9B 3F 00  00 00 C0 5C  9C 00 00 87
  * 03 00 00 00  00 86 C1 00  00 64 0C 00  
  *
  * 01 - number of combatants to display?
  * 00 00 00 01
  * 12 - combatant sprite index? 01 - sprite mobile or not
  * 12 00 01 00 
  * -- The rest seems to be coordinates for sprite movement?
  * C8 A4 00 00  5A 2B 00 00  00 40 88 9E
  * 00 00 08 AB  00 00 90 60  00 00 00 F0  20 40 5A 2B
  * 00 00 5A 2B  00 00 40 80  40 40 40 40  20 00 80   
  *
  */

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
    static constexpr auto sGameEventRecordOffset = 0x6e2; // -> 0xadc

    // dac0 -> dc3c

    // from 9fc -> 0xc0 ff ff will completely fill locklears "Unseen stats" 
    // but also overflow into Gorath health. Not sure why these flags don't
    // seem to be byte aligned.
    static constexpr auto sSkillImprovementOffset = 0x9fc;
    // 9ed:0x4 == Locklear Defense Selected (START)
    // 9ed:0x40 == Locklear Assessment selected
    // 9ee:0x1  == Locklear Weaponcraft selected
    // 9ee:0x20 == Locklear Stealth Selected (END)
    // 9ee:0x40 == Nothing
    // 9ef:0x08 == Gorath Defense (START)
    // 9ef:0x80 == Gorath Assessment
    // 9f0:0x01 == Gorath Armorcraft
    // 9f0:0x40 == Gorath Stealth (END)
    // 9f1:0x10 == Owyn Defense (START)
    // 9f1:0x80 == Owyn Stealth (END)
    static constexpr auto sSkillSelectedOffset    = 0x9ed;

    static constexpr auto sCombatEntityListCount  = 700;
    static constexpr auto sCombatEntityListOffset = 0x1383;

    static constexpr auto sCharacterInventoryOffset = 0x3a804; // -> 3aa4b

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

    static constexpr auto sCombatInventoryCount  = 1734;
    static constexpr auto sCombatInventoryOffset = 0x46053;

    GameData(const std::string& save)
    :
        mBuffer{FileBufferFactory::CreateFileBuffer(save)},
        mLogger{Logging::LogState::GetLogger("GameData")},
        mObjects{},
        mChapter{LoadChapter()},
        mLocation{LoadLocation()},
        mTime{LoadWorldTime()},
        mParty{LoadParty()}
    {

        mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
        mLogger.Info() << mParty << "\n";
        mLogger.Info() << mTime << "\n";
        //ReadEventWord(0xdac0);
        //ReadEventWord(0xc0da); // 0xb08
        //LoadChapterOffsetP();
        //LoadContainer();
        //LoadCombatEntityLists();
        //LoadCombatInventories(
        //    sCombatInventoryOffset,
        //    sCombatInventoryCount);
        //LoadCombatStats(0x914b, 1698);
    }

    /*
     * Sumani dialog
     * When click sumani we get a flag set at a8d
     * This flag tells the UI whether a choice has been clicked or not
     */

    Party LoadParty()
    {
        constexpr auto keyOffset = 0x3aaa4;
        auto characters = LoadCharacters();
        auto active = LoadActiveCharacters();
        auto gold = LoadGold();
        auto keys = LoadInventory(keyOffset);
        return Party{
            gold,
            keys,
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

        for (unsigned i = 0; i < characters; i++)
        {
            mBuffer.Seek(nameOffset + nameLength * i);
            auto name = mBuffer.GetString(nameLength);

            mBuffer.Seek(skillOffset + skillLength * i);
            mLogger.Debug() << "Name: " << name << "@" 
                << std::hex << mBuffer.Tell() << std::dec << "\n";

            auto unknown = mBuffer.GetArray<2>();
            auto spells = mBuffer.GetArray<6>();

            auto skills = Skills{};

            for (unsigned i = 0; i < Skills::sSkills; i++)
            {
                skills.mSkills[i] = Skill{
                    mBuffer.GetUint8(),
                    mBuffer.GetUint8(),
                    mBuffer.GetUint8(),
                    mBuffer.GetUint8(),
                    mBuffer.GetSint8(), // modifier can be -ve
                    false
                };
            }

            auto unknown2 = mBuffer.GetArray<7>();
            mLogger.Info() << " Finished loading : " << name << std::hex << mBuffer.Tell() << std::dec << "\n";
            // Load inventory
            auto inventory = LoadInventory(
                inventoryOffset + inventoryLength * i);

            auto conditions = LoadConditions(i);

            chars.emplace_back(
                name,
                skills,
                spells,
                unknown,
                unknown2,
                conditions,
                inventory);
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

    int LoadGold()
    {
        mBuffer.Seek(sGoldOffset);
        return mBuffer.GetSint32LE();
    }

    std::vector<std::uint8_t> LoadActiveCharacters()
    {
        mBuffer.Seek(sActiveCharactersOffset);
        const auto activeCharacters = mBuffer.GetUint8();

        auto active = std::vector<std::uint8_t>{};
        for (unsigned i = 0; i < activeCharacters; i++)
        {
            const auto c = mBuffer.GetUint8();
            active.emplace_back(c);
        }

        return active;
    }

    unsigned ReadComplexEvent(unsigned eventPtr) const
    {
        // the left over of divisor maybe is nibble?
        constexpr auto offset = -0xad7;
        constexpr auto divider = 10;
        const auto eventOffset = (eventPtr / divider) + offset;
        mBuffer.Seek(eventOffset);
        return mBuffer.GetUint8();
    }

    unsigned ReadEvent(unsigned eventPtr) const
    {
        unsigned startOffset = sGameEventRecordOffset;
        unsigned bitOffset = eventPtr & 0xf;
        unsigned eventLocation = (0xfffe & (eventPtr >> 3)) + startOffset;
        mBuffer.Seek(eventLocation);
        unsigned eventData = mBuffer.GetUint16LE();
        unsigned bitValue = (eventData >> bitOffset) & 0x1;
        mLogger.Spam() << "Ptr: " << std::hex << eventPtr << " loc: "
            << eventLocation << " val: " << eventData << " bitVal: "
            << bitValue << std::dec << std::endl;

        return bitValue;
    }

    unsigned ReadEventWord(unsigned eventPtr) const
    {
        unsigned startOffset = sGameEventRecordOffset;
        unsigned bitOffset = eventPtr & 0xf;
        unsigned eventLocation = (0xfffe & (eventPtr >> 3)) + startOffset;
        mBuffer.Seek(eventLocation);
        unsigned eventData = mBuffer.GetUint32LE();
        mLogger.Spam() << "Ptr: " << std::hex << eventPtr << " loc: "
            << eventLocation << " val: " << eventData << std::dec << "\n";

        return eventData;
    }

    Location LoadLocation()
    {
        mBuffer.Seek(sLocationOffset);

        unsigned zone = mBuffer.GetUint8();
        assert(zone < 12);
        mLogger.Info() << "Zone:" << zone << std::endl;

        int xtile = mBuffer.GetUint8();
        int ytile = mBuffer.GetUint8();
        int xpos = mBuffer.GetUint32LE();
        int ypos = mBuffer.GetUint32LE();

        mBuffer.DumpAndSkip(5);
        int heading = mBuffer.GetUint16LE();

        mLogger.Info() << "Tile: " << xtile << "," << ytile << std::endl;
        mLogger.Info() << "Pos: " << xpos << "," << ypos << std::endl;
        mLogger.Info() << "Heading: " << heading << std::endl;
        
        return Location{
            zone,
            heading,
            {xpos, ypos},
            {xtile, ytile}
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

        std::vector<InventoryItem> items{};

        const auto itemCount = mBuffer.GetUint8();
        const auto capacity = mBuffer.GetUint16LE();
        mLogger.Info() << " Items: " << +itemCount << " cap: " << capacity << "\n";
        for (unsigned i = 0; i < itemCount; i++)
        {
            const auto item = mBuffer.GetUint8();
            const auto& object = mObjects.GetObject(item);
            const auto condition = mBuffer.GetUint8();
            const auto status = mBuffer.GetUint8();
            const auto modifiers = mBuffer.GetUint8();
            mLogger.Info() << " Loaded: " << object.mName << "\n";

            items.emplace_back(
                object,
                item,
                condition,
                status,
                modifiers);
        }

        return Inventory{items};
    }


    std::vector<Container> LoadContainer()
    {
        const auto& logger = Logging::LogState::GetLogger("GameData");
        logger.Info() << "Loading containers" << std::endl;
        auto* objects = ObjectResource::GetInstance();
        std::vector<Container> containers{};

        // TombStone
        mBuffer.Seek(0x3b621); // 36 items 1
        const auto count = 36;

        /*
        mBuffer.Seek(0x3be55); // 25 2
        mBuffer.Seek(0x3c55f); // 54 3
        mBuffer.Seek(0x3d0b4); // 65 4
        mBuffer.Seek(0x3dc07); // 63 5
        mBuffer.Seek(0x3e708); // 131 6
        mBuffer.Seek(0x3f8b2); // 115 7
        mBuffer.Seek(0x40c97); // 67 8
        mBuffer.Seek(0x416b7); // 110 9
        mBuffer.Seek(0x42868); // 25 A
        mBuffer.Seek(0x43012); // 30 B
        mBuffer.Seek(0x4378f); // 60 C
        */

        for (int j = 0; j < count; j++)
        {
            unsigned address = mBuffer.Tell();
            logger.Info() << " Container: " << j
                << " addr: " << std::hex << address << std::dec << std::endl;

            mBuffer.Dump(4);

            //auto aLoc = mBuffer.GetUint16LE();
            //auto bLoc = mBuffer.GetUint16LE();
            auto dlogp = mBuffer.GetUint32LE();
            //auto pair = glm::vec<2, std::uint16_t>{aLoc, bLoc};
            // bLoc == C3 dbody
            // bLoc == C4 hole dirt
            // bLoc == C5 Bag
            auto xLoc = mBuffer.GetUint32LE();
            auto yLoc = mBuffer.GetUint32LE();
            auto location = glm::vec<2, unsigned>{xLoc, yLoc};
            auto chestNumber   = mBuffer.GetUint8();
            auto chestItems    = mBuffer.GetUint8();
            auto chestCapacity = mBuffer.GetUint8();
            auto containerType = mBuffer.GetUint8();

            assert(chestNumber == 6 || chestNumber == 9 || chestCapacity > 0);

            const auto Container = [](auto x) -> std::string
            {
                //if ((x & 0x04) == 0x04) return "Shop";
                if ((x & 0x04) == 0x04) return "Shop";
                if (x == 17) return "Fairy Chest";
                else return "Dunno";
            };

            logger.Info() << "DLog??: " << std::hex << dlogp << std::dec << " " 
                << xLoc << "," << yLoc << " #" << +chestNumber << " items: " 
                << +chestItems << " capacity: " << +chestCapacity 
                << " Tp: " << +containerType << " " 
                << Container(containerType) << std::endl;
            
            std::vector<Item> items{};
            items.reserve(7);
            std::stringstream ss{""};
            int i = 0;
            for (; i < chestItems; i++)
            {
                auto item = mBuffer.GetUint8();
                auto object = objects->GetObjectInfo(item);
                auto condition = mBuffer.GetUint8();
                auto modifiers = mBuffer.GetUint8();
                auto yy = mBuffer.GetUint8();
                ss << std::hex << "0x" << +item << " " << object.name 
                    << std::dec << " " << " cond/qty: "  << +condition 
                    <<" mod: " <<  +modifiers << " y; " << + yy << std::endl;
                items.emplace_back(item, object.name, condition, modifiers);
            }

            
            for (; i < chestCapacity; i++)
            {
                mBuffer.Skip(4);
            }

            logger.Info() << "Items: \n" << ss.str() << std::endl;
            mBuffer.DumpAndSkip(1);
            auto picklockSkill  = mBuffer.GetUint8();
            auto containerIndex = mBuffer.GetUint16LE();
            logger.Info() << "Picklock: " << std::dec << +picklockSkill 
                << " ContainerI: " << containerIndex << std::endl;
            mBuffer.DumpAndSkip(2);

            containers.emplace_back(
                address,
                chestNumber,
                chestItems,
                chestCapacity,
                containerType,
                containerIndex,
                location,
                items);

            if (Container(containerType) == "Shop")
            {
                mBuffer.DumpAndSkip(16);
            }
            else if (containerType == 0)
            {
                mBuffer.Skip(-6);
            }
            else if (containerType == 1)
            {
                mBuffer.Skip(-2);
            }
            else if (containerType == 3)
            {
                mBuffer.DumpAndSkip(4);
            }
            else if (containerType == 8)
            {
                mBuffer.DumpAndSkip(3);
            }
            else if (containerType == 10)
            {
                mBuffer.DumpAndSkip(9);
            }
            else if (containerType == 16)
            {
                mBuffer.Skip(-2);
            }
            else if (containerType == 17)
            {
                if (chestNumber != 4 && chestCapacity == 5)
                {
                    mBuffer.DumpAndSkip(3 * 8 + 1);
                }
                mBuffer.DumpAndSkip(2);
            }
            else if (containerType == 25)
            {
                mBuffer.DumpAndSkip(11);
            }
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
        auto* objects = ObjectResource::GetInstance();

        //auto combatInventoryLocation = 0x46053;
        auto combatInventoryLocation = offset;
        //auto combatInventoryLocation = 0x45fe5;
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

            int capacity = mBuffer.GetUint8();
            int items = mBuffer.GetUint8();
            int items2 = mBuffer.GetUint8();

            mLogger.Info() << "Inventory #" << i << " "
                << std::hex << x << std::dec << " CBT: " << +combatNo 
                << " PER: " << +combatantNo << " Cap: " << capacity 
                << " items: " << items << std::endl;
            std::stringstream ss{""};
            for (int i = 0; i < items; i++)
            {
                auto xx = mBuffer.GetUint8();
                auto item = mBuffer.GetUint8();
                auto object = objects->GetObjectInfo(item);
                auto condition = mBuffer.GetUint8();
                auto modifiers = mBuffer.GetUint8();
                ss << object.name << " " << +xx << " cond: " 
                    << +condition <<" mod: " <<  +modifiers << std::endl;
            }
            mLogger.Info() << ss.str() << std::endl;
            mBuffer.Skip(1);
        }
    }

    mutable FileBuffer mBuffer;
    Logging::Logger mLogger;

    ObjectIndex mObjects;
    Chapter mChapter;
    Location mLocation;
    WorldClock mTime;
    Party mParty;
};

}
