#pragma once

#include "bak/coordinates.hpp"

#include <cstdint>
#include <iosfwd>
#include <string_view>

#include <array>

namespace BAK {

enum class CombatResult : std::uint8_t
{
    None = 0,
    Won = 1,
    Fled = 2,
    Dead = 3
};

std::string_view ToString(CombatResult);

enum class CombatantWorldState : std::uint8_t
{
    Invisible0 = 0,
    Invisible1 = 1,
    Moving0 = 2,
    Moving1 = 3,
    Dead = 4
};

std::string_view ToString(CombatantWorldState);

class CombatWorldLocation
{
public:
    GamePositionAndHeading mPosition;
    // The game has 3 variants of each direction, this index
    // controls which of these variants is displayed. As the
    // combatant moves the index changes to simulate movement.
    std::uint8_t mImageIndex;
    CombatantWorldState mState;

    bool IsLoaded() const;
};

std::ostream& operator<<(std::ostream&, const CombatWorldLocation&);

class CombatantGridLocation
{
public:
    std::uint16_t mUnknown0;
    MonsterIndex mMonster;
    glm::uvec2 mGridPos;
    std::uint16_t mUnknown1;
    std::uint8_t mState;
    std::array<std::uint8_t, 13> mRest;
};

std::ostream& operator<<(std::ostream&, const CombatantGridLocation&);

/* The combat entity lists in the game (at least versions I have
 * access to) seem to be incorrect. They begin to differ from the
 * combatant inventories (which I take to be the truth) at entry 20.
 * Refer to notes/combat_entity_list.txt for more info.
 */
class CombatEntityList
{
public:
    std::vector<CombatantIndex> mCombatants;
};

std::ostream& operator<<(std::ostream&, const CombatEntityList&);

void LoadP1Dat();

// Used by combat end to decide whether to use dialog 0x142
bool IsSpecialBattle(CombatIndex);

static constexpr CombatIndex MakalaCombat{0x221};
    
}
