#pragma once

#include "bak/constants.hpp"
#include "bak/encounter/fwd.hpp"
#include "bak/encounter/encounterStore.hpp"
#include "bak/resourceNames.hpp"
#include "bak/entityType.hpp"
#include "bak/worldItem.hpp"

#include "graphics/texture.hpp"

#include <optional>

namespace Graphics {
class MeshObject;
}

namespace BAK {

class Palette;
class Model;
class MonsterNames;

class ZoneTextureStore
{
public:

    ZoneTextureStore(
        const ZoneLabel& zoneLabel);

    const Graphics::Texture& GetTexture(const unsigned i) const;
    const std::vector<Graphics::Texture>& GetTextures() const;

    unsigned GetMaxDim() const;
    unsigned GetTerrainOffset(BAK::Terrain t) const;
    unsigned GetHorizonOffset() const;

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
        const MonsterNames& monsters,
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
        const ZoneTextureStore& textureStore);

    const ZoneLabel& GetZoneLabel() const;

    const ZoneItem& GetZoneItem(const unsigned i) const;
    const ZoneItem& GetZoneItem(const std::string& name) const;

    const std::vector<ZoneItem>& GetItems() const;
    std::vector<ZoneItem>& GetItems();

private:
    const ZoneLabel mZoneLabel;
    std::vector<ZoneItem> mItems;
};

class WorldItemInstance
{
public:
    WorldItemInstance(
        const ZoneItem& zoneItem,
        const WorldItem& worldItem);

    const ZoneItem& GetZoneItem() const;
    const glm::vec3& GetRotation() const;
    const glm::vec3& GetLocation() const;
    const glm::uvec2& GetBakLocation() const;
    unsigned GetType() const;

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
        unsigned tileIndex);

    void LoadWorld(
        const ZoneItemStore& zoneItems,
        const Encounter::EncounterFactory ef,
        unsigned x,
        unsigned y,
        unsigned tileIndex);

    glm::vec<2, unsigned> GetTile() const;
    const std::vector<WorldItemInstance>& GetItems() const;
    const std::vector<Encounter::Encounter>& GetEncounters(Chapter chapter) const;
    glm::vec3 GetCenter() const;

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
        const Encounter::EncounterFactory& ef);

    const std::vector<World>& GetTiles() const;

private:
    std::vector<World> mWorlds;
};

}
