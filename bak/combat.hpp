#pragma once

#include "bak/coordinates.hpp"

#include <cstdint>
#include <iosfwd>

namespace BAK {

class CombatWorldLocation
{
public:
    GamePositionAndHeading mPosition;
    std::uint8_t mUnknownFlag;
    // 0 - invisible?
    // 1 - invisible?
    // 2 - moving
    // 3 - moving
    // 4 - dead
    std::uint8_t mState;
};

std::ostream& operator<<(std::ostream&, const CombatWorldLocation&);

class CombatGridLocation
{
public:
    MonsterIndex mMonster;
    glm::uvec2 mGridPos;
};

std::ostream& operator<<(std::ostream&, const CombatGridLocation&);

class CombatEntityList
{
public:
    std::vector<CombatantIndex> mCombatants;
};

std::ostream& operator<<(std::ostream&, const CombatEntityList&);

void LoadP1Dat();
    
}
