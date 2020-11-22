#pragma once

//#include <loguru.hpp>

#define INFO 0

#define LOG_S(level) (std::cout)

#include "FileBuffer.h"

#include "TileWorldResource.h"
#include "TableResource.h"

#include "tableResource.hpp"

#include <sstream>
#include <iomanip>   
#include <iostream>   
#include <cassert>   

namespace BAK {

class WorldItem
{
public:
    WorldItem(
        const std::string& name,
        DatInfo* datInfo,
        GidInfo* gidInfo)
    :
        mName{name},
        mDatItem{datInfo},
        mGidItem{gidInfo}
    {}

    struct GidItem
    {
        GidItem(GidInfo* other)
        :
            mTextureCoords{},
            mOtherCoords{}
        {
            assert(other);
            mXRadius = other->xradius;
            mYRadius = other->yradius;
            mFlags = other->flags;
            for (const auto& coord : other->textureCoords)
            {
                assert(coord);
                mTextureCoords.emplace_back(*coord);
            }

            for (const auto& coord : other->otherCoords)
            {
                assert(coord);
                mOtherCoords.emplace_back(*coord);
            }
        }
        unsigned mXRadius;
        unsigned mYRadius;
        unsigned mFlags;
        std::vector<Vector2D> mTextureCoords;
        std::vector<Vector2D> mOtherCoords;
    };

    struct DatItem
    {
        DatItem(DatInfo* other)
        :
            mVertices{}
        {
            assert(other);
            mEntityFlags  = other->entityFlags;
            mEntityType   = BAK::EntityType{other->entityType};
            mTerrainType  = BAK::TerrainType{other->terrainType};
            mTerrainClass = BAK::TerrainClass{other->terrainClass};
            mSpriteIndex  = other->sprite;
            mMin = other->min;
            mMax = other->max;
            mPos = other->pos;
            for (const auto& vertex : other->vertices)
            {
                assert(vertex);
                mVertices.emplace_back(*vertex);
            }
        }

        unsigned mEntityFlags;
        EntityType mEntityType;
        TerrainType mTerrainType;
        TerrainClass mTerrainClass;
        unsigned mSpriteIndex;
        Vector3D mMin;
        Vector3D mMax;
        Vector3D mPos;
        std::vector<Vector3D> mVertices;
    };

    const std::string& GetName() const { return mName; }
private:
    std::string mName;
    DatItem mDatItem;
    GidItem mGidItem;

    friend std::ostream& operator<<(std::ostream& os, const WorldItem& d);
};

std::ostream& operator<<(std::ostream& os, const WorldItem& d)
{
    os << d.mName;
    return os;
}

class WorldItemInstance
{
public:
    WorldItemInstance(const WorldItem& worldItem, unsigned flags, unsigned x, unsigned y)
    :
        mWorldItem{worldItem},
        mFlags{flags},
        mLocation{static_cast<int>(x), static_cast<int>(y)}
    {}

    const WorldItem& GetWorldItem() const { return mWorldItem; }

private:
    const WorldItem& mWorldItem;
    unsigned mFlags;
    Vector2D mLocation;

    friend std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d);
};

std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d)
{
    os << "[ Name: " << d.GetWorldItem().GetName() << " Flags: " 
        << std::hex << d.mFlags << std::dec << " Loc: " << d.mLocation << "]";
    return os;
}

class WorldFactory
{
public:
    static std::vector<WorldItem> LoadWorld(unsigned zone, const std::string& wld)
    {
        std::vector<WorldItem> items;
        std::vector<WorldItemInstance> itemInsts;
        
        TableResource table{};
        {
            std::stringstream str{""};
            str << "Z" << std::setfill('0') << std::setw(2) << zone << ".TBL";
            LOG_S(INFO) << "Loading table resource: " << str.str();
            auto fb = FileBufferFactory::CreateFileBuffer(str.str());
            table.Load(&fb);
        }
    
        assert((table.GetMapSize() == table.GetDatSize()) 
            && (table.GetDatSize() == table.GetGidSize()));

        std::cout << std::endl;
        for (unsigned i = 0; i < table.GetMapSize(); i++)
        {
            items.emplace_back(
                table.GetMapItem(i),
                table.GetDatItem(i),
                table.GetGidItem(i));
        }

        TileWorldResource world{};
        {
            std::stringstream str{""};
            str << "T" << std::setfill('0') << std::setw(2) << zone << wld << ".WLD";
            LOG_S(INFO) << "Loading world resource: " << str.str() << std::endl;
            auto fb = FileBufferFactory::CreateFileBuffer(str.str());
            world.Load(&fb);
        }

        for (unsigned i = 0; i < world.GetSize(); i++)
        {
            auto item = world.GetItem(i);
            itemInsts.emplace_back(items[item.type], item.flags, item.xloc, item.yloc);
        }

        for (const auto& i : itemInsts)
            std::cout << i << std::endl;
        
        return items;
    }

};

}
