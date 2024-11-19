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
#include "bak/entityType.hpp"

#include <functional>   
#include <optional>

namespace BAK {

class ZoneTextureStore
{
public:

    ZoneTextureStore(
        const ZoneLabel& zoneLabel);

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
        const ZoneTextureStore& textureStore);

    ZoneItem(
        unsigned i,
        const BAK::MonsterNames& monsters,
        const ZoneTextureStore& textureStore);

    void SetPush(unsigned i);
    const std::string& GetName() const;
    bool IsSprite() const;
    float GetScale() const;
    bool GetClickable() const;
    EntityType GetEntityType() const;

    const auto& GetColors() const { return mColors; }
    const auto& GetFaces() const { return mFaces; }
    const auto& GetPush() const { return mPush; }
    const auto& GetPalettes() const { return mPalettes; }
    const auto& GetVertices() const { return mVertices; }
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
