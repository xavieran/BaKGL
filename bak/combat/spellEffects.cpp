#include "bak/combat/spellEffects.hpp"

#include "bak/character.hpp"
#include "bak/combat/types.hpp"
#include "bak/condition.hpp"
#include "bak/skills.hpp"
#include "bak/spells.hpp"

#include <algorithm>

namespace BAK {

[[nodiscard]] bool TickCombatEffects(CombatState& combatState)
{
    auto& effects = combatState.mSpellEffects;
    bool expiredCharacter = false;

    for (auto& effect : effects)
    {
        effect.mAmount--;

        if (effect.mAmount == 0 && effect.mEffectId == sDannonsDelusions)
        {
            expiredCharacter = true;
        }
    }

    effects.erase(
        std::remove_if(effects.begin(), effects.end(), [](auto& effect)
        {
            return effect.mAmount == 0;
        }),
        effects.end());

    return expiredCharacter;
}

SpellEffect* GetSpellEffect(CombatState& combatState, unsigned effectId)
{
    auto& effects = combatState.mSpellEffects;
    auto it = std::find_if(effects.begin(), effects.end(), [effectId](const auto& effect)
    {
        return effect.mEffectId == effectId;
    });

    if (it != effects.end())
    {
        return &(*it);
    }

    return nullptr;
}

void RemoveSpellEffect(CombatState& combatState, unsigned effectId)
{
    auto& effects = combatState.mSpellEffects;
    auto it = std::find_if(effects.begin(), effects.end(), [effectId](const auto& effect)
    {
        return effect.mEffectId == effectId;
    });

    if (it == effects.end())
    {
        return;
    }

    effects.erase(it);
}

void CleanCharacterStateOnDeath(Character& character, CombatState& combatState)
{
    combatState.mSpellEffects.clear();

    auto& skills = character.GetSkills();
    skills.GetSkill(SkillType::Health).mTrueSkill = 0;
    character.GetSkill(SkillType::Health);
    skills.GetSkill(SkillType::Stamina).mTrueSkill = 0;
    character.GetSkill(SkillType::Stamina);

    combatState.mIsDead = true;

    character.AdjustCondition(BAK::Condition::NearDeath, 100);
}

}
