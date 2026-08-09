#pragma once

#include "bak/types.hpp"
#include "bak/zoneDef.hpp"

namespace Game {

class ZoomManager
{
public:
    ZoomManager();

    void LoadZoneDefaults(BAK::ZoneNumber);

    glm::uvec2 ZoomIn(glm::uvec2 screenDims);
    glm::uvec2 ZoomOut(glm::uvec2 screenDims);
    glm::uvec2 DefaultZoom(glm::uvec2 screenDims);
    glm::uvec2 CalculateZoom(glm::vec2 screenDims);

private:
    BAK::ZoneDefaults mZoneDefaults{};
    int mCurrentZoomLevel{};
    int mSteps{};
};

}
