#include "game/glyphStore.hpp"

#include "bak/constants.hpp"

#include "gui/fontManager.hpp"

#include "graphics/glm.hpp"
#include "graphics/meshObject.hpp"

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

        std::vector<glm::vec3> verts = {
            {-0.5, -0.5, 0}, {-0.5, 0.5, 0}, {0.5, 0.5, 0},
            {-0.5, -0.5, 0}, {0.5, 0.5, 0}, {0.5, -0.5, 0}};
        const std::vector<glm::vec3> norms(6, {0, 0, 1});
        const std::vector<glm::vec4> colors(6, {1, 1, 1, 1});
        const std::vector<glm::vec3> texcoords = {
            {0,    0,    layer},
            {0,    maxV, layer},
            {maxU, maxV, layer},
            {0,    0,    layer},
            {maxU, maxV, layer},
            {maxU, 0,    layer}};
        const std::vector<float> texblends(6, 1.0f);
        const std::vector<unsigned> indices = {0, 1, 2, 3, 4, 5};

        auto obj = objects.AddObject(
            std::string(1, c),
            {verts, norms, colors, texcoords, texblends, indices});

        float glyphAspect = static_cast<float>(tex.GetWidth())
            / static_cast<float>(tex.GetHeight());
        float adv = static_cast<float>(font.GetFont().GetWidth(c));

        mGlyphObjects[c] = {obj.first, obj.second, glyphAspect, adv};
    }

    mGlyphRenderData = std::make_unique<Graphics::RenderData>();
    mGlyphRenderData->LoadData(objects, characters.GetTextures(), maxDim);
}

}
