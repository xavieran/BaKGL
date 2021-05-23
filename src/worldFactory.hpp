#pragma once

#include "constants.hpp"
#include "coordinates.hpp"
#include "encounter.hpp"

#include "logger.hpp"
#include "resourceNames.hpp"
#include "texture.hpp"

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

#include <functional>   
#include <iomanip>   
#include <iostream>   
#include <optional>
#include <sstream>
#include <cassert>   

namespace BAK {

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
                    const auto& image = *spriteSlot.GetImage(j);

                    mTextures.push_back(ImageToTexture(image, palette));

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
            if (offset == 70)
                std::random_shuffle(image.begin(), image.end());

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
            return (lhs.GetHeight() < rhs.GetHeight());
            })->GetHeight();

        mMaxWidth = std::max_element(
			mTextures.begin(), mTextures.end(),
            [](const auto &lhs, const auto& rhs){
            return (lhs.GetWidth() < rhs.GetWidth());
            })->GetWidth();

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
        const DatInfo& datInfo,
        const TextureStore& textureStore)
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

std::ostream& operator<<(std::ostream& os, const ZoneItem& d)
{
    os << d.mName << " :: ";
        //<< d.GetDatItem().mVertices << "\n";
    for (const auto& face : d.GetFaces())
    {
        for (const auto i : face)
        {
            os << " :: " << i;
        }

        os << "\n";
    }

    return os;
}

Graphics::MeshObject ZoneItemToMeshObject(
    const ZoneItem& item,
    const TextureStore& store,
    const Palette& pal)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> textureCoords;
    std::vector<float> textureBlends;
    std::vector<unsigned> indices;

    auto glmVertices = std::vector<glm::vec3>{};

    const auto TextureBlend = [&](auto blend)
    {
        textureBlends.emplace_back(blend);
        textureBlends.emplace_back(blend);
        textureBlends.emplace_back(blend);
    };

    for (const auto& vertex : item.GetVertices())
    {
        glmVertices.emplace_back(
            glm::cast<float>(vertex) / BAK::gWorldScale);
    }

    unsigned index = 0;
    for (const auto& face : item.GetFaces())
    {
        if (face.size() < 3)
        {
            logger.Debug() << "Face with < 3 vertices: " << index
                << " - " << item.GetName() << std::endl;
            continue;
        }
        unsigned triangles = face.size() - 2;

        // Whether to push this face away from the main plane
        // (needed to avoid z-fighting for some objects)
        bool push = item.GetPush().at(index);
        
        // Tesselate the face
        // Generate normals and new indices for each face vertex
        // The normal must be inverted to account
        // for the Y direction being negated
        auto normal = glm::normalize(
            glm::cross(
                glmVertices[face[0]] - glmVertices[face[2]],
                glmVertices[face[0]] - glmVertices[face[1]]));

        for (unsigned triangle = 0; triangle < triangles; triangle++)
        {
            auto i_a = face[0];
            auto i_b = face[triangle + 1];
            auto i_c = face[triangle + 2];

            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);

            glm::vec3 zOff = normal;
            if (push) zOff = glm::vec3{0};
            
            vertices.emplace_back(glmVertices[i_a] - zOff * 0.02f);
            indices.emplace_back(vertices.size() - 1);
            vertices.emplace_back(glmVertices[i_b] - zOff * 0.02f);
            indices.emplace_back(vertices.size() - 1);
            vertices.emplace_back(glmVertices[i_c] - zOff * 0.02f);
            indices.emplace_back(vertices.size() - 1);
            
            // Hacky - only works for quads - but the game only
            // textures quads anyway... (not true...)
            auto colorIndex = item.GetColors().at(index);
            auto paletteIndex = item.GetPalettes().at(index);
            auto textureIndex = colorIndex;

            float u = 1.0;
            float v = 1.0;

            auto maxDim = store.GetMaxDim();
            
            // I feel like these "palettes" are probably collections of
            // flags?
            static constexpr std::uint8_t texturePalette0 = 0x90;
            static constexpr std::uint8_t texturePalette1 = 0x91;
            static constexpr std::uint8_t texturePalette2 = 0xd1;
            // texturePalette3 is optional and puts grass on mountains
            static constexpr std::uint8_t texturePalette3 = 0x81;
            static constexpr std::uint8_t texturePalette4 = 0x11;
            static constexpr std::uint8_t terrainPalette = 0xc1;

            // terrain palette
            // 0 = ground
            // 1 = road
            // 2 = waterfall
            // 3 = path
            // 4 = dirt/field
            // 5 = river
            // 6 = sand
            // 7 = riverbank
            if (item.GetName().substr(0, 2) == "t0")
            {
                if (textureIndex == 1)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Road);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 2)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Path);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 3)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(0.0);
                }
            }
            else if (item.GetName().substr(0, 2) == "r0")
            {
                if (textureIndex == 3)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 5)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(0.0);
                }
            }
            else if (item.GetName().substr(0, 2) == "g0")
            {
                if (textureIndex == 0)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Ground);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 5)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(0.0);
                }
            }
            else if (item.GetName().substr(0, 5) == "field")
            {
                if (textureIndex == 1)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Dirt);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 2)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(1.0);
                }
            }
            else if (item.GetName().substr(0, 4) == "fall"
                || item.GetName().substr(0, 6) == "spring")
            {
                if (textureIndex == 3)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 5)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                    TextureBlend(1.0);
                }
                else if (textureIndex == 6)
                {
                    textureIndex = store.GetTerrainOffset(BAK::Terrain::Waterfall);
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(0.0);
                }
            }
            else if (paletteIndex == terrainPalette)
            {
                textureIndex += store.GetTerrainOffset(BAK::Terrain::Ground);
                TextureBlend(1.0);
            }
            else if (paletteIndex == texturePalette0
                || paletteIndex == texturePalette1
                || paletteIndex == texturePalette2
                || paletteIndex == texturePalette4)
            {
                TextureBlend(1.0);
            }
            else
            {
                TextureBlend(0.0);
            }

            if (colorIndex < store.GetTextures().size())
            {
                u = static_cast<float>(store.GetTexture(textureIndex).GetWidth() - 1) 
                    / static_cast<float>(maxDim);
                v = static_cast<float>(store.GetTexture(textureIndex).GetHeight() - 1) 
                    / static_cast<float>(maxDim);
            }

            if (item.GetName().substr(0, 6) == "ground")
            {
                u *= 40;
                v *= 40;
            }

            if (triangle == 0)
            {
                textureCoords.emplace_back(u  , v,   textureIndex);
                textureCoords.emplace_back(0.0, v,   textureIndex);
                textureCoords.emplace_back(0.0, 0.0, textureIndex);
            }
            else
            {
                textureCoords.emplace_back(u,   v,   textureIndex);
                textureCoords.emplace_back(0.0, 0.0, textureIndex);
                textureCoords.emplace_back(u,   0.0, textureIndex);
            }

            auto color = pal.GetColor(colorIndex);
            auto glmCol = \
                glm::vec4(
                    static_cast<float>(color.r) / 256,
                    static_cast<float>(color.g) / 256,
                    static_cast<float>(color.b) / 256,
                    1);

            // bitta fun
            if (item.GetName().substr(0, 5) == "cryst")
                glmCol.a = 0.8;

            colors.emplace_back(glmCol);
            colors.emplace_back(glmCol);
            colors.emplace_back(glmCol);
        }

        index++;
    }

    return Graphics::MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices};
}

class ZoneItemStore
{
public:

    ZoneItemStore(
        const ZoneLabel& zoneLabel,
        // Should one really need a texture store to load this?
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

std::ostream& operator<<(std::ostream& os, const WorldItemInstance& d)
{
    os << "[ Name: " << d.GetZoneItem().GetName() << " Type: " << d.mType << " Rot: " 
        << glm::to_string(d.mRotation)
        << " Loc: " << glm::to_string(d.mLocation) << "]";
    os << std::endl << " Vertices::" << std::endl;

    const auto& vertices = d.GetZoneItem().GetVertices();
    for (const auto& vertex : vertices)
    {
        os << "  " << glm::to_string(vertex) << std::endl;
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
