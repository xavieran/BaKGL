#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/gameState.hpp"

#include <glm/glm.hpp>

namespace BAK {

class CampData
{
public:
    CampData();

    const std::vector<glm::vec2>& GetClockTicks() const;
    const std::vector<glm::vec2>& GetDaytimeShadow() const;
private:
    glm::vec2 mHighlightSize;
    std::vector<glm::vec2> mClockTicks;
    glm::vec2 mClockTwelve;
    glm::vec2 mClockCenter;
    std::vector<glm::vec2> mDaytimeShadow;
};

class MakeCamp
{
    MakeCamp(GameState& gameState)
    :
        mGameState{gameState}
    {}

    void ImproveActiveCharactersHealthAndStamina()
    {
    }

    void PartyConsumeRations()
    {
        // if (HaveNormalRations)
        // {
        //  ReduceStarving(100)
        // }
        // else if (HaveSpoiledRations)
        // {
        //     SetStarving(100);
        // }
        // else if (HavePoisonedRations)
        // {
        //   increase posioned by 4
        // }
        // else // no rations of any kind
        // {
        //  increase starving by 5
        // }
    }

    void Camp(unsigned hours, unsigned healPercent)
    {
//ovr181:24a

    }

    void HandleGameTimeChange(
        unsigned timeDelta,
        unsigned arg4,
        unsigned passedMidnight,
        unsigned arg8,
        unsigned percentage) // inn vs outdoors heal percentage?
    {
        //handleGameTimeChange
        //4221:7b5
        auto currentTime = 0;
        auto timeDeltaHours = Time{timeDelta} * Times::OneHour;
        auto hourOfDay = Time(currentTime % Times::OneDay.mTime) / Times::OneHour;

        auto daysElapsedBefore = currentTime / Times::OneDay.mTime;
        currentTime += timeDeltaHours.mTime;
        auto daysElapsedAfter = currentTime / Times::OneDay.mTime;
        if (daysElapsedAfter != daysElapsedBefore)
        {
            if (daysElapsedAfter > 30)
            {
                ImproveActiveCharactersHealthAndStamina();
            }
            if (passedMidnight)
            {
                PartyConsumeRations();
            }
            //IncreaseAllCharactersConditionByConditionAmount;
        }
        auto newHourOfDay = (currentTime % Times::OneDay.mTime) / Times::OneHour.mTime;
        if (newHourOfDay != hourOfDay.mTime)
        {
            auto arg4 = 1;
            auto arg8 = 0;

            HandleGameTimeIncreased(
                arg4, 
                arg8,
                percentage);
        }

        //EvaluateTimeExpiringState(timeDelta);
    }

    // HandleGameTimeChangeFromSleep calls HandleGameTimeIncreased with:
    //     HandleGameTimeIncreased(timeDeltaHours, 1, 1, healPercent??)
    // RunDialog calls it with
    //     HandleGameTimeChange(0x708 or timeDelta,1, charSkillWho?, 0
    // RepairScreen calls it with
    //     HandleGameTimeChange(0x708, varA, 1, 1)
    // MainView calls it twice with either
    //     HandleGameTimeIncreased(timePerStep, 0x10001, 1) or
    //     HandleGameTimeIncreased(timePerStep/4, 0x10001, 1)
    // MapView calls it twice with either
    //     HandleGameTimeIncreased(timePerStep, 0x10001, 1) or
    //     HandleGameTimeIncreased(timePerStep/4, 0x10001, 1)

    void HandleGameTimeIncreased(
        unsigned x,
        unsigned y,
        unsigned healPercent)
    {
        auto dx = y;
        auto var6 = 0;
        auto var8 = 1;

        auto timeSinceLastSleep = 5;
        auto currentTime = 3;
        auto timeDiff = currentTime - timeSinceLastSleep;

        // check what these are, I think its whether we camp or not???
        if (x == 0 && y == 1)
        {
        }

        mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
            for (unsigned cond = 0; cond < 7; cond++)
            {
                if (healPercent != 0)
                {
                    character.GetConditions().AdjustCondition(
                        static_cast<BAK::Condition>(cond),
                        0xfffd - 0xffff);
                    auto healPcnt = 0;
                    if (healPercent == 0x64)
                    {
                        healPcnt = 0x50;
                    }
                    else
                    {
                        healPcnt = 0x64;
                    }
                    auto healingMultiplierPerChar_si = 1;
                    auto x = (healingMultiplierPerChar_si * 0x64) / 0x64;
                    if (0) // getCondition(cond) != 0
                    {
                        auto dx = 0; // conditionHealEffectArray[cond];
                        // adjustConditionBy(dx)
                        // adjustConditionBy(0xfffe);
                    }
                    auto bx = 0;
                }
                else
                {
                //??
                }
            }
        });
    }

    GameState& mGameState;
};

}
