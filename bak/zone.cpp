#include "bak/zone.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/fixedObject.hpp"
#include "bak/palette.hpp"
#include "bak/resourceNames.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/cube.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/quad.hpp"

#include "com/assert.hpp"

#include <cmath>

namespace BAK {

namespace {

constexpr auto sGridTexSize = 300;
constexpr auto sGridFadePixels = 6u;
constexpr auto sGridBorderThick = 4u;
constexpr auto sGridBorderTotal = 2 * sGridFadePixels + sGridBorderThick;

Graphics::MeshObject MakeGridQuadMesh(unsigned textureLayer)
{
    const auto size = 0.55f;
    const auto layer = static_cast<float>(textureLayer);
    return Graphics::Quad{
        {{{-size, 2.0f,  size}, {-size, 2.0f, -size}, { size, 2.0f, -size}, { size, 2.0f,  size}}},
        {{{0, 0, layer}, {0, 1, layer}, {1, 1, layer}, {1, 0, layer}}}
    }.ToMeshObject(0.0f);
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
    for (unsigned i = 0; i < mZoneItems.GetItems().size(); i++)
    {
        const auto& item = mZoneItems.GetItems()[i];
        mObjects.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, mZoneTextures, mPalette));

        if (item.GetModelClip())
        {
            mObjects.AddObject(
                BAK::GetClipName(item.GetName()),
                ClipToMeshObject(
                    *item.GetModelClip(),
                    BAK::GetDebugColor(item.GetEntityType())));
        }

        if (item.HasUndergroundModel())
        {
            mObjects.AddObject(
                BAK::GetUndergroundName(item.GetName()),
                BAK::ZoneItemToMeshObject(
                    item.GetUndergroundModel(), mZoneTextures, mPalette));
        }

        const auto frameCount = mZoneItems.GetModelFrameCount(item.GetName());
        if (frameCount)
        {
            const auto& model = mZoneItems.GetModel(i);
            const auto& clip = mZoneItems.GetClip(i);
            for (unsigned frame = 1; frame < *frameCount; frame++)
            {
                BAK::ZoneItem frameItem(model, clip, mZoneTextures, frame);
                mObjects.AddObject(
                    item.GetName() + "_f" + std::to_string(frame),
                    BAK::ZoneItemToMeshObject(frameItem, mZoneTextures, mPalette));
            }
        }
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

bool IsUnderground(ZoneNumber zone)
{
    return zone.mValue == 10 || zone.mValue == 11 || zone.mValue == 12;
}

}
