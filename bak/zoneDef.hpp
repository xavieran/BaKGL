#pragma once

#include "bak/types.hpp"

namespace BAK {

struct ZoneDefaults
{
    float mMinMapZoom;
    float mDefaultMapZoom;
    float mMaxMapZoom;
    float mMapZoomRate;
};

ZoneDefaults LoadZoneDefDat(ZoneNumber zone);

};
