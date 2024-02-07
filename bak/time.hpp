#pragma once

#include "bak/gameState.hpp"

namespace BAK {

void EffectOfConditionsWithTime(
    Skills& skills,
    Conditions& conditions,
    unsigned healFraction,
    unsigned healPercentCeiling);

void ImproveNearDeath(
    Skills& skills,
    Conditions& conditions);

void DamageDueToLackOfSleep(
    Conditions& conditions,
    CharIndex charIndex,
    Skills& skills);

class TimeChanger
{
public:
    TimeChanger(GameState& gameState)
    :
        mGameState{gameState}
    {}

    void HandleGameTimeChange(
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
                return false;
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
        //EvaluateTimeExpiringState(timeDelta);
    }

    // Time change per step
    //   minStep: 0x1e 30 secs distance: 400
    //   medStep: 0x3c 60 secs distance: 800
    //   bigStep: 0x78 120 secs distance: 1600
    void ElapseTimeInMainView(Time delta)
    {
        HandleGameTimeChange(
            delta,
            true, true, true, 0, 0);
    }

    void ElapseTimeInSleepView(Time delta, unsigned healFraction, unsigned healPercentCeiling)
    {
        HandleGameTimeChange(
            BAK::Times::OneHour,
            true, true, false, healFraction, healPercentCeiling);
        mGameState.GetWorldTime().SetTimeLastSlept(
            mGameState.GetWorldTime().GetTime());
    }
private:
    void ImproveActiveCharactersHealthAndStamina()
    {
        mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
            character.ImproveSkill(SkillType::Health, SkillChange::Direct, 1);
            character.ImproveSkill(SkillType::Stamina, SkillChange::Direct, 1);
            return false;
        });
    }

    void ConsumeRations(auto& character)
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

    void PartyConsumeRations()
    {
        mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
            ConsumeRations(character);
            return false;
        });
    }

    void HandleGameTimeIncreased(
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
                        return false;
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
            return false;
        });
    }

    unsigned calculateTimeOfDayForPalette()
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

    void CheckAndClearSkillAffectors()
    {
        const auto time = mGameState.GetWorldTime().GetTime();
        mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
            auto& affectors = character.GetSkillAffectors();
            std::erase_if(
                affectors,
                [&](auto& affector){
                    return affector.mEndTime < time;
                });
            return false;
        });
    }

private:
    GameState& mGameState;
};

}
