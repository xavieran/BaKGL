#pragma once

#include "bak/skills.hpp"

namespace BAK {

class Bard
{
public:

    void DoBard(unsigned bardingSkill, unsigned innRequirement)
    {
        const auto xpGained = 128 - (112 * (bardingSkill / 100.));
        if (bardingSkill < innRequirement)
        {
            // TrainSkill(3, 1, 0xB);
            if (bardingSkill < ((innRequirement * 3) / 4))
            {
                // Fail
                // Play song 0x3f0
            }
            else
            {
                // Minimum wage
                // Play song 0x410
                // auto reward = Royals{(innReward * 10) / 4}
            }
        }
        else
        {
            // TrainSkill(3, 2, 0xB);
            if (bardingSkill < ((innRequirement + 100) / 2))
            {
                // Good
                // Play song 0x40f
                // auto reward = Royals{(innReward * 10) / 2}
            }
            else
            {
                // Perfect
                // Play song 0x3ef
                // auto reward = Royals{(innReward * 10)}
            }
            // xpGained *= 2;
        }


    }

};

}
