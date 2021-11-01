#pragma once

#include "bak/container.hpp"
#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <optional>
#include <ostream>
#include <variant>


namespace BAK {

class FixedObject
{
public:
    FixedObject(
        ContainerHeader header,
        Target dialogKey,
        std::optional<HotspotRef>,
        std::optional<glm::vec<2, unsigned>>);

    ContainerHeader mHeader;
    Target mDialogKey;
    std::optional<HotspotRef> mHotspotRef;
    std::optional<GamePosition> mEncounterPos;
};


std::ostream& operator<<(std::ostream& os, const FixedObject&);

std::vector<FixedObject>
LoadFixedObjects(unsigned targetZone);

}
