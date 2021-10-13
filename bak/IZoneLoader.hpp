#pragma once

#include "bak/types.hpp"

namespace BAK {

// Interface to load zone
class IZoneLoader
{
public:
    // Just load zone data
    virtual void LoadZone(ZoneNumber) = 0;
    // Load zone based on zone info in DEF_ZONE.DAT
    virtual void LoadZone(ZoneTransitionIndex) = 0;
};

}
