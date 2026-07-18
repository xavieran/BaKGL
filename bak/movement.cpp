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

}
