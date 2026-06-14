#include "bak/combat/retreat.hpp"

#include "bak/constants.hpp"
#include "com/assert.hpp"
#include "bak/encounter/combat.hpp"
#include "bak/encounter/encounter.hpp"

namespace BAK::Encounter {

CardinalDirection CalculateRetreatDirection(
    const Encounter& encounter,
    glm::uvec2 playerWorldPos)
{
    const auto tileOffset = encounter.GetTile()
        * glm::uvec2{static_cast<unsigned>(gTileSize)};
    const auto tl = encounter.mTopLeft + tileOffset;
    const auto br = encounter.mBottomRight + tileOffset;

    const auto distNorth = tl.y - playerWorldPos.y;
    const auto distSouth = playerWorldPos.y - br.y;
    const auto distEast  = br.x - playerWorldPos.x;
    const auto distWest  = playerWorldPos.x - tl.x;

    const auto minDist = std::min({distNorth, distSouth, distEast, distWest});
    if (minDist == distNorth) return CardinalDirection::North;
    if (minDist == distSouth) return CardinalDirection::South;
    if (minDist == distEast)  return CardinalDirection::East;
    return CardinalDirection::West;
}

const GamePositionAndHeading& GetRetreatPosition(
    const Combat& combat,
    CardinalDirection entryDirection)
{
    switch (entryDirection)
    {
    case CardinalDirection::North: return combat.mNorthRetreat;
    case CardinalDirection::South: return combat.mSouthRetreat;
    case CardinalDirection::East:  return combat.mEastRetreat;
    case CardinalDirection::West:  return combat.mWestRetreat;
    }
    ASSERT(false);
    return combat.mNorthRetreat;
}

}
