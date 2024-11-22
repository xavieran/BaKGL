#pragma once

#include "bak/types.hpp"

#include "bak/encounter/teleport.hpp"

namespace BAK {

// Interface to load zone
class IZoneLoader
{
public:
    // Load zone based on zone info in DEF_ZONE.DAT
    virtual void DoTeleport(BAK::Encounter::Teleport) = 0;
    virtual void LoadGame(std::string, std::optional<Chapter>) = 0;
};

}
