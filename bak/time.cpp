#include "bak/time.hpp"

#include "bak/character.hpp"
#include "bak/constants.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "com/logger.hpp"

namespace BAK {

TimeChanger::TimeChanger(GameState& gameState)
:
    mGameState{gameState}
{}

void TimeChanger::HandleGameTimeChange(
    Time timeDelta,
    bool canDisplayDialog,
    bool mustConsumeRations,
    bool isNotSleeping,
    unsigned healFraction,
    unsigned healPercentCeiling)
{
    Logging::LogDebug(__FUNCTION__) << "(" << timeDelta
        << " canDisplayDialog: " << canDisplayDialog << " consRat: "
        << mustConsumeRations << " sleep? "
        << isNotSleeping << " hl: " << healFraction << " - " << healPercentCeiling << ")\n";
    auto& currentTime = mGameState.GetWorldTime();
    auto oldTime = currentTime;
    auto hourOfDay = currentTime.GetTime().GetHour();

    auto daysElapsedBefore = currentTime.GetTime().GetDays();
    currentTime.AdvanceTime(timeDelta);
    auto daysElapsedAfter = currentTime.GetTime().GetDays();

    if (daysElapsedAfter != daysElapsedBefore)
    {
        if ((daysElapsedAfter % 30) == 0)
        {
            ImproveActiveCharactersHealthAndStamina();
        }

        if (mustConsumeRations)
        {
            PartyConsumeRations();
        }

        mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
            ImproveNearDeath(character.GetSkills(), character.GetConditions());
            return Loop::Continue;
        });
    }

    auto newHourOfDay = currentTime.GetTime().GetHour();
    if (newHourOfDay != hourOfDay)
    {
        HandleGameTimeIncreased(
            canDisplayDialog, 
            isNotSleeping,
            healFraction,
            healPercentCeiling);
    }

    CheckAndClearSkillAffectors();
    mGameState.ReduceAndEvaluateTimeExpiringState(timeDelta);
}

void TimeChanger::ElapseTimeInMainView(Time delta)
{
    HandleGameTimeChange(
        delta,
        true, true, true, 0, 0);
}

void TimeChanger::ElapseTimeInSleepView(Time delta, unsigned healFraction, unsigned healPercentCeiling)
{
    HandleGameTimeChange(
        BAK::Times::OneHour,
        true, true, false, healFraction, healPercentCeiling);
    mGameState.GetWorldTime().SetTimeLastSlept(
        mGameState.GetWorldTime().GetTime());
}

void TimeChanger::ImproveActiveCharactersHealthAndStamina()
{
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
        character.ImproveSkill(SkillType::Health, SkillChange::Direct, 1);
        character.ImproveSkill(SkillType::Stamina, SkillChange::Direct, 1);
        return Loop::Continue;
    });
}

void TimeChanger::ConsumeRations(Character& character)
{
    auto rations = InventoryItemFactory::MakeItem(sRations, 1);
    auto spoiled = InventoryItemFactory::MakeItem(sSpoiledRations, 1);
    auto poisoned = InventoryItemFactory::MakeItem(sPoisonedRations, 1);
    if (character.RemoveItem(rations))
    {
        // Eating rations instantly removes starving...
        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            BAK::Condition::Starving,
            -100);
    }
    else if (character.RemoveItem(spoiled))
    {
        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            BAK::Condition::Starving,
            -100);
        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            BAK::Condition::Sick,
            3);
    }
    else if (character.RemoveItem(poisoned))
    {
        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            BAK::Condition::Poisoned,
            4);
    }
    else // no rations of any kind
    {
        character.GetConditions().AdjustCondition(
            character.GetSkills(),
            BAK::Condition::Starving,
            5);
    }
}

void TimeChanger::PartyConsumeRations()
{
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
        ConsumeRations(character);
        return Loop::Continue;
    });
}

void TimeChanger::HandleGameTimeIncreased(
    bool canDisplayDialog,
    bool isNotSleeping,
    unsigned healFraction,
    unsigned healPercentCeiling)
{
    Logging::LogDebug(__FUNCTION__) << "(" << canDisplayDialog
        << ", " << isNotSleeping << ", " << healFraction 
        << " - " << healPercentCeiling << ")\n";
    if (canDisplayDialog)
    {
        if (isNotSleeping)
        {
            auto timeDiff = mGameState.GetWorldTime().GetTimeSinceLastSlept();
            if (timeDiff >= Times::SeventeenHours)
            {
                //ShowNeedSleepDialog(0x40)
                Logging::LogWarn(__FUNCTION__) << " We need sleep!\n";
            }
            if (timeDiff >= Times::EighteenHours)
            {
                Logging::LogWarn(__FUNCTION__) << " Damaging due to lack of  sleep!\n";
                mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
                    DamageDueToLackOfSleep(character.GetConditions(), character.mCharacterIndex, character.GetSkills());
                    return Loop::Continue;
                });
            }
        }
    }

    mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
        EffectOfConditionsWithTime(
            character.GetSkills(),
            character.GetConditions(),
            healFraction,
            healPercentCeiling);
        return Loop::Continue;
    });
}

unsigned TimeChanger::CalculateTimeOfDayForPalette()
{
    auto time = mGameState.GetWorldTime().GetTime();
    auto hourOfDay = time.GetHour();
    auto secondsOfDay = time.mTime % Times::OneDay.mTime;

    if (hourOfDay > 8 && hourOfDay < 17)
    {
        return 0x40;
    }
    if (hourOfDay < 4 || hourOfDay >= 20)
    {
        return 0xf;
    }
    if (hourOfDay < 17)
    {
        secondsOfDay -= 0x1c20;
        auto x = (secondsOfDay * 0x31) / 0x1c20;
        return x + 0xf;
    }
    else
    {
        secondsOfDay += 0x8878;
        auto x = (secondsOfDay * 0x31) / 0x1518;
        return 0x40 - x;
    }
}

void TimeChanger::CheckAndClearSkillAffectors()
{
    const auto time = mGameState.GetWorldTime().GetTime();
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
        auto& affectors = character.GetSkillAffectors();
        std::erase_if(
            affectors,
            [&](auto& affector){
                return affector.mEndTime < time;
            });
        return Loop::Continue;
    });
}

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
