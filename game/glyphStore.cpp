#include "game/glyphStore.hpp"

#include "bak/constants.hpp"

#include "gui/fontManager.hpp"

#include "graphics/glm.hpp"
#include "graphics/meshObject.hpp"
#include "graphics/quad.hpp"

#include <glm/glm.hpp>

#include <cassert>

namespace Game {

const Graphics::RenderData& GlyphStore::GetRenderData() const
{
    assert(mGlyphRenderData);
    return *mGlyphRenderData;
}

const GlyphStore::GlyphObject* GlyphStore::GetGlyph(char c) const
{
    auto it = mGlyphObjects.find(c);
    if (it != mGlyphObjects.end())
    {
        return &it->second;
    }
    return nullptr;
}

void GlyphStore::Init(const Gui::Font& font)
{
    const auto& characters = font.GetFont().GetCharacters();
    const auto& textures = characters.GetTextures();
    const auto maxDim = characters.GetMaxDim();

    Graphics::MeshObjectStorage objects;

    for (unsigned i = 0; i < textures.size(); i++)
    {
        const auto& tex = textures[i];
        float maxU = static_cast<float>(tex.GetWidth())  / maxDim;
        float maxV = static_cast<float>(tex.GetHeight()) / maxDim;
        float layer = static_cast<float>(i);
        char c = font.GetFont().GetFirstChar() + i;

        auto obj = objects.AddObject(
            std::string(1, c),
            Graphics::Quad{
                {{{-0.5f, -0.5f, 0}, {-0.5f, 0.5f, 0}, {0.5f, 0.5f, 0}, {0.5f, -0.5f, 0}}},
                {{{0, 0, layer}, {0, maxV, layer}, {maxU, maxV, layer}, {maxU, 0, layer}}}
            }.ToMeshObject(1.0f));

        float glyphAspect = static_cast<float>(tex.GetWidth())
            / static_cast<float>(tex.GetHeight());
        float adv = static_cast<float>(font.GetFont().GetWidth(c));

        mGlyphObjects[c] = {obj.first, obj.second, glyphAspect, adv};
    }

    mGlyphRenderData = std::make_unique<Graphics::RenderData>();
    mGlyphRenderData->LoadData(objects, characters.GetTextures(), maxDim);
}

}
