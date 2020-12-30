#pragma once

#include "FileBuffer.h"
#include "FileManager.h"

#include "ImageResource.h"
#include "Palette.h"
#include "TileWorldResource.h"
#include "TableResource.h"

#include "logger.hpp"
#include "tableResource.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>   
#include <iomanip>   
#include <iostream>   
#include <sstream>
#include <cassert>   

namespace BAK {

class ZoneItem
{
public:
    ZoneItem(
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
            mColors{},
            mVertices{},
            mPalettes{},
            mFaces{}
        {
            assert(other);
            mEntityFlags  = other->entityFlags;
            mEntityType   = BAK::EntityType{other->entityType};
            mTerrainType  = BAK::TerrainType{other->terrainType};
            mScale        = static_cast<double>(1 << other->terrainClass);
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
            for (const auto& palette : other->paletteSources)
            {
                mPalettes.emplace_back(palette);
            }
            for (const auto& color : other->faceColors)
            {
                mColors.emplace_back(color);
            }
        }

        unsigned mEntityFlags;
        EntityType mEntityType;
        TerrainType mTerrainType;
        double mScale;
        unsigned mSpriteIndex;
        Vector3D mMin;
        Vector3D mMax;
        Vector3D mPos;
        std::vector<std::uint8_t> mColors;
        std::vector<Vector3D> mVertices;
        std::vector<std::uint8_t> mPalettes;
        std::vector<std::vector<std::uint16_t>> mFaces;
    };

    const std::string& GetName() const { return mName; }

    const DatItem& GetDatItem() const { return mDatItem; }
    const GidItem& GetGidItem() const { return mGidItem; }

private:
    std::string mName;

    DatItem mDatItem;
    GidItem mGidItem;

    friend std::ostream& operator<<(std::ostream& os, const ZoneItem& d);
};

std::ostream& operator<<(std::ostream& os, const ZoneItem& d)
{
    os << d.mName << " :: "
        << d.GetDatItem().mVertices << "\n";
    for (const auto& face : d.GetDatItem().mFaces)
    {
        for (const auto i : face)
        {
            os << " :: " << i;
        }

        os << "\n";
    }
        
    return os;
}

class Texture
{
public:
    using TextureType = std::vector<glm::vec3>;

    TextureType mTexture;
    unsigned mWidth;
    unsigned mHeight;
};

class ZoneItemStore
{
public:

    ZoneItemStore(
        std::string zoneLabel,
        const Palette& palette)
    :
        mZoneLabel{zoneLabel},
        mItems{},
        mTextures{}
    {
        TableResource table{};
        {
            std::stringstream str{""};
            str << zoneLabel << ".TBL";
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
            
        unsigned n = 0;
        bool found = true;
        while ( found )
        {
            std::stringstream spriteStream;
            spriteStream << zoneLabel << "SLOT" << std::setfill('0') 
                << std::setw ( 1 ) << n << ".BMX";
            found = FileManager::GetInstance()->ResourceExists ( spriteStream.str() );
            if ( found )
            {
                ImageResource spriteSlot;
                FileManager::GetInstance()->Load ( &spriteSlot, spriteStream.str() );
                for ( unsigned int j = 0; j < spriteSlot.GetNumImages(); j++ )
                {
                    assert(spriteSlot.GetImage(j));
                    const auto& img = *spriteSlot.GetImage(j);
                    auto image = Texture::TextureType{};
                    for (int x = 0; x < img.GetWidth(); x++)
                    {
                        for (int y = 0; y < img.GetHeight(); y++)
                        {
                            auto color = palette.GetColor(img.GetPixel(x, y));
                            image.emplace_back(
                                static_cast<float>(color.r) / 256,
                                static_cast<float>(color.g) / 256,
                                static_cast<float>(color.b) / 256);
                        }
                    }

                    mTextures.push_back(
                        Texture{
                            image,
                            static_cast<unsigned>(img.GetWidth()),
                            static_cast<unsigned>(img.GetHeight())});
                }
            }

            n++;
        }
    }

    const std::string& GetZoneLabel() const { return mZoneLabel; }

    const ZoneItem& GetZoneItem(const unsigned i) const
    {
        assert(i < mItems.size());
        return mItems[i];
    }

    const ZoneItem& GetZoneItem(const std::string& name) const
    {
        auto it = std::find_if(mItems.begin(), mItems.end(),
            [&name](const auto& item){
                return name == item.GetName();
            });

        assert(it != mItems.end());
        return *it;
    }

    const Texture& GetTexture(const unsigned i) const
    {
        assert(i < mTextures.size());
        return mTextures[i];
    }

    const std::vector<ZoneItem>& GetItems() const { return mItems; }

private:
    const std::string mZoneLabel;
    std::vector<ZoneItem> mItems;
    std::vector<Texture> mTextures;
};

class WorldItemInstance
{
public:
    WorldItemInstance(
        const ZoneItem& zoneItem,
        unsigned type,
        unsigned xrot,
        unsigned yrot,
        unsigned zrot,
        unsigned x,
        unsigned y,
        unsigned z)
    :
        mZoneItem{zoneItem},
        mType{type},
        // Convert to radians - all these static casts are kinda disgusting
        mRotation{
            (glm::vec3{
                static_cast<float>(xrot),
                static_cast<float>(yrot),
                static_cast<float>(zrot)}
            / static_cast<float>(0xffff))
            * static_cast<float>(2)
            * glm::pi<float>()},
        mLocation{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(z)}
    {}

    const ZoneItem& GetZoneItem() const { return mZoneItem; }
    const glm::vec3& GetRotation() const { return mRotation; }
    const glm::vec3& GetLocation() const { return mLocation; }
    unsigned GetType() const { return mType; }

private:
    const ZoneItem& mZoneItem;

    unsigned mType;
    glm::vec3 mRotation;
    glm::vec3 mLocation;

    friend std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d);
};

std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d)
{
    os << "[ Name: " << d.GetZoneItem().GetName() << " Type: " << d.mType << " Flags: " 
        << std::hex << glm::to_string(d.mRotation) << std::dec 
        << " Loc: " << glm::to_string(d.mLocation) << "]";
    os << std::endl << "Pos: " << d.GetZoneItem().GetDatItem().mPos << " Vertices::" << std::endl;
    
    const auto& vertices = d.GetZoneItem().GetDatItem().mVertices;
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
        const ZoneItemStore& zoneItems,
        unsigned x,
        unsigned y)
    :
        mItemInsts{}
    {
        LoadWorld(zoneItems, x, y);
    }

    void LoadWorld(const ZoneItemStore& zoneItems, unsigned x, unsigned y)
    {
        const auto& logger = Logging::LogState::GetLogger("World");

        std::stringstream str{""};
        str << "T" << std::setfill('0') << zoneItems.GetZoneLabel().substr(1,2) << std::setw(2) << x << std::setw(2) << y << ".WLD";
        logger.Debug() << "Loading " << str.str() << std::endl;
        auto fb = FileBufferFactory::CreateFileBuffer(str.str());

        TileWorldResource world{};
        world.Load(&fb);

        for (unsigned i = 0; i < world.GetSize(); i++)
        {
            auto item = world.GetItem(i);
            if (item.type == static_cast<unsigned>(OBJECT_CENTER))
                mCenter = glm::vec3{
                    static_cast<int>(item.xloc),
                    0,
                    static_cast<int>(item.yloc)};

            mItemInsts.emplace_back(
                zoneItems.GetZoneItem(item.type),
                item.type,
                item.xrot,
                item.yrot,
                item.zrot,
                item.xloc,
                item.yloc,
                item.zloc
                );
        }

        for (const auto& i : mItemInsts)
            std::cout << i << std::endl;
    }

    std::vector<WorldItemInstance> mItemInsts;
    glm::vec3 mCenter;
};

}
