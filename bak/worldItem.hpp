#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace BAK {

class FileBuffer;

struct WorldItem
{
    unsigned mItemType;

    glm::ivec3 mRotation;
    glm::ivec3 mLocation;
};

auto LoadWorldTile(FileBuffer& fb) 
    -> std::pair<std::vector<WorldItem>, glm::ivec2>;

}
