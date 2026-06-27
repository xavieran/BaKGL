#include "bak/combat/calculations.hpp"

#include "bak/character.hpp"
#include "bak/combat/types.hpp"

namespace BAK {

MeleeInfo CalculateMeleeInfo(const Character& character)
{
    const auto meleeAcc = character.GetSkill(SkillType::Melee);
    const auto strength = character.GetSkill(SkillType::Strength);
    const auto& weapon  = character.GetMeleeWeapon();
    return MeleeInfo{
        meleeAcc + weapon.GetObject().mAccuracyThrust,
        strength + weapon.GetObject().mStrengthThrust,
        meleeAcc + weapon.GetObject().mAccuracySwing,
        strength + weapon.GetObject().mStrengthSwing
    };
}

}
