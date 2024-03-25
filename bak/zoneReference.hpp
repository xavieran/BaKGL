#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace BAK {

std::vector<glm::uvec2> LoadZoneRef(
    const std::string& path);

}
