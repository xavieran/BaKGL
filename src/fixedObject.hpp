#pragma once

#include "dialog.hpp"
#include "logger.hpp"

#include "xbak/Exception.h"
#include "xbak/FileBuffer.h"

#include "glm.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
