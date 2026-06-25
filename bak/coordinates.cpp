#include "bak/coordinates.hpp"
#include "bak/constants.hpp"

#include "com/assert.hpp"

#include "graphics/glm.hpp"

#include <cmath>
#include <utility>

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

glm::uvec2 GetTileSpaceOffset(GamePosition pos)
{
    auto tile = GetTile(pos);
    return glm::floor(
        glm::cast<float>(pos) - (glm::cast<float>(tile) * gTileSize));
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

GameHeading SnapHeading(GameHeading heading, GameHeading snapAmount)
{
    const auto h = static_cast<std::uint8_t>(heading);
    const auto snapped = static_cast<std::uint8_t>(
        (h + snapAmount / 2) / snapAmount * snapAmount);
    return snapped;
}

static glm::ivec2 RotateByBAKHeading(
    glm::ivec2 vec,
    GameHeading snappedHeading)
{
    constexpr double fullCircle = 256.0;
    double angle = static_cast<double>(snappedHeading)
        * 2.0 * glm::pi<double>() / fullCircle;
    double c = std::cos(angle);
    double s = std::sin(angle);
    return glm::ivec2{
        static_cast<int>(std::round(vec.x * c - vec.y * s)),
        static_cast<int>(std::round(vec.x * s + vec.y * c))};
}

// The combat grid 0,0 point is placed at -1200, +3200 from the
// player location, when facing north. Combatants are placed in
// the centre of each grid square, but the bottom leftmost corner
// will be at -1200, +3200
//
// I render everything centered around its position, including grid squares,
// so we need to shift these by half a grid
static constexpr glm::vec2 gCombatGridOffset =
    glm::vec2{-1200, 3200} + (glm::vec2{gCombatGridCellSize} * 0.5f);

GamePosition CalculateCombatGridOrigin(
    const GamePositionAndHeading& pos)
{
    auto snapped = SnapHeading(pos.mHeading);
    auto offset = RotateByBAKHeading(
        glm::ivec2{gCombatGridOffset}, snapped);
    return glm::cast<int>(pos.mPosition) + offset;
}

GamePosition MakeGamePositionFromGridCell(
    const GamePositionAndHeading& pos,
    glm::uvec2 gridPos)
{
    auto snapped = SnapHeading(pos.mHeading);
    auto origin = CalculateCombatGridOrigin(pos);
    auto gridOffset = RotateByBAKHeading(
        glm::ivec2{
            static_cast<int>(gridPos.x),
            static_cast<int>(gridPos.y)} * static_cast<int>(gCombatGridCellSize),
        snapped);

    return glm::floor(
        glm::cast<float>(origin) + glm::cast<float>(gridOffset));
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
    return 4 * ((heading / unit) % 8);
}

std::string_view ToString(Direction direction)
{
    using enum Direction;
    switch (direction)
    {
        case South: return "South";
        case SouthEast: return "SouthEast";
        case East: return "East";
        case NorthEast: return "NorthEast";
        case North: return "North";
        case NorthWest: return "NorthWest";
        case West: return "West";
        case SouthWest: return "SouthWest";
    }
    std::unreachable();
}

Direction GetDirectionBetween(GamePosition source, GamePosition dest)
{
    const auto delta = glm::ivec2(dest) - glm::ivec2(source);
    if (delta.x == 0 && delta.y == 0)
        return Direction::South;

    auto radians = std::atan2(
        static_cast<double>(delta.y),
        static_cast<double>(delta.x));

    auto heading = ToBakAngle(radians + glm::pi<float>() / 2.0);
    return static_cast<BAK::Direction>(
        ((heading + 144u) % 256u) / 32u);
}

bool IsCardinal(Direction direction)
{
    using enum Direction;
    switch (direction)
    {
        case South: [[fallthrough]];
        case East: [[fallthrough]]; 
        case North: [[fallthrough]];
        case West: return true;
        default: return false;
    }
    std::unreachable();
}

Direction NextAnticlockwise(Direction direction, unsigned steps)
{
    return static_cast<Direction>(
        (static_cast<unsigned>(direction) + steps) % 8);
}

Direction NextClockwise(Direction direction, unsigned steps)
{
    return static_cast<Direction>(
        (static_cast<unsigned>(direction) + 8 - (steps % 8)) % 8);
}

glm::ivec2 ToDelta(Direction direction)
{
    using enum Direction;
    switch (direction)
    {
        case South: return {0, -1};
        case SouthEast: return {1, -1};
        case East: return {1, 0};
        case NorthEast: return {1, 1};
        case North: return {0, 1};
        case NorthWest: return {-1, 1};
        case West: return {-1, 0};
        case SouthWest: return {-1, -1};
    }
    std::unreachable();
}

}
