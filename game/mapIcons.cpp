#include "game/mapIcons.hpp"

#include "bak/textureFactory.hpp"

#include "graphics/meshObject.hpp"
#include "graphics/quad.hpp"

#include <glm/glm.hpp>

#include <cassert>
#include <vector>

namespace Game {

const Graphics::RenderData& MapIcons::GetRenderData() const
{
    assert(mMapIconRenderData);
    return *mMapIconRenderData;
}

std::pair<unsigned, unsigned> MapIcons::GetObject() const
{
    return mObject;
}

glm::ivec2 MapIcons::GetDimensions() const
{
    return mDimensions;
}

void MapIcons::Init()
{
    Graphics::TextureStore textures;
    BAK::TextureFactory::AddToTextureStore(
        textures, "MAPICONS.BMX", "OPTIONS.PAL");

    const auto& tex = textures.GetTexture(0);
    const auto maxDim = textures.GetMaxDim();
    const float maxU = static_cast<float>(tex.GetWidth()) / maxDim;
    const float maxV = static_cast<float>(tex.GetHeight()) / maxDim;
    const float layer = 0.0f;

    Graphics::MeshObjectStorage objects;

    auto obj = objects.AddObject(
        "party_arrow",
        Graphics::Quad{
            {{{0.5f, 0, -0.5f}, {0.5f, 0, 0.5f}, {-0.5f, 0, 0.5f}, {-0.5f, 0, -0.5f}}},
            {{{maxU, maxV, layer}, {maxU, 0, layer}, {0, 0, layer}, {0, maxV, layer}}}
        }.ToMeshObject(1.0f));

    mObject = {obj.first, obj.second};
    mDimensions = tex.GetDims();

    mMapIconRenderData = std::make_unique<Graphics::RenderData>();
    mMapIconRenderData->LoadData(objects, textures.GetTextures(), maxDim);
}

}
