#pragma once

#include "bak/coordinates.hpp"

#include <glm/glm.hpp>

namespace BAK::Encounter {

class Encounter;
class Combat;

CardinalDirection CalculateRetreatDirection(
    const Encounter& encounter,
    glm::uvec2 playerWorldPos);

const GamePositionAndHeading& GetRetreatPosition(
    const Combat& combat,
    CardinalDirection entryDirection);

}
