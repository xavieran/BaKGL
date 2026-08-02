#include "bak/movement.hpp"

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"

#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <algorithm>
#include <cmath>

namespace BAK {

glm::vec2 GetForwardVector(GameHeading heading)
{
    const auto rad = ToGlAngle(heading).x;
    return {std::sin(rad), -std::cos(rad)};
}

GamePositionAndHeading MoveForward(
    const GamePositionAndHeading& location,
    float amount)
{
    const auto forward = GetForwardVector(location.mHeading);
    const auto pos = glm::vec2{location.mPosition} + forward * amount;
    return GamePositionAndHeading{
        glm::uvec2{glm::ivec2{pos}},
        location.mHeading};
}

GameHeading RotateHeading(GameHeading heading, float angleDelta)
{
    const auto rad = ToGlAngle(heading).x;
    return ToBakAngle(rad + angleDelta);
}

GameHeading RotateHeading(GameHeading heading, std::int16_t angleDelta)
{
    const int result = (static_cast<int>(heading) + angleDelta) % 256;
    return static_cast<GameHeading>((result + 256) % 256);
}

CardinalDirection GetRotationDirection(GameHeading currentHeading, GameHeading targetHeading)
{
    const auto diff = (static_cast<unsigned>(targetHeading)
        - static_cast<unsigned>(currentHeading)) % gBakHeadingFullCircle;
    if (diff > 0 && diff < gBakHeadingHalfCircle)
        return CardinalDirection::West;  // rotate left (counter-clockwise)
    else
        return CardinalDirection::East;  // rotate right (clockwise)
}

SlideProjection ProjectSlide(
    const glm::vec3& delta,
    const glm::vec3& currentGLPos,
    GameHeading openHeading,
    float maxDistance)
{
    const auto fwd = GetForwardVector(openHeading);
    const auto fwdGL = glm::vec3{fwd.x, 0, -fwd.y};
    const auto horizontalDelta = glm::vec3{delta.x, 0, delta.z};
    auto projected = glm::dot(horizontalDelta, fwdGL) * fwdGL;
    projected.y = delta.y;

    const auto projectedLength = glm::length(glm::vec3{projected.x, 0, projected.z});
    if (projectedLength > maxDistance)
        projected = projected * (maxDistance / projectedLength);

    const auto projectedPos = currentGLPos + projected;
    return SlideProjection{
        projected,
        GamePosition{glm::uvec2{projectedPos.x, -projectedPos.z}}};
}

GamePosition SnapPositionToCellCenter(GamePosition pos)
{
    const auto snapAxis = [](unsigned value) {
        const auto cell = std::max<int>(
            0,
            std::lround((static_cast<double>(value) - gHalfCellSize) / gCellSize));
        return static_cast<unsigned>(cell) * gCellSize + gHalfCellSize;
    };

    return GamePosition{
        glm::uvec2{
            snapAxis(pos.x),
            snapAxis(pos.y)}};
}

GamePosition SnapToCellVector(GamePosition pos, Direction direction)
{
    auto x = static_cast<int>(pos.x);
    auto y = static_cast<int>(pos.y);
    switch (direction)
    {
    case Direction::East: [[fallthrough]];
    case Direction::West:
        y = (y / static_cast<int>(gCellSize)) * static_cast<int>(gCellSize)
            + static_cast<int>(gHalfCellSize);
        break;
    case Direction::North: [[fallthrough]];
    case Direction::South:
        x = (x / static_cast<int>(gCellSize)) * static_cast<int>(gCellSize)
            + static_cast<int>(gHalfCellSize);
        break;
    case Direction::SouthEast: [[fallthrough]];
    case Direction::NorthWest:
    {
        const auto remainder = (x + y) % static_cast<int>(gCellSize);
        const auto adjusted = remainder > static_cast<int>(gHalfCellSize)
            ? remainder - static_cast<int>(gCellSize)
            : remainder;
        y -= adjusted;
        break;
    }
    case Direction::NorthEast: [[fallthrough]];
    case Direction::SouthWest:
    {
        const auto remainder = (x - y) % static_cast<int>(gCellSize);
        const auto adjusted = remainder > static_cast<int>(gHalfCellSize)
            ? remainder - static_cast<int>(gCellSize)
            : (remainder < -static_cast<int>(gHalfCellSize)
                ? remainder + static_cast<int>(gCellSize)
                : remainder);
        y += adjusted;
        break;
    }
    }
    return GamePosition{glm::uvec2{x, y}};
}

GamePosition FindNearestRoadCell(
    GamePosition pos,
    std::function<bool(GamePosition)> isOnRoad)
{
    constexpr glm::vec2 sCellOffsets[] = {
        { 0,  0}, { 1,  0}, {-1,  0},
        { 0,  1}, { 0, -1}, { 1,  1},
        {-1,  1}, { 1, -1}, {-1, -1}};

    for (const auto& offset : sCellOffsets)
    {
        const auto candidate = SnapPositionToCellCenter(
            pos + glm::uvec2{offset * static_cast<float>(gCellSize)});
        if (isOnRoad(candidate))
        {
            return candidate;
        }
    }
    return pos;
}

std::optional<GameHeading> SnapHeadingIfOvershot(
    GameHeading oldHeading,
    GameHeading newHeading,
    GameHeading targetHeading)
{
    const auto forwardDist =
        (newHeading - oldHeading + gBakHeadingFullCircle) % gBakHeadingFullCircle;

    if (forwardDist == 0 || forwardDist >= gBakHeadingHalfCircle)
        return std::nullopt;

    const auto distToTarget =
        (targetHeading - oldHeading + gBakHeadingFullCircle) % gBakHeadingFullCircle;

    if (distToTarget > 0 && distToTarget <= forwardDist)
        return targetHeading;

    return std::nullopt;
}

GamePosition GetCellPosition(GamePosition pos)
{
    return pos % static_cast<unsigned>(BAK::gCellSize);
}
}
