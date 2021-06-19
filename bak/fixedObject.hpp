#pragma once

#include "bak/dialog.hpp"

#include "com/logger.hpp"

#include <glm/glm.hpp>

namespace BAK {

class FixedObject
{
public:
    FixedObject(
        Target dialogKey,
        glm::vec<2, unsigned> location,
        std::uint8_t type)
    :
        mDialogKey{dialogKey},
        mLocation{location},
        mType{type}
    {}

    Target mDialogKey;
    glm::vec<2, unsigned> mLocation;
    std::uint8_t mType;
};

std::vector<FixedObject> LoadFixedObjects(unsigned targetZone);

}
