#pragma once

#include <ostream>
#include <vector>

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

struct SpellEffect
{
    std::uint16_t mEffectId;
    std::uint16_t mParamA;
    int mAmount;
    std::uint16_t mCounter;
    std::uint8_t mFlags;
};

struct CombatState
{
    bool mTurnPending{true};
    bool mIsDead{false};
    bool mIsPoisoned{false};
    bool mIsDefending{false};
    bool mIsFrozen{false};
    bool mIsExorcsed{false};
    bool mIsFleeing{false};

    //std::optional<GridPos> mCurrentTarget;
    std::vector<SpellEffect> mSpellEffects{};
};

enum class AttackType
{
    Slash,
    Thrust
};

std::ostream& operator<<(std::ostream& os, AttackType type);

}
