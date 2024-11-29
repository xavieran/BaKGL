#include "bak/worldFactory.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/image.hpp"
#include "bak/imageStore.hpp"
#include "bak/model.hpp"
#include "bak/palette.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"
#include "bak/zoneReference.hpp"

#include "com/string.hpp"

#include "graphics/meshObject.hpp"

#include <functional>   
#include <vector>

namespace BAK {

ZoneTextureStore::ZoneTextureStore(
    const ZoneLabel& zoneLabel)
:
    mTextures{},
    mTerrainOffset{0},
    mHorizonOffset{0}
{
    bool found = true;
    unsigned spriteSlot = 0;

    while (found)
    {
        auto spriteSlotLbl = zoneLabel.GetSpriteSlot(spriteSlot++);
        if ((found = FileBufferFactory::Get().DataBufferExists(spriteSlotLbl)))
        {
            TextureFactory::AddToTextureStore(
                mTextures, spriteSlotLbl, zoneLabel.GetPalette());
        }
    }

    mTerrainOffset = GetTextures().size();

    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetTerrain());
    const auto terrain = LoadScreenResource(fb);
    auto pal = Palette{zoneLabel.GetPalette()};
    TextureFactory::AddTerrainToTextureStore(
        mTextures,
        terrain,
        pal);

    mHorizonOffset = GetTextures().size();
}

const Graphics::Texture& ZoneTextureStore::GetTexture(const unsigned i) const
{
    return mTextures.GetTexture(i);
}

const std::vector<Graphics::Texture>& ZoneTextureStore::GetTextures() const { return mTextures.GetTextures(); }

unsigned ZoneTextureStore::GetMaxDim() const { return mTextures.GetMaxDim(); }
unsigned ZoneTextureStore::GetTerrainOffset(BAK::Terrain t) const
{
    return mTerrainOffset + static_cast<unsigned>(t);
}
unsigned ZoneTextureStore::GetHorizonOffset() const { return mHorizonOffset; }



ZoneItem::ZoneItem(
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
    // True 3D model
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
    // Billboarded sprite
    else
    {
        // Need this to set the right dimensions for the texture
        const auto& tex = textureStore.GetTexture(mSpriteIndex);
        const auto spriteScale = 7.0f;
        auto width  = static_cast<int>(static_cast<float>(tex.GetTargetWidth()) * (spriteScale * .75));
        auto height = tex.GetTargetHeight() * spriteScale;
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

ZoneItem::ZoneItem(
    unsigned spriteIndex,
    const Graphics::Texture& texture)
:
    mName{""},
    mEntityFlags{0},
    mEntityType{},
    mScale{1},
    mSpriteIndex{spriteIndex},
    mColors{},
    mVertices{},
    mPalettes{},
    mFaces{},
    mPush{}
{
    // Need this to set the right dimensions for the texture
    const auto spriteScale = 7.0f;
    auto width  = static_cast<int>(static_cast<float>(texture.GetTargetWidth()) * (spriteScale * .75));
    auto height = texture.GetTargetHeight() * spriteScale;
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
    mColors.emplace_back(spriteIndex);

    ASSERT((mFaces.size() == mColors.size())
        && (mFaces.size() == mPalettes.size())
        && (mFaces.size() == mPush.size()));
}

void ZoneItem::SetPush(unsigned i){ mPush[i] = true; }
const std::string& ZoneItem::GetName() const { return mName; }
bool ZoneItem::IsSprite() const { return mSpriteIndex > 0 && mSpriteIndex < 400; }
float ZoneItem::GetScale() const { return mScale; }
bool ZoneItem::GetClickable() const
{
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

EntityType ZoneItem::GetEntityType() const { return mEntityType; }

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
    const ZoneTextureStore& store,
    const BAK::Palette& pal)
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
        if (face.size() < 3) // line
        {
            auto start = glmVertices[face[0]];
            auto end = glmVertices[face[1]];
            auto normal = glm::normalize(
                glm::cross(end - start, glm::vec3(0, 0, 1.0)));
            float linewidth = 0.05f;
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(end + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start - linewidth * normal);

            indices.emplace_back(vertices.size());
            vertices.emplace_back(end + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start - linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(end - linewidth * normal);

            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);

            auto colorIndex = item.GetColors().at(index);
            auto paletteIndex = item.GetPalettes().at(index);
            auto textureIndex = colorIndex;

            auto color = pal.GetColor(colorIndex);
            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);

            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);

            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);

            TextureBlend(0.0);
            TextureBlend(0.0);

            index++;
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
        if (item.IsSprite())
        {
            normal = glm::cross(normal, glm::vec3{1, 0, 1});
        }

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

            // controls the size of the "pixel" effect of the ground
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

            // bitta fun
            if (item.GetName().substr(0, 5) == "cryst")
                color.a = 0.8;

            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);
        }

        index++;
    }

    assert(vertices.size() == normals.size());
    assert(vertices.size() == colors.size());
    assert(vertices.size() == textureCoords.size());
    assert(vertices.size() == textureBlends.size());
    assert(vertices.size() == indices.size());
    return Graphics::MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices};
}

Graphics::MeshObject ZoneItemToMeshObject(
    const ZoneItem& item,
    const Graphics::TextureStore& store)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> textureCoords;
    std::vector<float> textureBlends;
    std::vector<unsigned> indices;

    auto glmVertices = std::vector<glm::vec3>{};
    if (item.GetName().substr(0, 5) == "tston")
    {
        logger.Info() << "Tombstone\n";
    }

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
        if (face.size() < 3) // line
        {
            auto start = glmVertices[face[0]];
            auto end = glmVertices[face[1]];
            auto normal = glm::normalize(
                glm::cross(end - start, glm::vec3(0, 0, 1.0)));
            float linewidth = 0.05f;
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(end + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start - linewidth * normal);

            indices.emplace_back(vertices.size());
            vertices.emplace_back(end + linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(start - linewidth * normal);
            indices.emplace_back(vertices.size());
            vertices.emplace_back(end - linewidth * normal);

            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);
            normals.emplace_back(normal);

            auto colorIndex = item.GetColors().at(index);
            auto paletteIndex = item.GetPalettes().at(index);
            auto textureIndex = colorIndex;

            auto color = glm::vec4{0};
            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);

            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);

            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);
            textureCoords.emplace_back(0.0, 0.0, textureIndex);

            TextureBlend(0.0);
            TextureBlend(0.0);

            index++;
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
        if (item.IsSprite())
        {
            normal = glm::cross(normal, glm::vec3{1, 0, 1});
        }

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

            TextureBlend(1.0);

            auto maxDim = store.GetMaxDim();
            u = static_cast<float>(store.GetTexture(textureIndex).GetWidth() - 1) 
                / static_cast<float>(maxDim);
            v = static_cast<float>(store.GetTexture(textureIndex).GetHeight() - 1) 
                / static_cast<float>(maxDim);

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

            auto color = glm::vec4{0};

            colors.emplace_back(color);
            colors.emplace_back(color);
            colors.emplace_back(color);
        }

        index++;
    }

    assert(vertices.size() == normals.size());
    assert(vertices.size() == colors.size());
    assert(vertices.size() == textureCoords.size());
    assert(vertices.size() == textureBlends.size());
    assert(vertices.size() == indices.size());
    return Graphics::MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices};
}
ZoneItemStore::ZoneItemStore(
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

const ZoneLabel& ZoneItemStore::GetZoneLabel() const { return mZoneLabel; }

const ZoneItem& ZoneItemStore::GetZoneItem(const unsigned i) const
{
    ASSERT(i < mItems.size());
    return mItems[i];
}

const ZoneItem& ZoneItemStore::GetZoneItem(const std::string& name) const
{
    auto it = std::find_if(mItems.begin(), mItems.end(),
        [&name](const auto& item){
        return name == item.GetName();
        });

    ASSERT(it != mItems.end());
    return *it;
}

const std::vector<ZoneItem>& ZoneItemStore::GetItems() const { return mItems; }
std::vector<ZoneItem>& ZoneItemStore::GetItems() { return mItems; }

WorldItemInstance::WorldItemInstance(
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

const ZoneItem& WorldItemInstance::GetZoneItem() const { return mZoneItem; }
const glm::vec3& WorldItemInstance::GetRotation() const { return mRotation; }
const glm::vec3& WorldItemInstance::GetLocation() const { return mLocation; }
const glm::uvec2& WorldItemInstance::GetBakLocation() const { return mBakLocation; }
unsigned WorldItemInstance::GetType() const { return mType; }

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

World::World(
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

void World::LoadWorld(
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

glm::vec<2, unsigned> World::GetTile() const { return mTile; }
unsigned World::GetTileIndex() const { return mTileIndex; }
const std::vector<WorldItemInstance>& World::GetItems() const { return mItemInsts; }
const std::vector<Encounter::Encounter>& World::GetEncounters(Chapter chapter) const
{
    if (mEncounters)
        return mEncounters->GetEncounters(chapter);
    else
        return mEmpty;
}

glm::vec3 World::GetCenter() const
{
    return mCenter.value_or(
        GetItems().front().GetLocation());
}

WorldTileStore::WorldTileStore(
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

const std::vector<World>& WorldTileStore::GetTiles() const
{
    return mWorlds;
}

}

