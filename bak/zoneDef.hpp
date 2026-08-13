#pragma once

#include "bak/types.hpp"

namespace BAK {

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

};
