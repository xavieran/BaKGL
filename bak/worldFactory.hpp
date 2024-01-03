#pragma once

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/model.hpp"
#include "bak/monster.hpp"
#include "bak/resourceNames.hpp"
#include "bak/textureFactory.hpp"
#include "bak/worldItem.hpp"
#include "bak/zoneReference.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/meshObject.hpp"

#include "bak/fileBufferFactory.hpp"

#include <functional>   
#include <optional>

namespace BAK {

enum class EntityType
{
    TERRAIN    =  0,
    EXTERIOR   =  1,
    BRIDGE     =  2,
    INTERIOR   =  3,
    HILL       =  4,
    TREE       =  5,
    CHEST      =  6,
    DEADBODY1  =  7,
    FENCE      =  8,
    GATE       =  9, // RIFT GATE
    BUILDING   = 10,
    TOMBSTONE  = 12,
    SIGN       = 13,
    TUNNEL1    = 14, // ALSO TUNNEL...
    PIT        = 15,
    DEADBODY2  = 16,
    DIRTPILE   = 17,
    CORN       = 18,
    FIRE       = 19,
    ENTRANCE   = 20,
    GROVE      = 21,
    FERN       = 22,
    DOOR       = 23,
    CRYST      = 24,
    ROCKPILE   = 25,
    BUSH1      = 26,
    BUSH2      = 27,
    BUSH3      = 28,
    SLAB       = 29,
    STUMP      = 30,
    WELL       = 31,
    ENGINE     = 33,
    SCARECROW  = 34,
    TRAP       = 35,
    CATAPULT   = 36,
    COLUMN     = 37,
    LANDSCAPE  = 38,
    TUNNEL2    = 39, // with tunnel
    BAG        = 41,
    LADDER     = 42
};

class ZoneTextureStore
{
public:

    ZoneTextureStore(
        const ZoneLabel& zoneLabel,
        const BAK::Palette& palette);

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
        const Model& model,
        const ZoneTextureStore& textureStore)
    :
        mName{model.mName},
        mEntityFlags{model.mEntityFlags},
        mEntityType{static_cast<EntityType>(model.mEntityType)},
        mScale{static_cast<float>(1 << model.mScale)},
        mSpriteIndex{model.mSprite},
        mColors{},
        mVertices{},
        mPalettes{},
        mFaces{},
        mPush{}
    {
        if (mSpriteIndex == 0 || mSpriteIndex > 400)
        {
            for (const auto& vertex : model.mVertices)
            {
                mVertices.emplace_back(BAK::ToGlCoord<int>(vertex));
            }
            for (const auto& component : model.mComponents)
            {
                for (const auto& mesh : component.mMeshes)
                {
                    assert(mesh.mFaceOptions.size() > 0);
                    // Only show the first face option. These typically correspond to 
                    // animation states, e.g. for the door, or catapult, or rift gate.
                    // Will need to work out how to handle animated things later...
                    const auto& faceOption = mesh.mFaceOptions[0];
                    //for (const auto& faceOption : mesh.mFaceOptions)
                    {
                        for (const auto& face : faceOption.mFaces)
                        {
                            mFaces.emplace_back(face);
                        }
                        for (const auto& palette : faceOption.mPalettes)
                        {
                            mPalettes.emplace_back(palette);
                        }
                        for (const auto& colorVec : faceOption.mFaceColors)
                        {
                            const auto color = colorVec.x;
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
                                || color == 0))
                                mPush.emplace_back(false);
                            else if (GetName().substr(0, 6) == "ground")
                                mPush.emplace_back(false);
                            else
                                mPush.emplace_back(true);
                        }
                    }
                }
            }
        }
        else
        {
            // Need this to set the right dimensions for the texture
            const auto& tex = textureStore.GetTexture(mSpriteIndex);
            const auto spriteScale = 7.0f;
            auto width  = static_cast<int>(static_cast<float>(tex.GetWidth()) * (spriteScale * .75));
            auto height = tex.GetHeight() * spriteScale;
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
            mColors.emplace_back(model.mSprite);
        }

        ASSERT((mFaces.size() == mColors.size())
            && (mFaces.size() == mPalettes.size())
            && (mFaces.size() == mPush.size()));
    }

    ZoneItem(
        unsigned i,
        const BAK::MonsterNames& monsters,
        const ZoneTextureStore& textureStore)
    :
        mName{monsters.GetMonsterAnimationFile(MonsterIndex{i})},
        mEntityFlags{0},
        mEntityType{EntityType::DEADBODY1},
        mScale{1},
        mSpriteIndex{i + textureStore.GetHorizonOffset()},
        mColors{},
        mVertices{},
        mPalettes{},
        mFaces{},
        mPush{}
    {
        // Need this to set the right dimensions for the texture
        const auto& tex = textureStore.GetTexture(mSpriteIndex);
            
        const auto spriteScale = 7.0f;
        auto width  = static_cast<int>(static_cast<float>(tex.GetWidth()) * (spriteScale * .75));
        auto height = tex.GetHeight() * spriteScale;
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
        mColors.emplace_back(mSpriteIndex);

        ASSERT((mFaces.size() == mColors.size())
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
    auto GetScale() const { return mScale; }
    bool GetClickable() const
    {
        //return static_cast<unsigned>(mEntityType) > 5;
        for (std::string s : {
            "ground",
            "genmtn",
            "zero",
            "one",
            "bridge",
            "fence",
            "tree",
            "db0",
            "db1",
            "db2",
            "db8",
            "t0",
            "g0",
            "r0",
            "spring",
            "fall",
            "landscp",
            // Mine stuff
            "m_r",
            "m_1",
            "m_2",
            "m_3",
            "m_4",
            "m_b",
            "m_c",
            "m_h"})
        {
            if (mName.substr(0, s.length()) == s)
                return false;
        }
        return true;
    }

    EntityType GetEntityType() const { return mEntityType; }

private:
    std::string mName;
    unsigned mEntityFlags;
    EntityType mEntityType;
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
        auto fb = FileBufferFactory::Get()
            .CreateDataBuffer(mZoneLabel.GetTable());
        const auto models = LoadTBL(fb);

        for (unsigned i = 0; i < models.size(); i++)
        {
            mItems.emplace_back(
                models[i],
                textureStore);
        }
    }

    const ZoneLabel& GetZoneLabel() const { return mZoneLabel; }

    const ZoneItem& GetZoneItem(const unsigned i) const
    {
        ASSERT(i < mItems.size());
        return mItems[i];
    }

    const ZoneItem& GetZoneItem(const std::string& name) const
    {
        auto it = std::find_if(mItems.begin(), mItems.end(),
            [&name](const auto& item){
            return name == item.GetName();
            });

        ASSERT(it != mItems.end());
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
        const WorldItem& worldItem)
    :
        mZoneItem{zoneItem},
        mType{worldItem.mItemType},
        mRotation{BAK::ToGlAngle(worldItem.mRotation)},
        mLocation{BAK::ToGlCoord<float>(worldItem.mLocation)},
        mBakLocation{worldItem.mLocation.x, worldItem.mLocation.y}
    {
    }

    const ZoneItem& GetZoneItem() const { return mZoneItem; }
    const glm::vec3& GetRotation() const { return mRotation; }
    const glm::vec3& GetLocation() const { return mLocation; }
    const glm::uvec2& GetBakLocation() const { return mBakLocation; }
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
        Encounter::EncounterFactory ef,
        unsigned x,
        unsigned y,
        unsigned tileIndex)
    :
        mCenter{},
        mTile{x, y},
        mTileIndex{tileIndex},
        mItemInsts{},
        mEncounters{},
        mEmpty{}
    {
        LoadWorld(zoneItems, ef, x, y, tileIndex);
    }

    void LoadWorld(
        const ZoneItemStore& zoneItems,
        const Encounter::EncounterFactory ef,
        unsigned x,
        unsigned y,
        unsigned tileIndex)
    {
        const auto& logger = Logging::LogState::GetLogger("World");
        const auto tileWorld = zoneItems.GetZoneLabel().GetTileWorld(x, y);
        logger.Debug() << "Loading tile: " << tileWorld << std::endl;

        auto fb = FileBufferFactory::Get().CreateDataBuffer(tileWorld);
        const auto [tileWorldItems, tileCenter] = LoadWorldTile(fb);

        for (const auto& item : tileWorldItems)
        {
            if (item.mItemType == 0)
                mCenter = ToGlCoord<float>(item.mLocation);

            mItemInsts.emplace_back(
                zoneItems.GetZoneItem(item.mItemType),
                item);
        }

        const auto tileData = zoneItems.GetZoneLabel().GetTileData(x, y);
        if (FileBufferFactory::Get().DataBufferExists(tileData))
        {
            auto fb = FileBufferFactory::Get().CreateDataBuffer(tileData);
                
            mEncounters = Encounter::EncounterStore(
                ef,
                fb,
                mTile,
                mTileIndex);
        }
    }

    const auto& GetTile() const { return mTile; }
    const auto& GetItems() const { return mItemInsts; }
    const auto& GetEncounters(Chapter chapter) const
    {
        if (mEncounters)
            return mEncounters->GetEncounters(chapter);
        else
            return mEmpty;
    }
    auto GetCenter() const
    {
        return mCenter.value_or(
            GetItems().front().GetLocation());
    }

private:
    std::optional<glm::vec3> mCenter;
    glm::vec<2, unsigned> mTile;
    unsigned mTileIndex;

    std::vector<WorldItemInstance> mItemInsts;
    std::optional<Encounter::EncounterStore> mEncounters;
    std::vector<Encounter::Encounter> mEmpty;
};


class WorldTileStore
{
public:
    WorldTileStore(
        const ZoneItemStore& zoneItems,
        const Encounter::EncounterFactory& ef)
    :
        mWorlds{
            std::invoke([&zoneItems, &ef]()
            {
                const auto tiles = LoadZoneRef(
                    zoneItems.GetZoneLabel().GetZoneReference());

                std::vector<World> worlds{};
                worlds.reserve(tiles.size());

                for (unsigned tileIndex = 0; tileIndex < tiles.size(); tileIndex++)
                {
                    const auto& tile = tiles[tileIndex];
                    auto it = worlds.emplace_back(
                        zoneItems,
                        ef,
                        tile.x,
                        tile.y,
                        tileIndex);
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
