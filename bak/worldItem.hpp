#pragma once

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"

#include "bak/fileBuffer.hpp"

#include <vector>

namespace BAK {

struct WorldItem
{
    unsigned mItemType;

    glm::ivec3 mRotation;
    glm::ivec3 mLocation;
};

auto LoadWorldTile(FileBuffer& fb) 
    -> std::pair<std::vector<WorldItem>, glm::ivec2>;

}
