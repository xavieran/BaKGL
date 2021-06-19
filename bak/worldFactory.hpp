#pragma once

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/encounter.hpp"
#include "com/logger.hpp"
#include "bak/resourceNames.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/meshObject.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"
#include "xbak/ImageResource.h"
#include "xbak/Palette.h"
#include "xbak/ScreenResource.h"
#include "xbak/TableResource.h"
#include "xbak/TileWorldResource.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>   
#include <functional>   
#include <iomanip>   
#include <iostream>   
#include <optional>
#include <sstream>
#include <unordered_map>

namespace BAK {

class ZoneTextureStore
{
public:

    ZoneTextureStore(
        const ZoneLabel& zoneLabel,
        const Palette& palette);

    const Graphics::Texture& GetTexture(const unsigned i) const
    {
        return mTextures.GetTexture(i);
    }

    const std::vector<Graphics::Texture>& GetTextures() const { return mTextures.GetTextures(); }

    unsigned GetMaxDim() const { return mTextures.GetMaxDim(); }
    unsigned GetTerrainOffset(BAK::Terrain t) const
    {
        return mTerrainOffset + static_cast<unsigned>(t);
    }
    unsigned GetHorizonOffset() const { return mHorizonOffset; }

private:
    Graphics::TextureStore mTextures;

    unsigned mTerrainOffset;
    unsigned mHorizonOffset;
};

class ZoneItem
{
public:
    ZoneItem(
        const std::string& name,
        const DatInfo& datInfo,
        const ZoneTextureStore& textureStore)
    :
        mName{name},
        mEntityFlags{datInfo.entityFlags},
        mScale{static_cast<float>(1 << datInfo.terrainClass)},
        mSpriteIndex{datInfo.sprite},
        mColors{},
        mVertices{},
        mPalettes{},
        mFaces{},
        mPush{}
    {
        // FIXME: 400 -- the ground has sprite index != 0 for some reason...
        if (mSpriteIndex == 0 || mSpriteIndex > 400)
        {
            for (const auto& vertex : datInfo.vertices)
            {
                assert(vertex);
                mVertices.emplace_back(BAK::ToGlCoord<int>(*vertex));
            }
            for (const auto& face : datInfo.faces)
            {
                mFaces.emplace_back(face);
            }
            for (const auto& palette : datInfo.paletteSources)
            {
                mPalettes.emplace_back(palette);
            }
            for (const auto& color : datInfo.faceColors)
            {
                mColors.emplace_back(color);
                if ((GetName().substr(0, 5) == "house"
                    || GetName().substr(0, 3) == "inn")
                    && (color == 190
                    || color == 191))
                    mPush.emplace_back(false);
                else if (GetName().substr(0, 4) == "blck"
                    && (color == 145
                    || color == 191))
                    mPush.emplace_back(false);
                else if (GetName().substr(0, 4) == "brid"
                    && (color == 147))
                    mPush.emplace_back(false);
                else if (GetName().substr(0, 4) == "temp"
                    && (color == 218
                    || color == 220
                    || color == 221))
                    mPush.emplace_back(false);
                else if (GetName().substr(0, 6) == "church"
                    && (color == 191
                    || color == 0
                    || color == 0))
                    mPush.emplace_back(false);
                else if (GetName().substr(0, 6) == "ground")
                    mPush.emplace_back(false);
                else
                    mPush.emplace_back(true);
            }
        }
        else
        {
            // Need this to set the right dimensions for the texture
            const auto& tex = textureStore.GetTexture(mSpriteIndex);
            auto width  = tex.GetWidth() * 5;
            auto height = tex.GetHeight() * 5;
            mVertices.emplace_back(-width, height, 0);
            mVertices.emplace_back(width, height, 0);
            mVertices.emplace_back(width, 0, 0);
            mVertices.emplace_back(-width, 0, 0);

            auto faces = std::vector<std::uint16_t>{};
            faces.emplace_back(0);
            faces.emplace_back(1);
            faces.emplace_back(2);
            faces.emplace_back(3);
            mFaces.emplace_back(faces);
            mPush.emplace_back(false);

            mPalettes.emplace_back(0x91);
            mColors.emplace_back(datInfo.sprite);
        }

        assert((mFaces.size() == mColors.size())
            && (mFaces.size() == mPalettes.size())
            && (mFaces.size() == mPush.size()));
    }

    void SetPush(unsigned i){ mPush[i] = true; }
    const std::string& GetName() const { return mName; }
    bool IsSprite() const { return mSpriteIndex > 0 && mSpriteIndex < 400; }
    const auto& GetColors() const { return mColors; }
    const auto& GetFaces() const { return mFaces; }
    const auto& GetPush() const { return mPush; }
    const auto& GetPalettes() const { return mPalettes; }
    const auto& GetVertices() const { return mVertices; }
    const auto& GetScale() const { return mScale; }
    bool GetClickable() const
    {
        for (std::string s : {
            "ground",
            "zero",
            "one",
            "tree",
            "cryst",
            "t0",
            "g0",
            "r0",
            "spring",
            "fall",
            "landscp",
            "m_"})
        {
            if (mName.substr(0, s.length()) == s)
                return false;
        }
        return true;
    }

private:
    std::string mName;
    unsigned mEntityFlags;
    float mScale;
    unsigned mSpriteIndex;
    std::vector<std::uint8_t> mColors;
    std::vector<glm::vec<3, int>> mVertices;
    std::vector<std::uint8_t> mPalettes;
    std::vector<std::vector<std::uint16_t>> mFaces;
    std::vector<bool> mPush;

    friend std::ostream& operator<<(std::ostream& os, const ZoneItem& d);
};

std::ostream& operator<<(std::ostream& os, const ZoneItem& d);

Graphics::MeshObject ZoneItemToMeshObject(
    const ZoneItem& item,
    const ZoneTextureStore& store,
    const Palette& pal);

class ZoneItemStore
{
public:

    ZoneItemStore(
        const ZoneLabel& zoneLabel,
        // Should one really need a texture store to load this?
        const ZoneTextureStore& textureStore)
    :
        mZoneLabel{zoneLabel},
        mItems{}
    {
        TableResource table{};

        auto fb = FileBufferFactory::CreateFileBuffer(
            mZoneLabel.GetTable());
        table.Load(&fb);

        assert(table.GetMapSize() == table.GetDatSize());

        for (unsigned i = 0; i < table.GetMapSize(); i++)
        {
            assert(table.GetDatItem(i) != nullptr);
            mItems.emplace_back(
                table.GetMapItem(i),
                *table.GetDatItem(i),
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
    std::vector<ZoneItem>& GetItems() { return mItems; }

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
        const Vector3D& rotation,
        const Vector3D& location)
    :
        mZoneItem{zoneItem},
        mType{type},
        mRotation{BAK::ToGlAngle(rotation)},
        mLocation{BAK::ToGlCoord<float>(location)},
        mBakLocation{location.GetX(), location.GetY()}
    {}

    const ZoneItem& GetZoneItem() const { return mZoneItem; }
    const glm::vec3& GetRotation() const { return mRotation; }
    const glm::vec3& GetLocation() const { return mLocation; }
    const glm::vec<2, unsigned>& GetBakLocation() const { return mBakLocation; }
    unsigned GetType() const { return mType; }

private:
   const ZoneItem& mZoneItem;

    unsigned mType;
    glm::vec3 mRotation;
    glm::vec3 mLocation;
    glm::vec<2, unsigned> mBakLocation;

    friend std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d);
};

std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d);

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
        mItemInsts{},
        mEncounters{}
    {
        LoadWorld(zoneItems, x, y);
    }

    void LoadWorld(const ZoneItemStore& zoneItems, unsigned x, unsigned y)
    {
        const auto& logger = Logging::LogState::GetLogger("World");
        const auto tile = zoneItems.GetZoneLabel().GetTileWorld(x, y);
        auto fb = FileBufferFactory::CreateFileBuffer(tile);
        logger.Debug() << "Loading tile: " << tile << std::endl;

        TileWorldResource world{};
        world.Load(&fb);

        for (unsigned i = 0; i < world.GetSize(); i++)
        {
            const auto& item = world.GetItem(i);
            if (item.type == static_cast<unsigned>(OBJECT_CENTER))
                mCenter = ToGlCoord<float>(item.mLocation);

            mItemInsts.emplace_back(
                zoneItems.GetZoneItem(item.type),
                item.type,
                item.mRotation,
                item.mLocation);
        }
        {
            try
            {
                auto fb = FileBufferFactory::CreateFileBuffer(
                    zoneItems.GetZoneLabel().GetTileData(x, y));
                mEncounters = BAK::LoadEncounters(fb, 1, mTile);
            }
            catch (const OpenError&)
            {
                logger.Spam() << "No tile data for: " << mTile << std::endl;
            }
        }
    }

    const auto& GetTile() const { return mTile; }
    const auto& GetItems() const { return mItemInsts; }
    const auto& GetEncounters() const { return mEncounters; }
    auto GetCenter() const
    {
        return mCenter.value_or(
            GetItems().front().GetLocation());
    }

private:
    std::optional<glm::vec3> mCenter;
    glm::vec<2, unsigned> mTile;

    std::vector<WorldItemInstance> mItemInsts;
    std::vector<Encounter> mEncounters;
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
                        }
                        catch (const OpenError&)
                        {
                            Logging::LogSpam("WorldTileStore")
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
