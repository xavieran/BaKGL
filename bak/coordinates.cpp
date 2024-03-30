#include "bak/coordinates.hpp"
#include "bak/constants.hpp"

#include "com/assert.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const GamePositionAndHeading& pah)
{
    os << "{ pos: " << pah.mPosition << ", angle: " << pah.mHeading << "}";
    return os;
}

glm::uvec2 GetTile(
    glm::uvec2 pos)
{
    return glm::floor(glm::cast<float>(pos)
        / glm::vec2{BAK::gTileSize});
}

GamePosition MakeGamePositionFromTileAndCell(
    glm::uvec2 tile,
    glm::vec<2, std::uint8_t> cell)
{
    const auto Convert = [](auto t, auto c)
    {
        return t * BAK::gTileSize + gCellSize * c + (gCellSize >> 1);
    };
    
    return GamePosition{
        Convert(tile.x, cell.x),
        Convert(tile.y, cell.y)};
}

// Convert a 16 bit BAK angle to radians
glm::vec3 ToGlAngle(const glm::ivec3& angle)
{
    return ToGlCoord<float>(angle) 
        / static_cast<float>(0xffff)
        * 2.0f * glm::pi<float>();
}

BAK::GameHeading ToBakAngle(double angle)
{
    const auto multiplier = static_cast<double>(0xff);
    const auto angleGreaterThanZero = angle + glm::pi<double>();
    const auto bakAngle = multiplier * (angleGreaterThanZero / (2 * glm::pi<double>()));
    ASSERT(bakAngle > 0);
    return static_cast<BAK::GameHeading>(bakAngle);
}

double NormaliseRadians(double angle)
{
    if (angle > glm::pi<double>())
        return angle - (glm::pi<double>() * 2);
    else if (angle < -glm::pi<double>())
        return angle + (glm::pi<double>() * 2);
    return angle;
}

// Convert an 8 bit BAK angle to radians
glm::vec2 ToGlAngle(GameHeading heading)
{
    constexpr auto divider = static_cast<float>(0xff);
    const auto ratio = static_cast<float>(heading) / divider;
    const auto angle = ratio * 2.0f * glm::pi<float>();
    return glm::vec2{NormaliseRadians(angle + glm::pi<float>()), 0};
}

std::ostream& operator<<(std::ostream& os, const MapLocation& l)
{
    os << "MapLocation{ pos: " << l.mPosition
        << " Heading: " << l.mHeading << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Location& l)
{
    os << "Location{ zone: " << l.mZone << " tile: " << l.mTile
        << " Pos: " << l.mLocation << "}";
    return os;
}

std::uint16_t HeadingToFullMapAngle(std::uint16_t heading)
{
    constexpr auto unit = 0xff / 8;
    return 4 * ((heading/ unit) % 8);
}
}
