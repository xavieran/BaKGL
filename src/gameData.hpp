#pragma once

#include <glm/glm.hpp>

#include "logger.hpp"

#include "FileBuffer.h"
#include "ObjectResource.h"

namespace BAK {

class GameData
{   
public:

    GameData(FileBuffer& mBuffer)
    :
        mBuffer{mBuffer},
        mLogger{
            Logging::LogState::GetLogger("GameData")},
        mZone{0}
    {

        mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;

        LoadChestContents();
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

        mLogger.Info() << "Loc: " << xloc << "," << yloc << std::endl;
        mLogger.Info() << "Tile: " << xtile << "," << ytile << std::endl;
        mLogger.Info() << "Pos: " << xpos << "," << ypos << std::endl;
        mLogger.Info() << "Heading: " << heading << std::endl;
        
        mLocus.mHeading = heading;
        mLocus.mPosition= {xpos, -ypos};
        mLocus.mTile = {xtile, ytile};
    }

    void LoadChestContents()
    {
        auto* objects = ObjectResource::GetInstance();

        mBuffer.Seek(0x3bea9);
        for (int j = 0; j < 20; j++)
        {
            auto chestNumber = mBuffer.GetUint8();
            mLogger.Info() << " Chest Number: " << +chestNumber << std::endl;
            mBuffer.Skip(17);
            auto chestMarker = mBuffer.GetUint8();
            auto chestItems = mBuffer.GetUint8();
            auto chestCapacity = mBuffer.GetUint8();
            auto xx = mBuffer.GetUint8();

            mLogger.Info() << "Chest: " << +chestMarker << " items: " << +chestItems 
                << " capacity: " << +chestCapacity << " X: " << +xx << std::endl;

            std::stringstream ss{""};
            int i = 0;
            for (; i < chestItems; i++)
            {
                auto item = mBuffer.GetUint8();
                auto object = objects->GetObjectInfo(item);
                auto condition = mBuffer.GetUint8();
                auto modifiers = mBuffer.GetUint8();
                auto yy = mBuffer.GetUint8();
                ss << object.name << " " << +yy << " cond: " 
                    << +condition <<" mod: " <<  +modifiers << std::endl;
            }

            for (; i < chestCapacity; i++)
            {
                mBuffer.Skip(4);
            }

            mLogger.Info() << "Items: " << ss.str() << std::endl;
            mBuffer.Skip(2);
        }

    }

    void LoadCombatStats()
    {
        // Combat statistics
        mBuffer.Seek(0x914b);
        for (int i = 0; i < 1698; i++)
        {
            mLogger.Info() << "Combat #" << std::dec << i << std::endl;
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
        auto numberCombatInventories = 1733;
        mBuffer.Seek(combatInventoryLocation);
        for (int i = 0; i < numberCombatInventories; i++)
        {
            mBuffer.Dump(13);
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

            mLogger.Info() << "Inventory #" << i << " CBT: " << +combatNo 
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
