#pragma once

#include <ostream>

namespace BAK {

struct MeleeInfo
{
    unsigned mThrustChance;
    unsigned mThrustDamage;
    unsigned mSlashChance;
    unsigned mSlashDamage;
};

enum class MeleeResult
{
    Miss,
    Hit
};

struct CombatState
{
    bool mTurnPending{true};
    bool mIsDead{false};
    bool mIsPoisoned{false};
    bool mIsDefending{false};
    bool mIsFrozen{false};
    bool mIsFleeing{false};
};

enum class AttackType
{
    Slash,
    Thrust
};

std::ostream& operator<<(std::ostream& os, AttackType type);

}
