#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/types.hpp"

#include <optional>

namespace BAK::Encounter {

struct Teleport
{
    Teleport(
        std::optional<ZoneNumber> targetZone,
        GamePositionAndHeading targetLocation,
        std::optional<HotspotRef> targetGDSScene)
    :
        mTargetZone{targetZone},
        mTargetLocation{targetLocation},
        mTargetGDSScene{targetGDSScene}
    {}

    std::optional<ZoneNumber> mTargetZone;
    GamePositionAndHeading mTargetLocation;
    std::optional<HotspotRef> mTargetGDSScene;
};

std::ostream& operator<<(std::ostream& os, const Teleport&);

class TeleportFactory
{
public:
    static constexpr auto sFilename = "TELEPORT.DAT";

    TeleportFactory();

    const Teleport& Get(unsigned i) const;

private:
    void Load();
    std::vector<Teleport> mTeleports;
};

}
