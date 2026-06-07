#pragma once

#include "bak/character.hpp"
#include "bak/combat/mechanics.hpp"

#include <vector>

namespace Game::Combat {


struct Combatant
{
    BAK::Character* mCharacter;
    BAK::MonsterIndex mMonster;
    glm::uvec2 mGridPos;
    BAK::Combat::CombatantState mState;

};

class CombatManager
{
public:
    void AddCombatant(Combatant combatant)
    {
        mCombatants.emplace_back(combatant);
    }

    std::vector<Combatant> GetCombatants()
    {
        return mCombatants;
    }

    void Clear()
    {
        mCombatants.clear();
    }

private:
    std::vector<Combatant> mCombatants{};

};

}
