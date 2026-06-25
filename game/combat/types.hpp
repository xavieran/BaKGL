#pragma once

#include "bak/character.hpp"
#include "bak/combat/mechanics.hpp"
#include "bak/coordinates.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace Game::Combat
{

using GridPos = glm::ivec2;

struct Combatant
{
    BAK::Character* mCharacter;
    BAK::MonsterIndex mMonster;
    GridPos mGridPos;
    BAK::Combat::CombatantState mState;
    BAK::EntityIndex mEntityIndex;

    bool mTurnPending{true};
    bool mIsDead{false};
    bool mIsPoisoned{false};
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

enum class AttackType
{
    Slash,
    Thrust
};

struct Attack
{
    GridPos mTarget;
    AttackType mType;
};

using CombatAction = std::variant<
    Move,
    Attack>;

std::ostream& operator<<(std::ostream& os, Move move);
std::ostream& operator<<(std::ostream& os, AttackType type);
std::ostream& operator<<(std::ostream& os, const Attack& attack);
std::ostream& operator<<(std::ostream& os, const CombatAction& action);

}
