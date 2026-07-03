#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialogTarget.hpp"

#include <cstdint>
#include <iosfwd>
#include <string_view>

#include <array>

namespace BAK {

enum class CombatOutcome : std::uint8_t
{
    None = 0,
    Won = 1,
    Fled = 2,
    Dead = 3
};

std::string_view ToString(CombatOutcome);

struct CombatResult
{
    CombatOutcome mOutcome{};
    BAK::Target mDialog{};
    unsigned mContextVar{};
};

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
    std::uint16_t mCurrentTarget;
    MonsterIndex mMonster;
    glm::uvec2 mGridPos;
    glm::uvec2 mTarget;
    std::uint8_t mState;
    std::uint8_t mHealthChangeAmount;
    std::uint8_t mEffectPtr;
    std::array<std::uint8_t, 2> mUnknown;
    std::uint8_t mHealthChangeColor;
    std::uint8_t mRetreatFactor;
    std::array<std::uint8_t, 5> mUnknown2;
    std::uint8_t mDisplayDamage;
    std::uint8_t mHitTextType;
};

std::ostream& operator<<(std::ostream&, const CombatantGridLocation&);

struct CombatRelInfo
{
    CombatIndex mCombatIndex;
    unsigned mCombatantRelativeIndex;
};

std::ostream& operator<<(std::ostream&, const CombatRelInfo&);

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
