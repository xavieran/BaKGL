#include "bak/condition.hpp"

namespace BAK {

std::string_view ToString(Condition s)
{
    switch (s)
    {
    case Condition::Sick: return "Sick";
    case Condition::Plagued: return "Plagued";
    case Condition::Poisoned: return "Poisoned";
    case Condition::Drunk: return "Drunk";
    case Condition::Healing: return "Healing";
    case Condition::Starving: return "Starving";
    case Condition::NearDeath: return "NearDeath";
    default: return "UnknownCondition";
    }
}

std::ostream& operator<<(std::ostream& os, const Conditions& s)
{
    os << "Conditions: { ";
    char sep = ' ';
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        os << sep << ToString(static_cast<Condition>(i)) 
            << ": " << s.mConditions[i] << "%";
        sep = ',';
    }
    os << "}";
    return os;
}


bool Conditions::NoConditions() const
{
    for (const auto cond : mConditions)
        if (cond != 0) return false;
    return true;
}

const ConditionValue& Conditions::GetCondition(BAK::Condition cond) const
{
    const auto i = static_cast<unsigned>(cond);
    ASSERT(i < sNumConditions);
    return mConditions[i];
}

void Conditions::IncreaseCondition(BAK::Condition cond, signed value)
{
    const auto i = static_cast<unsigned>(cond);
    ASSERT(i < sNumConditions);
    mConditions[i] += value;
}

void Conditions::AdjustCondition(BAK::Condition cond, signed amount)
{
    const auto currentValue = GetCondition(cond).Get();
    auto newValue = currentValue + amount;
    if (newValue > 100)
    {
        newValue = 100;
    }
    else if (newValue < 0)
    {
        newValue = 0;
    }

    //SetCondition(cond, newValue);

    const bool inCombat = false;

    if (cond != Condition::Healing
        && cond != Condition::Drunk
        && (cond != Condition::NearDeath || !inCombat))
    {
        if (currentValue != 0 && newValue == 0)
        {
            //mGameState.SetEventFlag(activeCharIndex * 7 + static_cast<unsigned>(cond) + sConditionBasedFlag, 0);
        }
        else if (currentValue == 0 && newValue != 0)
        {
            //mGameState.SetEventFlag(activeCharIndex * 7 + static_cast<unsigned>(cond) + sConditionBasedFlag, 1);
        }
    }
    
    if (cond == Condition::NearDeath)
    {
        // Check all character NearDeath
        //bool allDead = true;
        //for (auto character : mActiveCharacter)
        //{
        //    if (character.GetCondition(BAK::Condition::NearDeath).Get() == 0)
        //    {
        //        allDead = false;
        //    }
        //}
        if (amount > 0)
        {
            //ClearAllConditionsExceptNearDeath
            //ClearAllConditionStatesExceptNearDeath
            //SetHealthToZero
            //SetStaminaToZero
            //ChangeHealth(
            //    character,
            //    0x10, // health
            //    0x7fff, // multiplier
            //    0x64, // skillChangeType (percent health change)
            //);
        }
    }
}

}
