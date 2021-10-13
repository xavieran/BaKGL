#pragma once

#include "bak/types.hpp"

namespace BAK {

// Interface to load zone
class IZoneLoader
{
public:
    // Load zone based on zone info in DEF_ZONE.DAT
    virtual void DoTeleport(TeleportIndex) = 0;
};

}
