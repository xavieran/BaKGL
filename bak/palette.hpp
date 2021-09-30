#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace BAK {

class Palette
{
public:
    Palette(const std::string& filename);

    const glm::vec4& GetColor(unsigned i) const;
private:
    std::vector<glm::vec4> mColors;
};

}
