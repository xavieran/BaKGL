#pragma once

#include "bak/coordinates.hpp"

#include <glm/glm.hpp>

#include <cstdint>

namespace BAK {

glm::vec2 GetForwardVector(GameHeading heading);

GamePositionAndHeading MoveForward(
    const GamePositionAndHeading& location,
    float amount);

GameHeading RotateHeading(GameHeading heading, float angleDelta);
GameHeading RotateHeading(GameHeading heading, std::int16_t angleDelta);

struct SlideProjection {
    glm::vec3 mProjectedDelta;
    GamePosition mProjectedPosition;
};

SlideProjection ProjectSlide(
    const glm::vec3& delta,
    const glm::vec3& currentGLPos,
    GameHeading openHeading,
    float maxDistance);

CardinalDirection GetRotationDirection(GameHeading currentHeading, GameHeading targetHeading);

}
