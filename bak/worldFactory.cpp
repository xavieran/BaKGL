#include "bak/worldFactory.hpp"

#include "bak/textureFactory.hpp"

#include "com/string.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/ImageResource.h"

namespace BAK {

ZoneTextureStore::ZoneTextureStore(
    const ZoneLabel& zoneLabel,
    const BAK::Palette& palette)
:
    mTextures{},
    mTerrainOffset{0},
    mHorizonOffset{0}
{
    bool found = true;
    unsigned spriteSlot = 0;

    while ( found )
    {
        auto spriteSlotLbl = zoneLabel.GetSpriteSlot(spriteSlot++);
        try
        {
            auto fb = FileBufferFactory::Get().CreateDataBuffer(spriteSlotLbl);
            ImageResource sprites{};
            sprites.Load(&fb);
            TextureFactory::AddToTextureStore(
                mTextures,
                sprites,
                palette);
        }
        catch (const std::exception&)
        {
            found = false;
        }
    }

    mTerrainOffset = GetTextures().size();

    ScreenResource terrain{};
    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetTerrain());
    terrain.Load(&fb);

    TextureFactory::AddTerrainToTextureStore(
        mTextures,
        terrain,
        palette);

    mHorizonOffset = GetTextures().size();

    const auto monsters = MonsterNames{};
    for (unsigned i = 0; i < monsters.size(); i ++)
    {
        auto prefix = monsters.GetMonsterAnimationFile(MonsterIndex{i});
        if (prefix == "")
            prefix = "ogr";
        prefix = ToUpper(prefix);
        prefix += "1.BMX";

        auto images = ImageResource{};
        auto fb = FileBufferFactory::Get().CreateDataBuffer(prefix);
        images.Load(&fb);

        auto pal = Palette{zoneLabel.GetPalette()};
        const auto colorSwap = monsters.GetColorSwap(MonsterIndex{i});
        if (colorSwap <= 9)
        {
            auto ss = std::stringstream{};
            ss << "CS";
            ss << +colorSwap << ".DAT";
            const auto cs = ColorSwap{ss.str()};
            pal = Palette{pal, cs};
        }

        TextureFactory::AddToTextureStore(
            mTextures,
            images,
            pal,
            0);
    }
}

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

    return Graphics::MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices};
}

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

}
