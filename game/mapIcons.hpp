#pragma once

#include "graphics/renderData.hpp"
#include "graphics/renderer.hpp"
#include "graphics/texture.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <utility>

namespace Game {

class MapIcons
{
public:
    void Init();

    const Graphics::RenderData& GetRenderData() const;
    std::pair<unsigned, unsigned> GetObject() const;
    glm::ivec2 GetDimensions() const;

private:
    std::unique_ptr<Graphics::RenderData> mMapIconRenderData;
    std::pair<unsigned, unsigned> mObject{0, 0};
    glm::ivec2 mDimensions{0, 0};
};

}
