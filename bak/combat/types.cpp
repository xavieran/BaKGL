#include "bak/combat/types.hpp"
#include "com/ostream.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, AttackType type)
{
    switch (type)
    {
    case AttackType::Slash: return os << "Slash";
    case AttackType::Thrust: return os << "Thrust";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const SpellEffect& se)
{
    os << "SpellEffect{"
        << "id: " << se.mEffectId
        << ", paramA: " << se.mParamA
        << ", amount: " << se.mAmount
        << ", counter: " << se.mCounter
        << ", flags: " << +se.mFlags
        << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CombatState& state)
{
    os << "CombatState{"
        << std::boolalpha
        << "turnPending: " << state.mTurnPending
        << ", isDead: " << state.mIsDead
        << ", isPoisoned: " << state.mIsPoisoned
        << ", isDefending: " << state.mIsDefending
        << ", isFrozen: " << state.mIsFrozen
        << ", isExorcised: " << state.mIsExorcised
        << ", isFleeing: " << state.mIsFleeing
        << ", spellEffects: [" << state.mSpellEffects << "]"
        << "}";
    return os;
}

}
