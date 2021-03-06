#include "bak/coordinates.hpp"

namespace BAK {

// Convert a 16 bit BAK angle to radians
glm::vec3 ToGlAngle(const Vector3D& angle)
{
    return ToGlCoord<float>(angle) 
        / static_cast<float>(0xffff)
        * 2.0f * glm::pi<float>();
}

}
