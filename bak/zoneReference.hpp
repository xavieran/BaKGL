#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace BAK {

std::vector<glm::vec<2, unsigned>> LoadZoneRef(
    const std::string& path);


}
