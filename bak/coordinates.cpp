#include "bak/coordinates.hpp"
#include "bak/constants.hpp"

#include "com/assert.hpp"

#include "graphics/glm.hpp"

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

CardinalDirection HeadingToCardinalDirection(GameHeading heading)
{
    // BAK heading: 0 = North, 64 = West, 128 = South, 192 = East
    // (increases counterclockwise to match the game's convention)
    const auto clamped = (heading + 32) % 256;
    switch (clamped / 64)
    {
    case 0: return CardinalDirection::North;
    case 1: return CardinalDirection::West;
    case 2: return CardinalDirection::South;
    case 3: return CardinalDirection::East;
    }
    return {};
}

// The combat grid 0,0 point is placed at -1200, +3200 from the
// player location, when facing north. Combatants are placed in
// the centre of each grid square, but the bottom leftmost corner
// will be at -1200, +3200
//
// I render everything centered around its position, including grid squares,
// so we need to shift these by half a grid
static constexpr glm::vec2 gCombatGridOffset =
    glm::vec2{-1200, 3200} + (glm::vec2{gCombatGridSize} * 0.5f);

GamePosition CalculateCombatGridOrigin(
    const GamePositionAndHeading& pos)
{
    const int x = gCombatGridOffset.x;
    const int y = gCombatGridOffset.y;

    switch (HeadingToCardinalDirection(pos.mHeading))
    {
    case CardinalDirection::North:
        return glm::cast<int>(pos.mPosition) + glm::ivec2{x, y};
    case CardinalDirection::East:
        return glm::cast<int>(pos.mPosition) + glm::ivec2{y, -x};
    case CardinalDirection::South:
        return glm::cast<int>(pos.mPosition) + glm::ivec2{-x, -y};
    case CardinalDirection::West:
        return glm::cast<int>(pos.mPosition) + glm::ivec2{-y, x};
    }
    return {};
}

GamePosition MakeGamePositionFromGridCell(
    const GamePositionAndHeading& pos,
    glm::uvec2 gridPos)
{
    const auto dir = HeadingToCardinalDirection(pos.mHeading);
    const auto origin = CalculateCombatGridOrigin(pos);
    const auto col = static_cast<int>(gridPos.x);
    const auto row = static_cast<int>(gridPos.y);

    int gx, gy;
    switch (dir)
    {
    case CardinalDirection::North:
        gx = col * gCombatGridSize;
        gy = row * gCombatGridSize;
        break;
    case CardinalDirection::South:
        gx = -col * gCombatGridSize;
        gy = -row * gCombatGridSize;
        break;
    case CardinalDirection::East:
        gx = row * gCombatGridSize;
        gy = -col * gCombatGridSize;
        break;
    case CardinalDirection::West:
        gx = -row * gCombatGridSize;
        gy = col * gCombatGridSize;
        break;
    }

    return glm::floor(
        glm::cast<float>(origin) + glm::vec2{gx, gy});
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
}
