#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace BAK {

class CampData
{
public:
    CampData();

    const std::vector<glm::vec2>& GetClockTicks() const;
    const std::vector<glm::vec2>& GetDaytimeShadow() const;
private:
    glm::vec2 mHighlightSize{};
    std::vector<glm::vec2> mClockTicks{};
    glm::vec2 mClockTwelve{};
    glm::vec2 mClockCenter{};
    std::vector<glm::vec2> mDaytimeShadow{};
};

}
