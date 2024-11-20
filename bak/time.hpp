#pragma once

#include "bak/types.hpp"

namespace BAK {

class Time;
class Skills;
class Character;
class Conditions;
class GameState;

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
    TimeChanger(GameState& gameState);

    void HandleGameTimeChange(
        Time timeDelta,
        bool canDisplayDialog,
        bool mustConsumeRations,
        bool isNotSleeping,
        unsigned healFraction,
        unsigned healPercentCeiling);

    // Time change per step
    //   minStep: 0x1e 30 secs distance: 400
    //   medStep: 0x3c 60 secs distance: 800
    //   bigStep: 0x78 120 secs distance: 1600
    void ElapseTimeInMainView(Time delta);
    void ElapseTimeInSleepView(Time delta, unsigned healFraction, unsigned healPercentCeiling);
private:
    void ImproveActiveCharactersHealthAndStamina();
    void ConsumeRations(Character& character);
    void PartyConsumeRations();

    void HandleGameTimeIncreased(
        bool canDisplayDialog,
        bool isNotSleeping,
        unsigned healFraction,
        unsigned healPercentCeiling);

    unsigned CalculateTimeOfDayForPalette();
    void CheckAndClearSkillAffectors();

private:
    GameState& mGameState;
};

}
