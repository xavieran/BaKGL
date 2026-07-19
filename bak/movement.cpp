#include "bak/movement.hpp"

#include "bak/coordinates.hpp"

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

}
