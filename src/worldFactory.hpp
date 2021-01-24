#pragma once


#include "constants.hpp"
#include "coordinates.hpp"
#include "logger.hpp"
#include "tableResource.hpp"

#include "Exception.h"

#include "FileBuffer.h"
#include "FileManager.h"

#include "ImageResource.h"
#include "Palette.h"
#include "ScreenResource.h"
#include "TileWorldResource.h"
#include "TableResource.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>   
#include <iomanip>   
#include <iostream>   
#include <optional>
#include <sstream>
#include <cassert>   

namespace BAK {

class ZoneLabel
{
public:

    ZoneLabel(const std::string& zoneLabel)
    :
        mZoneLabel{zoneLabel}
    {}

    std::string GetHorizon() const
    {
        std::stringstream ss{""};
        ss << GetZone() << "H.SCX";
        return ss.str();
    }

    std::string GetTerrain() const
    {
        std::stringstream ss{""};
        ss << GetZone() << "L.SCX";
        return ss.str();
    }

    std::string GetSpriteSlot(unsigned i) const
    {
        std::stringstream ss{""};
        ss << GetZone() << "SLOT" << std::setfill('0') 
            << std::setw(1) << i << ".BMX";
        return ss.str();
    }

    std::string GetPalette() const
    {
        std::stringstream ss{""};
        ss << GetZone() << ".PAL";
        return ss.str();
    }

    std::string GetWorld() const
    {
        return mZoneLabel.substr(1,2);
    }

    std::string GetTable() const
    {
        std::stringstream ss{""};
        ss << GetZoneLabel() << ".TBL";
        return ss.str();
    }

    std::string GetZone() const
    {
        return mZoneLabel.substr(0, 3);
    }

    std::string GetZoneLabel() const
    {
        return mZoneLabel;
    }

private:
    const std::string mZoneLabel;
};

class Texture
{
public:
    using TextureType = std::vector<glm::vec4>;

    TextureType mTexture;
    unsigned mWidth;
    unsigned mHeight;
};


class TextureStore
{
public:

    TextureStore(
        const ZoneLabel& zoneLabel,
        const Palette& palette)
    :
        mTextures{},
        mTerrainOffset{0},
        mHorizonOffset{0},
        mMaxHeight{0},
        mMaxWidth{0},
        mMaxDim{0}
    {
        unsigned n = 0;
        unsigned textures = 0;
        bool found = true;

        while ( found )
        {
            auto spriteSlotLbl = zoneLabel.GetSpriteSlot(n);
            found = FileManager::GetInstance()->ResourceExists(spriteSlotLbl);
            if ( found )
            {
                ImageResource spriteSlot;
                FileManager::GetInstance()->Load(&spriteSlot, spriteSlotLbl);

                for (unsigned j = 0; j < spriteSlot.GetNumImages(); j++)
                {
                    assert(spriteSlot.GetImage(j));
                    const auto& img = *spriteSlot.GetImage(j);

                    auto image = Texture::TextureType{};
                    auto* pixels = img.GetPixels();

                    for (int i = 0; i < (img.GetWidth() * img.GetHeight()); i++)
                    {
                        auto color = palette.GetColor(pixels[i]);
                        // palette color 0 is transparency
                        image.push_back(
                            BAK::ToGlColor<float>(color, pixels[i] == 0));
                    }

                    // Need to invert the image over x axis for opengl
                    for (int x = 0; x < img.GetWidth(); x++)
                        for (int y = 0; y < (img.GetHeight() / 2); y++)
                            std::swap(
                                image[x + (y * img.GetWidth())],
                                image[x + ((img.GetHeight() - 1 - y) * img.GetWidth())]);

                    mTextures.push_back(
                        Texture{
                            image,
                            static_cast<unsigned>(img.GetWidth()),
                            static_cast<unsigned>(img.GetHeight())});

                    textures++;
                }
            }

            n++;
        }

        mTerrainOffset = textures;

        ScreenResource terrain;
        FileManager::GetInstance()->Load(&terrain, zoneLabel.GetTerrain());

        auto* pixels = terrain.GetImage()->GetPixels();
        auto width = terrain.GetImage()->GetWidth();

        auto startOff = 0;
        // FIXME: Can I find these in the data files somewhere?
        for (auto offset : {70, 20, 20, 32, 20, 27, 6, 5})
        {
            auto image = Texture::TextureType{};
            for (int i = startOff * width; i < (startOff + offset) * width; i++)
            {
                auto color = palette.GetColor(pixels[i]);
                image.push_back(
                    BAK::ToGlColor<float>(color, pixels[i] == 0));
            }

            startOff += offset;

            mTextures.push_back(
                Texture{
                    image,
                    static_cast<unsigned>(width),
                    static_cast<unsigned>(offset)});

            textures++;
        }

        mHorizonOffset = textures;
        /*
        ImageResource horizon;
        FileManager::GetInstance()->Load(
            &horizon,
            zoneLabel.GetHorizonLabel());

        for (unsigned j = 0; j < horizon.GetNumImages(); j++)
        {
            assert(horizon.GetImage(j));
            const auto& img = *horizon.GetImage(j);

            auto image = Texture::TextureType{};
            auto* pixels = img.GetPixels();

            for (int i = 0; i < (img.GetWidth() * img.GetHeight()); i++)
            {
                auto color = palette.GetColor(pixels[i]);
                // palette color 0 is transparency
                image.push_back(
                    BAK::ToGlColor<float>(color, pixels[i] == 0));
            }

            // Need to invert the image over x axis for opengl
            for (int x = 0; x < img.GetWidth(); x++)
                for (int y = 0; y < (img.GetHeight() / 2); y++)
                    std::swap(
                        image[x + (y * img.GetWidth())],
                        image[x + ((img.GetHeight() - 1 - y) * img.GetWidth())]);

            mTextures.push_back(
                Texture{
                    image,
                    static_cast<unsigned>(img.GetWidth()),
                    static_cast<unsigned>(img.GetHeight())});

            textures++;
        }
        */

        // Set max width and height
        mMaxHeight = std::max_element(
            mTextures.begin(), mTextures.end(),
            [](const auto &lhs, const auto& rhs){
            return (lhs.mHeight < rhs.mHeight);
            })->mHeight;

        mMaxWidth = std::max_element(
            mTextures.begin(), mTextures.end(),
            [](const auto &lhs, const auto& rhs){
            return (lhs.mWidth < rhs.mWidth);
            })->mWidth;

        mMaxDim = std::max(mMaxWidth, mMaxHeight);
    }

    const Texture& GetTexture(const unsigned i) const
    {
        assert(i < mTextures.size());
        return mTextures[i];
    }

    const std::vector<Texture>& GetTextures() const { return mTextures; }

    unsigned GetMaxDim() const { return mMaxDim; }
    unsigned GetTerrainOffset(BAK::Terrain t) const
    {
        return mTerrainOffset + static_cast<unsigned>(t);
    }
    unsigned GetHorizonOffset() const { return mHorizonOffset; }

private:
    std::vector<Texture> mTextures;

    unsigned mTerrainOffset;
    unsigned mHorizonOffset;
    unsigned mMaxHeight;
    unsigned mMaxWidth;
    unsigned mMaxDim;
};


class ZoneItem
{
public:
    ZoneItem(
        const std::string& name,
        DatInfo* datInfo,
        const TextureStore& textureStore)
    :
        mName{name},
        mDatItem{datInfo, textureStore}
    {}

    struct DatItem
    {
        DatItem(
            DatInfo* other,
            const TextureStore& textureStore)
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

            // FIXME: 400 -- the ground has sprite index != 0 for some reason...
            if (mSpriteIndex == 0 || mSpriteIndex > 400)
            {
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
            else
            {
                // Need this to set the right dimensions for the texture
                const auto& tex = textureStore.GetTexture(mSpriteIndex);
                auto width  = tex.mWidth * 10;
                auto height = tex.mHeight * 10;
                mVertices.emplace_back(-width, 0, height);
                mVertices.emplace_back(width, 0, height);
                mVertices.emplace_back(width, 0, 0);
                mVertices.emplace_back(-width, 0, 0);

                auto faces = std::vector<std::uint16_t>{};
                faces.emplace_back(0);
                faces.emplace_back(1);
                faces.emplace_back(2);
                faces.emplace_back(3);
                mFaces.emplace_back(faces);

                mPalettes.emplace_back(0x91);
                mColors.emplace_back(other->sprite);
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

private:
    std::string mName;

    DatItem mDatItem;

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


class ZoneItemStore
{
public:

    ZoneItemStore(
        const ZoneLabel& zoneLabel,
        const TextureStore& textureStore)
    :
        mZoneLabel{zoneLabel},
        mItems{}
    {
        TableResource table{};

        auto fb = FileBufferFactory::CreateFileBuffer(
            mZoneLabel.GetTable());
        table.Load(&fb);

        assert(table.GetMapSize() == table.GetDatSize());

        std::cout << std::endl;
        for (unsigned i = 0; i < table.GetMapSize(); i++)
        {
            mItems.emplace_back(
                table.GetMapItem(i),
                table.GetDatItem(i),
                textureStore);
        }
    }

    const ZoneLabel& GetZoneLabel() const { return mZoneLabel; }

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

    const std::vector<ZoneItem>& GetItems() const { return mItems; }

private:
    const ZoneLabel mZoneLabel;
    std::vector<ZoneItem> mItems;
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
                * 2.0f * glm::pi<float>()},
        mLocation{
            static_cast<float>(x),
            -1 * static_cast<float>(y),
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
    os << "[ Name: " << d.GetZoneItem().GetName() << " Type: " << d.mType << " Rot: " 
        << glm::to_string(d.mRotation)
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
        mCenter{},
        mTile{x, y},
        mItemInsts{}
    {
        LoadWorld(zoneItems, x, y);
    }

    void LoadWorld(const ZoneItemStore& zoneItems, unsigned x, unsigned y)
    {
        const auto& logger = Logging::LogState::GetLogger("World");

        std::stringstream str{""};
        str << "T" << std::setfill('0') << zoneItems.GetZoneLabel().GetWorld() 
            << std::setw(2) << x << std::setw(2) << y << ".WLD";
        auto fb = FileBufferFactory::CreateFileBuffer(str.str());
        logger.Debug() << "Loading Tile" << str.str() << std::endl;

        TileWorldResource world{};
        world.Load(&fb);

        for (unsigned i = 0; i < world.GetSize(); i++)
        {
            const auto& item = world.GetItem(i);
            /*if (item.type == static_cast<unsigned>(OBJECT_CENTER))
                mCenter = glm::vec3{
                    static_cast<int>(item.xloc),
                    0,
                    -static_cast<int>(item.yloc)};*/

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
    }

    const auto& GetTile() const { return mTile; }
    const auto& GetItems() const { return mItemInsts; }
    const auto GetCenter() const
    {
        return mCenter.value_or(
            GetItems().back().GetLocation());
    }

private:
    std::optional<glm::vec3> mCenter;
    glm::vec<2, unsigned> mTile;

    std::vector<WorldItemInstance> mItemInsts;
};


class WorldTileStore
{
public:
    WorldTileStore(const ZoneItemStore& zoneItems)
    :
        mWorlds{
            std::invoke([&zoneItems]()
            {
                std::vector<World> worlds{};
                
                // Min and max tile world indices
                static constexpr unsigned xMin = 9;
                static constexpr unsigned xMax = 24;

                static constexpr unsigned yMin = 9;
                static constexpr unsigned yMax = 24;

                for (unsigned x = xMin; x < xMax; x++)
                {
                    for (unsigned y = yMin; y < yMax; y++)
                    {
                        try
                        {
                            auto it = worlds.emplace_back(zoneItems, x, y);
                            for (const auto& itemInst : it.GetItems())
                            {
                                std::cout << itemInst << std::endl;
                            }
                        }
                        catch (const OpenError&)
                        {
                            Logging::LogDebug("WorldTileStore")
                                << "World: " << x << " , " << y 
                                << " does not exist" << std::endl;
                        }
                    }
                }

                return worlds;
            })
        }
    {}

    const std::vector<World>& GetTiles() const
    {
        return mWorlds;
    }

private:
    std::vector<World> mWorlds;
};

}
