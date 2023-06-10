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
    void Camp(unsigned hours, unsigned healPercent)
    {
//ovr181:24a

//handleGameTimeChange
//4221:7b5
        auto currentTime = 0;
        auto timeDeltaHours = hours * 0x708;
        auto hourOfDay = (currentTime % 0xa8c0) / 0x708;

        auto daysElapsed_cx = currentTime / 0xa8c0;
        currentTime += timeDeltaHours;
        auto daysElapedNow = currentTime / 0xa8c0;
        if (daysElapedNow != daysElapsed_cx)
        {
        }
        else
        {
            auto newHourOfDay = (currentTime % 0xa8c0) / 0x708;
            if (newHourOfDay != hourOfDay)
            {
                auto arg4 = 1;
                auto arg8 = 0;

                HandleGameTimeIncreased(
                    arg4, 
                    arg8,
                    healPercent);
            }
        }
    }

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
