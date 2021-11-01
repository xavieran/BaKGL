#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <optional>


namespace BAK {

class FixedObject
{
public:
    FixedObject(
        Target dialogKey,
        glm::vec<2, unsigned> location,
        std::uint8_t type,
        std::optional<HotspotRef>,
        std::optional<glm::vec<2, unsigned>>);

    Target mDialogKey;
    glm::vec<2, unsigned> mLocation;
    std::uint8_t mType;
    std::optional<HotspotRef> mHotspotRef;
    std::optional<GamePosition> mEncounterPos;
};


std::ostream& operator<<(std::ostream& os, const FixedObject&);

std::vector<FixedObject>
LoadFixedObjects(unsigned targetZone);

}
