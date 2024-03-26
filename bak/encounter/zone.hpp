#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"
#include "bak/dialogTarget.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace BAK::Encounter {

class Zone
{
public:
    ZoneNumber mTargetZone;
    GamePositionAndHeading mTargetLocation;
    KeyTarget mDialog;
};

std::ostream& operator<<(std::ostream& os, const Zone&);

class ZoneFactory
{
public:
    static constexpr auto sFilename = "DEF_ZONE.DAT";

    ZoneFactory();

    const Zone& Get(unsigned i) const;

private:
    void Load();
    std::vector<Zone> mZones;
};

}
