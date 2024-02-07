#include "bak/time.hpp"

namespace BAK {

void EffectOfConditionsWithTime(
    Skills& skills,
    Conditions& conditions,
    unsigned healFraction,
    unsigned healPercentCeiling)
{
    unsigned conditionChangePcnt = 0;
    int healAmount = 0;
    if (healFraction != 0)
    {
        conditions.AdjustCondition(
            skills,
            BAK::Condition::Sick,
            -3);

        if (healPercentCeiling == 80)
        {
            conditionChangePcnt = 80;
        }
        else
        {
            conditionChangePcnt = 100;
        }
        healAmount = (1 * healFraction) / 0x64;
        if (conditions.GetCondition(Condition::Healing).Get() > 0)
        {
            healAmount *= 2;
        }
    }
    else
    {
        healAmount = 0;
        conditionChangePcnt = 100;
    }

    for (unsigned i = 0; i < 7; i++)
    {
        if (conditions.GetCondition(static_cast<Condition>(i)).Get() > 0)
        {
            // deteriorate amount
            int deterioration = static_cast<std::int16_t>(sConditionSkillEffect[i][0]);
            if (i < static_cast<unsigned>(Condition::Healing))
            {
                if (conditions.GetCondition(Condition::Healing).Get() > 0)
                {
                    auto deteriorateReduction = 2;
                    if (i == static_cast<unsigned>(Condition::Sick))
                    {
                        deteriorateReduction += 1;
                    }
                    deterioration -= deteriorateReduction;
                }
            }

            conditions.AdjustCondition(skills, static_cast<Condition>(i), deterioration);
            
            if (conditions.GetCondition(static_cast<Condition>(i)).Get() > 0)
            {
                const auto reduction = static_cast<std::int16_t>(sConditionSkillEffect[i][1]);
                healAmount += reduction;
            }
        }
    }

    if (healAmount != 0)
    {
        skills.ImproveSkill(
            conditions,
            SkillType::TotalHealth,
            static_cast<SkillChange>(conditionChangePcnt),
            healAmount << 8);
    }
}

void ImproveNearDeath(
    Skills& skills,
    Conditions& conditions)
{
    const auto nearDeathValue = conditions.GetCondition(Condition::NearDeath).Get();
    if (nearDeathValue == 0)
    {
        return;
    }
    const auto healing = conditions.GetCondition(Condition::Healing).Get();
    auto improveAmount = ((nearDeathValue - 100) / 10) - 1;
    if (healing > 0)
    {
        improveAmount *= 2;
    }

    conditions.AdjustCondition(skills, Condition::NearDeath, improveAmount);
}

void DamageDueToLackOfSleep(
    Conditions& conditions,
    CharIndex charIndex,
    Skills& skills)
{
    static constexpr std::array<int, 6> damagePerCharacter = {-2, -1, -2, -2, -2, -3};
    skills.ImproveSkill(
        conditions,
        SkillType::TotalHealth,
        SkillChange::HealMultiplier_100,
        damagePerCharacter[charIndex.mValue] << 8);
}

}
