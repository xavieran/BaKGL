#include "game/zoomManager.hpp"

#include "com/logger.hpp"

#include "graphics/glm.hpp"

namespace Game {

ZoomManager::ZoomManager()
{}

void ZoomManager::LoadZoneDefaults(BAK::ZoneNumber zone)
{
    mZoneDefaults = BAK::LoadZoneDefDat(zone);
    mSteps = static_cast<int>(std::floor((mZoneDefaults.mMaxMapZoom - mZoneDefaults.mMinMapZoom) 
        / mZoneDefaults.mMapZoomRate));
    DefaultZoom({});
}

glm::uvec2 ZoomManager::ZoomIn(glm::uvec2 screenDims)
{
    mCurrentZoomLevel -= 1;
    if (mCurrentZoomLevel < 0)
    {
        mCurrentZoomLevel = 0;
    }

    return CalculateZoom(screenDims);
}

glm::uvec2 ZoomManager::ZoomOut(glm::uvec2 screenDims)
{
    mCurrentZoomLevel += 1;
    if (mCurrentZoomLevel >= mSteps)
    {
        mCurrentZoomLevel = mSteps - 1;
    }

    return CalculateZoom(screenDims);
}

glm::uvec2 ZoomManager::DefaultZoom(glm::uvec2 screenDims)
{
    mCurrentZoomLevel = std::floor((mZoneDefaults.mDefaultMapZoom - mZoneDefaults.mMinMapZoom)
        / mZoneDefaults.mMapZoomRate);
    return CalculateZoom(screenDims);
}

glm::uvec2 ZoomManager::CalculateZoom(glm::vec2 screenDims)
{
    auto normed = screenDims / mZoneDefaults.mMaxMapZoom;
    auto zoomed = (mZoneDefaults.mMinMapZoom + mCurrentZoomLevel * mZoneDefaults.mMapZoomRate) * normed;
    Logging::LogDebug(__FUNCTION__) << "Dims: " << screenDims << " step: " << mCurrentZoomLevel
        << " steps: " << mSteps << " normed: " << normed
        << " zoomed: " << zoomed << "\n";
    return zoomed;
}

bool ZoomManager::CanZoomIn() const
{
    return mCurrentZoomLevel > 0;
}

bool ZoomManager::CanZoomOut() const
{
    return mCurrentZoomLevel < (mSteps - 1);
}

}
