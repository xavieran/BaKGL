#include "bak/condition.hpp"

#include "bak/skills.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

namespace BAK {

std::string_view ToString(Condition s)
{
    using enum Condition;
    switch (s)
    {
    case Sick: return "Sick";
    case Plagued: return "Plagued";
    case Poisoned: return "Poisoned";
    case Drunk: return "Drunk";
    case Healing: return "Healing";
    case Starving: return "Starving";
    case NearDeath: return "NearDeath";
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

void Conditions::SetCondition(BAK::Condition cond, std::uint8_t amount)
{
    mConditions[static_cast<std::uint8_t>(cond)] = ConditionValue{amount};
}

void Conditions::AdjustCondition(Skills& skills, BAK::Condition cond, signed amount)
{
    Logging::LogDebug(__FUNCTION__) << " called with : cond: " << ToString(cond) << " amt: " << amount << "\n";
    const auto currentValue = GetCondition(cond).Get();
    int newValue = currentValue + amount;
    if (newValue > 100)
    {
        newValue = 100;
    }
    else if (newValue < 0)
    {
        newValue = 0;
    }

    SetCondition(cond, newValue);

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
            Logging::LogDebug(__FUNCTION__) << " Near death...\n";
            for (unsigned i = 0; i < Conditions::sNumConditions - 1; i++)
            {
                SetCondition(static_cast<Condition>(i), 0);
            }
            SetCondition(BAK::Condition::NearDeath, 100);
            skills.GetSkill(SkillType::Health).mTrueSkill = 0;
            skills.GetSkill(SkillType::Stamina).mTrueSkill = 0;
            // Near death condition will inhibit this health "increase"...
            skills.ImproveSkill(*this, SkillType::TotalHealth, SkillChange::HealMultiplier_100, 0x7fff);
        }
    }
}

}
