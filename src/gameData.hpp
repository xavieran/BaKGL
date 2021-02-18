#pragma once

#include <glm/glm.hpp>

#include "constants.hpp"
#include "encounter.hpp"
#include "logger.hpp"
#include "resourceNames.hpp"

#include "FileBuffer.h"
#include "DialogResource.h"
#include "ObjectResource.h"

#include <memory>

namespace BAK {

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
  ** DEF_TOWN.DAT
  * - Seems to affect zone crossing (e.g. from Tyr-Sog to highcastle etc.
  *
  * T******.DAT
  * How many entries??
  * 01 00 01 00
  * Potentially coordinates???
  * 19 0f 23 07
  * 0d - combat number/index?
  * 0d 00 00 00
  *
  *
  *
  */

    GameData(FileBuffer& mBuffer)
    :
        mBuffer{mBuffer},
        mLogger{
            Logging::LogState::GetLogger("GameData")},
        mZone{0}
    {

        mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
        ZoneLabel zone{"Z01"};
        LoadTileData(1, zone, 10, 15);
        return;
        LoadLocation();
        LoadCombatStats(0xdb, 6);
        LoadCombatInventories(0x3a7f7, 6);
        LoadInventoryOffsetsP();
        LoadContainer();
        LoadCombatEntityLists();
        LoadCombatInventories(0x46053, 1733);
        LoadCombatStats(0x914b, 1698);
    }

    void LoadLocation()
    {
        mBuffer.Seek(0x5a);
        mBuffer.Skip(16);

        int xloc = mBuffer.GetUint32LE();
        int yloc = mBuffer.GetUint32LE();
        mBuffer.Skip(4);

        mZone = mBuffer.GetUint8();
        assert(mZone < 12);

        mLogger.Info() << "Zone:" << mZone << std::endl;

        int xtile = mBuffer.GetUint8();
        int ytile = mBuffer.GetUint8();
        int xpos = mBuffer.GetUint32LE();
        int ypos = mBuffer.GetUint32LE();

        mBuffer.Skip(5);
        int heading = mBuffer.GetUint16LE();

        mLogger.Info() << "Loc: " << std::dec << xloc << "," << yloc << std::endl;
        mLogger.Info() << "Tile: " << xtile << "," << ytile << std::endl;
        mLogger.Info() << "Pos: " << xpos << "," << ypos << std::endl;
        mLogger.Info() << "Heading: " << heading << std::endl;
        
        mLocus.mHeading = heading;
        mLocus.mPosition= {xpos, -ypos};
        mLocus.mTile = {xtile, ytile};
    }

    void LoadTileData(unsigned chapter, const ZoneLabel& zone, unsigned x, unsigned y)
    {
        mLogger.Info() << "Loading data for: " << zone.GetTileData(x, y) << std::endl;
        auto fb = FileBufferFactory::CreateFileBuffer(zone.GetTileData(x, y));

        fb.Skip((chapter - 1) * 192);
        
        fb.Dump(20);
        unsigned numberOfEncounters = fb.GetUint16LE();
        for (unsigned i = 0; i < numberOfEncounters; i++)
        {
            auto encounterType = static_cast<BAK::EncounterType>(fb.GetUint16LE());
            unsigned xLoc = fb.GetUint16LE();
            unsigned yLoc = fb.GetUint16LE();
            unsigned encounterIndex = fb.GetUint16LE();
            // Don't know
            fb.GetUint16LE();
            fb.GetUint16LE();
            fb.GetUint8();
            unsigned saveAddr = fb.GetUint16LE();
            fb.GetUint16LE();
            fb.GetUint16LE();
            mLogger.Info() << "ZoneEntry: " << BAK::EncounterTypeToString(encounterType) 
                << " loc: " << xLoc << "," << yLoc 
                << " index: " << encounterIndex 
                << " saveAddr: " << saveAddr << " 0x" 
                << std::hex << saveAddr << std::dec << std::endl;
        }
    }

    void LoadContainer()
    {
        mLogger.Info() << "Loading containers" << std::endl;
        auto* objects = ObjectResource::GetInstance();

        // TombStone
        mBuffer.Seek(0x3b621); // 36 items 1

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
        */
        mBuffer.Seek(0x4378f); // 60 C

        for (int j = 0; j < 60; j++)
        {
            mLogger.Info() << " Container: " << j
                << " addr: " << std::hex << mBuffer.Tell() << std::dec << std::endl;

            mBuffer.Dump(4);

            auto aLoc = mBuffer.GetUint16LE();
            auto bLoc = mBuffer.GetUint16LE();
            // bLoc == C3 dbody
            // bLoc == C4 hole dirt
            // bLoc == C5 Bag
            auto xLoc = mBuffer.GetUint32LE();
            auto yLoc = mBuffer.GetUint32LE();
            auto chestNumber   = mBuffer.GetUint8();
            auto chestItems    = mBuffer.GetUint8();
            auto chestCapacity = mBuffer.GetUint8();
            auto containerType = mBuffer.GetUint8();

            assert(chestNumber == 6 || chestNumber == 9 || chestCapacity > 0);

            const auto Container = [](auto x) -> std::string
            {
                //if ((x & 0x04) == 0x04) return "Shop";
                if ((x & 0x04) == 0x04) return "Shop";
                else return "Dunno";
            };
            mLogger.Info() << "Loc: " << aLoc << ":" << bLoc << " " 
                << xLoc << "," << yLoc << " #" << +chestNumber << " items: " 
                << +chestItems << " capacity: " << +chestCapacity 
                << " Tp: " << +containerType << " " 
                << Container(containerType) << std::endl;

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
            }

            for (; i < chestCapacity; i++)
            {
                mBuffer.Skip(4);
            }

            mLogger.Info() << "Items: \n" << ss.str() << std::endl;
            mBuffer.Dump(6);
            mBuffer.Skip(4);
            mBuffer.Skip(2);

            if (Container(containerType) == "Shop")
            {
                mBuffer.Dump(16);
                mBuffer.Skip(16);
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
                mBuffer.Dump(4);
                mBuffer.Skip(4);
            }
            else if (containerType == 8)
            {
                mBuffer.Dump(3);
                mBuffer.Skip(3);
            }
            else if (containerType == 10)
            {
                mBuffer.Dump(9);
                mBuffer.Skip(9);
            }
            else if (containerType == 16)
            {
                mBuffer.Skip(-2);
            }
            else if (containerType == 17)
            {
                if (chestNumber != 4 && chestCapacity == 5)
                {
                    mBuffer.Dump(3 * 8 + 1);
                    mBuffer.Skip(3 * 8 + 1);
                }
                mBuffer.Skip(2);
            }
            else if (containerType == 25)
            {
                mBuffer.Dump(11);
                mBuffer.Skip(11);
            }
            std::cout << std::endl;
        }
    }

    void LoadInventoryOffsetsP()
    {
        constexpr unsigned inventoryOffsetsStart = 0x11a3;
        mBuffer.Seek(inventoryOffsetsStart);

        mLogger.Info() << "Inventory Offsets Start @" 
            << std::hex << inventoryOffsetsStart << std::dec << std::endl;

        for (unsigned i = 0; i < 10; i++)
        {
            std::stringstream ss{};
            ss << "Inventory #" << i << " : " << mBuffer.GetUint16LE();
            for (unsigned i = 0; i < 5; i++)
            {
                unsigned addr = mBuffer.GetUint32LE();
                ss << " a: " << std::hex << addr;
            }
            mLogger.Info() << ss.str() << std::endl;
        }

        mLogger.Info() << "Inventory Offsets End @" 
            << std::hex << mBuffer.Tell() << std::dec << std::endl;

    }

    void LoadCombatEntityLists()
    {
        constexpr unsigned combatEntityListStart = 0x1383;
        mBuffer.Seek(combatEntityListStart);

        mLogger.Info() << "Combat Entity Lists Start @" 
            << std::hex << combatEntityListStart << std::dec << std::endl;

        for (int i = 0; i < 700; i++)
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
                ss << std::dec << stat << ": " << +mBuffer.GetUint8() << " " << +mBuffer.GetUint8() << " " << +mBuffer.GetUint8() << " ";
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
            assert(mBuffer.GetUint8() == 0x0a);
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

    struct Locus
    {
        int mHeading;
        glm::vec<2, int> mPosition;
        glm::vec<2, int> mTile;
    };
    
    FileBuffer& mBuffer;
    Logging::Logger mLogger;

    Locus mLocus;
    unsigned mZone;
};

}
