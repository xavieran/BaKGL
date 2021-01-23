#pragma once

#include <glm/glm.hpp>

#include "logger.hpp"

#include "FileBuffer.h"
#include "ObjectResource.h"

namespace BAK {

class GameData
{   
public:
/*
 * 0x011a0 - Start of FFFF??
 * 0x039c0 - End of FFFF??
 *
 * 0x04fb0 - Events Start??
 * 0x0913e - Events End??
 * 0x0914b - Combat Stats Start 
 * 0x3070a - Combat Stats End
 *
 * 0x31340 - ???
 * 
 * 0x3b621 - Start of Containers
 * 0x4378f - End of Containers ???
 * 0x46053 - Combat Inventory Start
 * 0x51720 - Combat Inventory End (1773)
 */

    GameData(FileBuffer& mBuffer)
    :
        mBuffer{mBuffer},
        mLogger{
            Logging::LogState::GetLogger("GameData")},
        mZone{0}
    {

        mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;

        //LoadContainer();
        LoadCombatInventories();
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

    void LoadEvents()
    {
        // Phillip a80 dialogue options
    }

    void LoadContainer()
    {
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
        mBuffer.Seek(0x4378f); // 60 C
        */

        for (int j = 0; j < 40; j++)
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

    void LoadCombatStats()
    {
        // Combat statistics
        mBuffer.Seek(0x914b);
        // ends at 3070a
        for (int i = 0; i < 1698; i++)
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
                ss << std::dec << stat << ": " << +mBuffer.GetUint8() << " " << +mBuffer.GetUint8() << " " << +mBuffer.GetUint8() << std::endl;
                mBuffer.Skip(2);
            }
            mBuffer.Dump(7);
            mBuffer.Skip(7);
            mLogger.Info() << ss.str() << std::endl;
        }
    }

    void LoadCombatInventories()
    {
        auto* objects = ObjectResource::GetInstance();

        auto combatInventoryLocation = 0x46053;
        //auto combatInventoryLocation = 0x45fe5;
        auto numberCombatInventories = 2000;
        mBuffer.Seek(combatInventoryLocation);
        for (int i = 0; i < numberCombatInventories; i++)
        {
            mBuffer.Dump(13);
            auto x = mBuffer.Tell();
            assert(mBuffer.GetUint8() == 0x64);
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
