#pragma once

#include "bak/coordinates.hpp"

#include <cstdint>
#include <iosfwd>

namespace BAK {

enum class CombatantWorldState : std::uint8_t
{
    Invisible0,
    Invisible1,
    Moving0,
    Moving1,
    Dead
};

class CombatWorldLocation
{
public:
    GamePositionAndHeading mPosition;
    // The game has 3 variants of each direction, this index
    // controls which of these variants is displayed. As the
    // combatant moves the index changes to simulate movement.
    std::uint8_t mImageIndex;
    // 0 - invisible?
    // 1 - invisible?
    // 2 - moving
    // 3 - moving
    // 4 - dead
    std::uint8_t mState;
};

std::ostream& operator<<(std::ostream&, const CombatWorldLocation&);

class CombatantGridLocation
{
public:
    MonsterIndex mMonster;
    glm::uvec2 mGridPos;
};

std::ostream& operator<<(std::ostream&, const CombatantGridLocation&);

class CombatEntityList
{
public:
    std::vector<CombatantIndex> mCombatants;
};

std::ostream& operator<<(std::ostream&, const CombatEntityList&);

void LoadP1Dat();
    
}
