#pragma once

#include "graphics/renderData.hpp"
#include "graphics/renderer.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class Camera;

namespace Gui { class Font; }

namespace Game {

class GlyphStore
{
public:
    struct GlyphObject
    {
        unsigned mOffset;
        unsigned mLength;
        float mGlyphAspect;
        float mAdvance;
    };

    void Init(const Gui::Font& font);

    const Graphics::RenderData& GetRenderData() const;
    const GlyphObject* GetGlyph(char c) const;

private:
    std::unique_ptr<Graphics::RenderData> mGlyphRenderData;
    std::unordered_map<char, GlyphObject> mGlyphObjects;
};

}
