#pragma once

//#include <loguru.hpp>

#define INFO 0

#define LOG_S(level) (std::cout)

#include "FileBuffer.h"

#include "TileWorldResource.h"
#include "TableResource.h"

#include "tableResource.hpp"

#include <functional>   
#include <iomanip>   
#include <iostream>   
#include <sstream>
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
            mRadius{
                static_cast<int>(other->xradius),
                static_cast<int>(other->yradius)},
            mFlags{other->flags},
            mExtras{other->extras},
            mExtraFlag{other->extraFlag},
            mTextureCoords{std::invoke([&other](){
                std::vector<Vector2D> coords{};
                for (const auto& coord : other->textureCoords)
                    coords.emplace_back(*coord);
                return coords;
            })},
            mOtherCoords{std::invoke([&other](){
                std::vector<Vector2D> coords{};
                for (const auto& coord : other->otherCoords)
                    coords.emplace_back(*coord);
                return coords;
            })}
        {}

        Vector2D mRadius;
        unsigned mFlags;
        unsigned mExtras;
        char mExtraFlag;
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
            for (const auto& face : other->faces)
            {
                mFaces.emplace_back(face);
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
        std::vector<std::vector<std::uint16_t>> mFaces;
    };

    const std::string& GetName() const { return mName; }

    const DatItem& GetDatItem() const { return mDatItem; }
    const GidItem& GetGidItem() const { return mGidItem; }

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
    WorldItemInstance(const WorldItem& worldItem, unsigned type, unsigned flags, unsigned x, unsigned y)
    :
        mWorldItem{worldItem},
        mType{type},
        mFlags{flags},
        mLocation{
            static_cast<int>(x),
            static_cast<int>(y)}
    {}

    const WorldItem& GetWorldItem() const { return mWorldItem; }
    const Vector2D& GetLocation() const { return mLocation; }
    unsigned GetType() const { return mType; }

private:
    const WorldItem& mWorldItem;

    unsigned mType;
    unsigned mFlags;
    Vector2D mLocation;

    friend std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d);
};

std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d)
{
    os << "[ Name: " << d.GetWorldItem().GetName() << " Type: " << d.mType << " Flags: " 
        << std::hex << d.mFlags << std::dec << " Loc: " << d.mLocation << "]";
    os << std::endl << "Pos: " << d.GetWorldItem().GetDatItem().mPos << " Vertices::" << std::endl;
    
    const auto& vertices = d.GetWorldItem().GetDatItem().mVertices;
    for (const auto& vertex : vertices)
    {
        os << "  " << vertex << std::endl;
    }
    return os;
}

class World
{
public:

    World(
        unsigned zone,
        unsigned x,
        unsigned y)
    :
        mItems{},
        mItemInsts{}
    {
        LoadWorld(zone, x, y);
    }

    void LoadWorld(unsigned zone, unsigned x, unsigned y)
    {
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
            mItems.emplace_back(
                table.GetMapItem(i),
                table.GetDatItem(i),
                table.GetGidItem(i));
        }

        TileWorldResource world{};
        {
            std::stringstream str{""};
            str << "T" << std::setfill('0') << std::setw(2) << zone << std::setw(2) << x << std::setw(2) << y << ".WLD";
            LOG_S(INFO) << "Loading world resource: " << str.str() << std::endl;
            auto fb = FileBufferFactory::CreateFileBuffer(str.str());
            world.Load(&fb);
        }

        for (unsigned i = 0; i < world.GetSize(); i++)
        {
            auto item = world.GetItem(i);
            if (item.type == static_cast<unsigned>(OBJECT_CENTER))
                mCenter = Vector2D{
                    static_cast<int>(item.xloc),
                    static_cast<int>(item.yloc)};
            else
                mItemInsts.emplace_back(
                    mItems[item.type],
                    item.type,
                    item.flags,
                    item.xloc,
                    item.yloc);
        }

        for (const auto& i : mItemInsts)
            std::cout << i << std::endl;
    }

    std::vector<WorldItem> mItems;
    std::vector<WorldItemInstance> mItemInsts;
    Vector2D mCenter;
};

}
