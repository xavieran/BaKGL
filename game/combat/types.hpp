#pragma once

#include "bak/character.hpp"
#include "bak/combat/mechanics.hpp"
#include "bak/combat/types.hpp"
#include "bak/coordinates.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace Game::Combat
{

using GridPos = glm::ivec2;
using BAK::AttackType;

struct Combatant
{
    BAK::Character* mCharacter;
    BAK::MonsterIndex mMonster;
    GridPos mGridPos;
    BAK::Combat::CombatantState mState;
    BAK::EntityIndex mEntityIndex;
    BAK::CombatState mCombatState{};

    bool& GetTurnPending() { return mCombatState.mTurnPending; }
    bool GetTurnPending() const { return mCombatState.mTurnPending; }
    bool& IsDead() { return mCombatState.mIsDead; }
    bool IsDead() const { return mCombatState.mIsDead; }
    bool& IsPoisoned() { return mCombatState.mIsPoisoned; }
    bool IsPoisoned() const { return mCombatState.mIsPoisoned; }
    bool& IsDefending() { return mCombatState.mIsDefending; }
    bool IsDefending() const { return mCombatState.mIsDefending; }
    bool& IsFleeing() { return mCombatState.mIsFleeing; }
    bool IsFleeing() const { return mCombatState.mIsFleeing; }
    bool& IsExorcised() { return mCombatState.mIsExorcised; }
    bool IsExorcised() const { return mCombatState.mIsExorcised; }
};

enum class StateFlags
{
    Reachable     = 0,
    Attackable    = 1,
    IsAlly        = 2,
    LOSAttackable = 3,
    HasZap        = 4,
    HasMine       = 5,
    HasCrystal    = 6
};

struct Move
{
    GridPos mTarget;
};

struct Attack
{
    GridPos mTarget;
    BAK::AttackType mType;
};

struct AnimateDeath
{
    GridPos mTarget;
};

using CombatAction = std::variant<
    Move,
    Attack,
    AnimateDeath
    >;

std::ostream& operator<<(std::ostream& os, const Move&);
std::ostream& operator<<(std::ostream& os, const Attack&);
std::ostream& operator<<(std::ostream& os, const AnimateDeath&);
std::ostream& operator<<(std::ostream& os, const CombatAction&);

}
