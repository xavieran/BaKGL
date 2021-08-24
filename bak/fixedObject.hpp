#pragma once

#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"

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
        std::optional<HotspotRef> hotspotRef);

    Target mDialogKey;
    glm::vec<2, unsigned> mLocation;
    std::uint8_t mType;
    std::optional<HotspotRef> mHotspotRef;
};

std::vector<FixedObject>
LoadFixedObjects(unsigned targetZone);

}
