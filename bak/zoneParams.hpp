#pragma once

#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <cstdint>

namespace BAK {

struct ZoneViewport
{
    int mX;
    int mY;
    int mWidth;
    int mHeight;
};

ZoneViewport LoadZoneViewport();
glm::ivec4 ToGlViewport(ZoneViewport viewport, float scale);

struct ZoneDefaults
{
    float mMinMapZoom;
    float mDefaultMapZoom;
    float mMaxMapZoom;
    float mMapZoomRate;
    int mDefaultCameraHeight;
    int mFocalLengthScale;
};

ZoneDefaults LoadZoneDefDat(ZoneNumber zone);

struct CombatCamera
{
    float mHeight;
    float mHeightUnderground;
    float mViewAngleRadians;
    float mViewAngleRadiansUnderground;
    int mCombatGridCellSize;
    ZoneViewport mViewport;
    int mFocalLengthScale;
};

CombatCamera LoadCombatCamera();

std::uint8_t LoadCombatGridColour(ZoneNumber zone);

}
