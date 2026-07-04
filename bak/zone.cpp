#include "bak/zone.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/fixedObject.hpp"
#include "bak/palette.hpp"
#include "bak/resourceNames.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/cube.hpp"
#include "graphics/meshObject.hpp"

#include "com/assert.hpp"

namespace BAK {

namespace {

constexpr auto sGridTexSize = 300;
constexpr auto sGridFadePixels = 6u;
constexpr auto sGridBorderThick = 4u;
constexpr auto sGridBorderTotal = 2 * sGridFadePixels + sGridBorderThick;

Graphics::MeshObject MakeGridQuadMesh(unsigned textureLayer)
{
    struct Corner { glm::vec3 pos; glm::vec3 uv; };
    // va: back-left, vb: back-right, vc: front-right, vd: front-left
    auto size = 0.55f;
    const Corner vd{{-size, 2.0f,  size}, {0.00f, 0.00f, static_cast<float>(textureLayer)}};
    const Corner va{{-size, 2.0f, -size}, {0.00f, 1.0f, static_cast<float>(textureLayer)}};
    const Corner vb{{ size, 2.0f, -size}, {1.00f, 1.00f, static_cast<float>(textureLayer)}};
    const Corner vc{{ size, 2.0f,  size}, {1.00f, 0.00f, static_cast<float>(textureLayer)}};

    constexpr auto normal = glm::vec3{0.0f, 1.0f, 0.0f};
    constexpr auto texBlend = 0.0f;

    const Corner verts[6] = {vd, va, vb, vd, vb, vc};
    auto vertices = std::vector<glm::vec3>{};
    auto normals = std::vector<glm::vec3>{};
    auto colors = std::vector<glm::vec4>{};
    auto texCoords = std::vector<glm::vec3>{};
    auto texBlends = std::vector<float>{};
    auto indices = std::vector<unsigned>{};

    for (unsigned i = 0; i < 6; i++)
    {
        vertices.emplace_back(verts[i].pos);
        normals.emplace_back(normal);
        colors.emplace_back(glm::vec4{1.0f});
        texCoords.emplace_back(verts[i].uv);
        texBlends.emplace_back(texBlend);
        indices.emplace_back(i);
    }

    return Graphics::MeshObject{vertices, normals, colors, texCoords, texBlends, indices};
}

}

// Contains all the data one would need for a zone
Zone::Zone(unsigned zoneNumber)
:
    mZoneLabel{zoneNumber},
    mPalette{mZoneLabel.GetPalette()},
    mFixedObjects{LoadFixedObjects(zoneNumber)},
    mZoneTextures{mZoneLabel},
    mZoneItems{mZoneLabel, mZoneTextures},
    mWorldTiles{mZoneItems, BAK::Encounter::EncounterFactory{}},
    mObjects{}
{
    for (auto& item : mZoneItems.GetItems())
    {
        mObjects.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, mZoneTextures, mPalette));
    }

    const auto cube = Graphics::Cuboid{1, 1, 50};
    mObjects.AddObject("Combat", cube.ToMeshObject(glm::vec4{1.0, 0, 0, .3}));
    mObjects.AddObject("Trap", cube.ToMeshObject(glm::vec4{.8, 0, 0, .3}));
    mObjects.AddObject("Dialog", cube.ToMeshObject(glm::vec4{0.0, 1, 0, .3}));
    mObjects.AddObject("Zone", cube.ToMeshObject(glm::vec4{1.0, 1, 0, .3}));
    mObjects.AddObject("GDSEntry", cube.ToMeshObject(glm::vec4{1.0, 0, 1, .3}));
    mObjects.AddObject("EventFlag", cube.ToMeshObject(glm::vec4{.0, .0, .7, .3}));
    mObjects.AddObject("Block", cube.ToMeshObject(glm::vec4{0,0,0, .3}));

    const auto click = Graphics::Cuboid{1, 1, 50};
    mObjects.AddObject("clickable", click.ToMeshObject(glm::vec4{1.0, 0, 0, .3}));

    // Grid visualization texture
    {
        auto gridTex = Graphics::Texture{sGridTexSize, sGridTexSize, sGridTexSize, sGridTexSize};
        for (unsigned y = 0; y < sGridTexSize; y++)
        {
            for (unsigned x = 0; x < sGridTexSize; x++)
            {
                const auto d = std::min(
                    std::min(x, sGridTexSize - 1 - x),
                    std::min(y, sGridTexSize - 1 - y));

                float alpha = 0.0f;
                if (d < sGridFadePixels)
                {
                    const float t = static_cast<float>(d) / sGridFadePixels;
                    alpha = t * t;
                }
                else if (d < sGridFadePixels + sGridBorderThick)
                {
                    alpha = 1.0f;
                }
                else if (d < sGridBorderTotal)
                {
                    const float t = static_cast<float>(sGridBorderTotal - d) / sGridFadePixels;
                    alpha = t * t;
                }

                gridTex.SetPixel(x, y, glm::vec4{1.0f, 1.0f, 1.0f, alpha + 0.03});
            }
        }
        const auto gridLayer = mZoneTextures.GetTextures().size();
        gridTex.SetRepeat(false);
        mZoneTextures.AddTexture(gridTex);
        mObjects.AddObject("GridCell", MakeGridQuadMesh(gridLayer));
    }
}

}
