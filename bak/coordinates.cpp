#include "bak/coordinates.hpp"
#include "bak/constants.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const GamePositionAndHeading& pah)
{
    os << "{ pos: " << pah.mPosition << ", angle: " << pah.mHeading << "}";
    return os;
}

GamePosition MakeGamePositionFromTileAndOffset(
    glm::vec<2, unsigned> tile,
    glm::vec<2, std::uint8_t> offset)
{
    const auto Convert = [](auto t, auto o)
    {
        return t * BAK::gTileSize + gOffsetScale * o;
    };
    
    return GamePosition{
        Convert(tile.x, offset.x),
        Convert(tile.y, offset.y)};
}


// Convert a 16 bit BAK angle to radians
glm::vec3 ToGlAngle(const Vector3D& angle)
{
    return ToGlCoord<float>(angle) 
        / static_cast<float>(0xffff)
        * 2.0f * glm::pi<float>();
}

}
