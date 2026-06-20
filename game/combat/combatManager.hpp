#pragma once

#include "game/combat/grid.hpp"

#include "bak/combat/ICombatManager.hpp"

#include "bak/character.hpp"
#include "bak/combat/mechanics.hpp"
#include "bak/coordinates.hpp"

#include <vector>

namespace Game::Combat {


// Make it polymorphic?
struct Combatant
{
    BAK::Character* mCharacter;
    BAK::MonsterIndex mMonster;
    glm::uvec2 mGridPos;
    BAK::Combat::CombatantState mState;
    BAK::EntityIndex mEntityIndex;

    bool mTurnPending{true};
    bool mIsDead{false};
    bool mIsPoisoned{false};
};

struct GridElem
{
    BAK::GamePositionAndHeading mPos;
    Combatant* mElement;
};

/* 
 * Combat procedure
 * Each turn
 *   Order the combatants by speed
 *   In order, combatants take a turn
 *
 * Combatant turn
 *   | Defend
 *   | Rest
 *   | Assess
 *   | Cast
 *   | Shoot
 *   | Move
 *     + Melee
 *   | Melee
 *   Next combatant
 */

class CombatManager : public BAK::ICombatManager
{
public:
    void SetCastingSpell(BAK::SpellIndex) override {}
    void SetUsingCrossbow() override {}

    void AddCombatant(Combatant combatant)
    {
        mCombatants.emplace_back(combatant);
    }

    Combatant* GetCombatant(BAK::CharIndex character)
    {
        auto it = std::find_if(mCombatants.begin(), mCombatants.end(),
            [&](auto& combatant){
                return (combatant.mCharacter != nullptr)
                    && combatant.mCharacter->GetIndex() == character;
        });

        if (it != mCombatants.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    Combatant* GetCombatant(BAK::EntityIndex entityIndex)
    {
        auto it = std::find_if(mCombatants.begin(), mCombatants.end(),
            [&](auto& combatant){
                return combatant.mEntityIndex == entityIndex;
        });

        if (it != mCombatants.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    Combatant* GetCombatant(glm::uvec2 gridPos)
    {
        auto it = std::find_if(mCombatants.begin(), mCombatants.end(),
            [&](auto& combatant){
                return combatant.mGridPos == gridPos;
        });

        if (it != mCombatants.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    std::vector<Combatant> GetCombatants()
    {
        return mCombatants;
    }

    void Clear()
    {
        mCombatants.clear();
    }

    void SetCurrentCombatant(bool onlyParty)
    {
        std::optional<unsigned> bestSpeed{};
        auto combatant = 0;
        for (unsigned i = 0; i < mCombatants.size(); i++)
        {
            auto* character = mCombatants[combatant].mCharacter;
            if (!character)
            {
                continue;
            }

            if (onlyParty && character->IsEnemy())
            {
                continue;
            }

            auto speed = character->GetSkill(BAK::SkillType::Speed);

            if (!bestSpeed || speed > *bestSpeed)
            {
                bestSpeed = speed;
                combatant = i;
            }
        }

        mCurrentCombatant = combatant;
    }

    Combatant& GetCurrentCombatant()
    {
        assert(mCurrentCombatant < mCombatants.size());
        return mCombatants[mCurrentCombatant];
    }

private:
    std::vector<Combatant> mCombatants{};

    std::size_t mCurrentCombatant;
    Grid<GridElem> mGrid;

};

}
