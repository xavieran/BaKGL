#include "bak/combat/calculations.hpp"
#include "bak/combat/spellEffects.hpp"

#include "bak/character.hpp"
#include "bak/combat/types.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/monster.hpp"
#include "bak/sounds.hpp"
#include "bak/spells.hpp"

#include "com/random.hpp"

#include <utility>

namespace BAK {

static constexpr auto sItemRaceEffect = std::array<std::int16_t, 12>
{
// None, Tsurani, Elf, Human
     0,   -1,   -1,   -2,   // None
    -1,    0,   -1,   -2,   // Tsurani
    -1,   -1,    0,   -2,   // Elf
};

// triple check this, might be off by one or two ....
static const std::array<std::uint16_t, 64> sMonsterResistanceArr = {
    0,     0,     0,     0, 0, 0,    0, 0, 0,    0,    0,     0,    0,     0, 0,     1,
    0x200, 0,     0,     1, 0, 0xC0, 1, 0, 0,    0xC0, 0x200, 0x0C, 0,     0, 0,     0,
    0,     0,     2,     0, 0, 0,    0, 0, 1,    0,    0x0C,  2,    0,     1, 0x200, 0,
    0x200, 0x100, 0x300, 1, 1, 0,    0, 0, 0,    0xC0, 0xC0,  0,    0,     0, 0x300, 0
 };

bool IsCombatantActive(CombatState& combatState, bool checkPending)
{
    if (checkPending)
    {
        if (!combatState.mTurnPending || combatState.mIsDead)
        {
            return false;
        }
    }

    for (const auto& effect : combatState.mSpellEffects)
    {
        switch (effect.mEffectId)
        {
        case sDannonsDelusions: [[fallthrough]];
        case sDespairThyEyes: [[fallthrough]];
        case sGriefOf1000Nights: return false;
        default: break;
        }
    }

    return true;
}

MeleeInfo CalculateMeleeInfo(Character& character)
{
    const auto meleeAcc = character.GetSkill(SkillType::Melee);
    const auto strength = character.GetSkill(SkillType::Strength);
    const auto* weapon  = character.GetMeleeWeapon();
    auto addCapped = [](int a, int b) -> unsigned
    {
        if ((a + b) <= 0)
        {
            return 1;
        }
        return static_cast<unsigned>(a + b);
    };

    return MeleeInfo{
        addCapped(meleeAcc, weapon->GetObject().mAccuracyThrust),
        addCapped(strength, weapon->GetObject().mStrengthThrust),
        addCapped(meleeAcc, weapon->GetObject().mAccuracySwing),
        addCapped(strength, weapon->GetObject().mStrengthSwing)
    };
}

int CalculateBlessingEffect(
    int value,
    std::vector<Modifier> modifiers)
{
    int factor = 100;
    // This code will break if more than one modifier is set :)
    for (const auto& modifier : modifiers)
    {
        switch (modifier)
        {
            case Modifier::Blessing1:
                factor = 105;
                break;
            case Modifier::Blessing2:
                factor = 110;
                break;
            case Modifier::Blessing3:
                factor = 115;
                break;
            default:
                break;
        }
    }

    return value * factor / 100;
}

int CalculateParry(
    Character& defender,
    CombatState& defenderState)
{
    auto parry = defender.GetSkill(SkillType::Defense) / 4;

    if (!IsCombatantActive(defenderState, false))
    {
        parry = 0;
    }

    auto* armor = defender.GetArmor();
    if (armor)
    {
        parry = CalculateBlessingEffect(parry, armor->GetModifiers());
    }

    if (parry > 98)
    {
        parry = 98;
    }
    if (parry < 0)
    {
        parry = 0;
    }

    return parry;
}

RacialModifier GetCombatantRace(MonsterIndex monster)
{
    switch (monster.mValue)
    {
        case 15: [[fallthrough]]; // gorath
        case 18: [[fallthrough]]; // moredhel
        case 21: // moredhel
            return RacialModifier::Elf;
        default:
            return RacialModifier::None;
    }

    return RacialModifier::None;
}

int CalculateRaceEffect(
    Character& attacker,
    InventoryItem& item)
{
    auto race = static_cast<unsigned>(GetCombatantRace(attacker.GetMonsterIndex()));
    auto itemRace = static_cast<unsigned>(item.GetObject().mRace);
    return sItemRaceEffect[race * 4 + itemRace];
}

int CalculateAccuracyBonus(
    int raceEffect,
    int conditionEffect,
    int baseAccuracy)
{
    auto bonus = (baseAccuracy * (raceEffect + 100) / 100);
    bonus = bonus * conditionEffect / 100;
    return bonus;
}

MeleeResult CalculateMeleeResult(
    Character& attacker,
    Character& defender,
    CombatState& defenderState,
    int weaponAccuracy)
{
    auto hitRoll = GetRandomNumber(0, 0xfff) % 100;
    hitRoll += defenderState.mIsDefending ? 20 : 0;

    auto* weapon = attacker.GetMeleeWeapon();

    auto weaponBonus = 0;
    if (weapon)
    {
        auto raceEffect = CalculateRaceEffect(attacker, *weapon);
        auto condition = weapon->IsConditionBased() ? weapon->GetCondition() : 100;
        weaponBonus = CalculateAccuracyBonus(
            raceEffect,
            condition,
            weaponAccuracy);
    }

    auto skillScore = attacker.GetSkill(SkillType::Melee) + weaponBonus;

    if (weapon)
    {
        skillScore = CalculateBlessingEffect(skillScore, weapon->GetModifiers());
    }


    auto parry = CalculateParry(
        defender,
        defenderState);

    skillScore -= parry;

    if (skillScore < 2)
    {
        skillScore = 2;
    }

    if (skillScore > 98)
    {
        skillScore = 98;
    }

    if (hitRoll < skillScore)
    {
        return MeleeResult::Hit;
    }

    return MeleeResult::Miss;
}

int CalculateArmorModReduction(
    Modifier attackModifier,
    const std::vector<Modifier>& defenseModifiers,
    int bonusDamage)
{
    int reduction = 0;
    for (auto mod : defenseModifiers)
    {
        if (attackModifier == mod)
        {
            reduction = 100;
        }
    }

    // In all cases the protective mod completely
    // neutralises the bonus damage...

    return bonusDamage - ((bonusDamage * reduction) / 100);
}

int CalculateBonusDamageForSwordMod(
    Character& attacker,
    Character& defender,
    bool isThrust)
{
    assert(attacker.GetMeleeWeapon());
    auto& weapon = *attacker.GetMeleeWeapon();

    // this is a hack because I don't model poison as a modifier
    bool isPoisoned{false};
    int bonusDamage = 0;

    std::optional<Modifier> activeMod{};

    const auto& object = weapon.GetObject();
    int strength = isThrust ? object.mStrengthThrust : object.mStrengthSwing;

    if (weapon.IsPoisoned())
    {
        bonusDamage = 10;
        isPoisoned = true;
    }

    for (auto modifier : weapon.GetModifiers())
    {
        switch (modifier)
        {
            case Modifier::Flaming:
                bonusDamage = (strength * 75) / 100;
                activeMod = Modifier::Flaming;
                break;
            case Modifier::SteelFire:
                bonusDamage = strength;
                activeMod = Modifier::SteelFire;
                break;
            case Modifier::Frost:
                bonusDamage = strength / 2;
                activeMod = Modifier::Frost;
                break;
            case Modifier::Enhancement1:
                bonusDamage = strength * 2;
                activeMod = Modifier::Enhancement1;
                break;
            case Modifier::Enhancement2:
                bonusDamage = (strength * 75) / 100;
                activeMod = Modifier::Enhancement2;
                break;
            default:
                break;
        }
    }

    auto* armor = defender.GetArmor();
    if (armor && activeMod)
    {
        bonusDamage = CalculateArmorModReduction(
            *activeMod,
            armor->GetModifiers(),
            bonusDamage);
    }
    else if (armor && isPoisoned)
    {
        if (armor->IsPoisoned())
        {
            bonusDamage = 0;
        }
    }

    return bonusDamage;
}

int CalculateMeleeDamage(
    Character& attacker,
    Character& defender,
    bool isThrust)
{
    auto* weapon = attacker.GetMeleeWeapon();

    auto totalDamage = attacker.GetSkill(SkillType::Strength);

    if (weapon)
    {
        auto condition = weapon->GetCondition();
        const auto& object = weapon->GetObject();
        int weaponStrength = isThrust ? object.mStrengthThrust : object.mStrengthSwing;
        totalDamage += (weaponStrength * condition) / 100;
    }

    totalDamage += CalculateBonusDamageForSwordMod(attacker, defender, isThrust);

    if (weapon)
    {
        // Guarda Revanche does double damage vs. Moredhel :)
        if (weapon->GetItemIndex() == sGuardaRevanche
            && (defender.GetMonsterIndex() == sMoredhelWarrior
            || defender.GetMonsterIndex() == sMoredhelSpellcaster))
        {
            totalDamage *= 2;
        }
    }

    if (totalDamage < 1)
    {
        totalDamage = 1;
    }

    return totalDamage;
}

std::uint16_t GetMeleeModifierFlags(Character& attacker)
{
    std::uint16_t modifierFlags = 0x0580;
    auto* weapon = attacker.GetMeleeWeapon();
    if (!weapon || weapon->IsItemType(ItemType::Staff))
    {
        return modifierFlags;
    }

    if (weapon->IsPoisoned())
    {
        modifierFlags |= std::to_underlying(ModifierFlags::Poison);
    }

    for (auto mod : weapon->GetModifiers())
    {
        switch (mod)
        {
        case Modifier::Flaming: [[fallthrough]];
        case Modifier::SteelFire:
            modifierFlags |= std::to_underlying(ModifierFlags::Fire);
            break;
        case Modifier::Frost:
            modifierFlags |= std::to_underlying(ModifierFlags::Frost);
            break;
        case Modifier::Blessing1: [[fallthrough]];
        case Modifier::Enhancement1:
            modifierFlags |= std::to_underlying(ModifierFlags::Enhance1);
            break;
        case Modifier::Enhancement2:
            modifierFlags |= std::to_underlying(ModifierFlags::Enhance2);
            break;
            break;
        case Modifier::Blessing2: [[fallthrough]];
        case Modifier::Blessing3:
            modifierFlags |= std::to_underlying(ModifierFlags::Bless);
            break;
        default:
            break;
        }
    }
    return modifierFlags;
}

int CalculateMonsterResistance(MonsterIndex monster, std::uint16_t modifierFlags, int damage)
{
    bool resistsModifier = sMonsterResistanceArr[monster.mValue];
    if (resistsModifier)
    {
        damage /= 2;
    }

    return damage;
}

int CalculateMonsterWeakness(MonsterIndex monster, std::uint16_t modifierFlags, int damage)
{
    bool weakToModifier{false};

    switch (monster.mValue)
    {
    case sBrakNurr.mValue:
        weakToModifier = (modifierFlags & std::to_underlying(ModifierFlags::Poison)) != 0;
        break;
    case sPantathian.mValue:
        weakToModifier = (modifierFlags & std::to_underlying(ModifierFlags::Enhance1)) != 0;
        break;
    case sPanthTiandn.mValue:
        weakToModifier = (modifierFlags & 0x200) != 0;
        break;
    case sServitorOfLimsKragma.mValue:
        weakToModifier = (modifierFlags & std::to_underlying(ModifierFlags::Frost)) != 0;
        break;
    case sRogueMage.mValue:
        weakToModifier = (modifierFlags & 0x100) != 0;
        break;
    case sBulldrakeWyvern.mValue:
        weakToModifier = (modifierFlags & std::to_underlying(ModifierFlags::Frost)) != 0;
        break;
    case sGrandsireWyvern.mValue:
        // 4 + 8 => Fire and ?
        weakToModifier = (modifierFlags & 0xc) != 0;
        break;
    case sHatchlingWyvern.mValue:
        weakToModifier = (modifierFlags & std::to_underlying(ModifierFlags::Enhance1)) != 0;
        break;
    case sTroll.mValue:
        weakToModifier = (modifierFlags & 0x200) != 0;
        break;
    case sNethermander.mValue:
        weakToModifier = (modifierFlags & 0x40) != 0;
        break;
    }

    if (weakToModifier)
    {
        damage += (damage / 2);
    }

    return damage;
}


int CalculateArmorReduction(Character& defender)
{
    auto* armor = defender.GetArmor();
    if (!armor)
    {
        return 0;
    }

    auto reduction = defender.GetSkill(SkillType::Defense) / 4;

    auto condition = armor->GetCondition();
    // I guess I should use a union or something for this...
    reduction += (condition * armor->GetObject().mAccuracySwing) / 100;

    auto raceEffect = CalculateRaceEffect(defender, *armor);
    reduction = reduction * (raceEffect + 100) / 100;

    if (reduction > 98)
    {
        reduction = 98;
    }

    return reduction;
}

unsigned GetAttackSound(MonsterIndex monster, bool hasStaff)
{
    switch (monster.mValue)
    {
        case 19: [[fallthrough]];
        case 28: [[fallthrough]];
        case 41: [[fallthrough]];
        case 42: [[fallthrough]];
        case 43: [[fallthrough]];
        case 46: [[fallthrough]];
        case 48:
            return sAttackSoundFist;
        case 39: [[fallthrough]];
        case 44: [[fallthrough]];
        case 49: [[fallthrough]];
        case 58: 
            return sAttackSoundZap;
        default:
            return hasStaff
                ? sAttackSoundStaff
                : sAttackSoundSword;
    }
}

unsigned GetDefenseSound(bool attackerHasStaff, bool defenderHasStaff)
{
    if (attackerHasStaff && defenderHasStaff)
    {
        return sParryStaffOnStaff;
    }
    else if (attackerHasStaff || defenderHasStaff)
    {
        return sParryStaffOnArmor;
    }
    else
    {
        return sParrySword;
    }
}

void UseCombatItemAndDull(Character& character, ItemType itemType, int factor)
{
    auto& inv = character.GetInventory();
    auto item = inv.FindEquipped(itemType);
    if (item == inv.GetItems().end())
    {
        return;
    }

    if (!item->IsConditionBased())
    {
        // Can't dull staves
        return;
    }

    const auto& object = item->GetObject();

    unsigned willDullRoll = GetRandomNumber(0, 0xfff);
    if ((willDullRoll % 100) >= object.mDullChance)
    {
        return;
    }


    int dullAmount = 1;

    if (object.mMaxDullAmount > 1)
    {
        int dullRoll = GetRandomNumber(0, 0xfff);
        dullAmount = (dullRoll % (object.mMaxDullAmount - 1)) + 1;
    }

    dullAmount = (dullAmount * factor) / 256;
    int newCondition = item->GetCondition() - dullAmount;

    if (item->IsItemType(ItemType::Crossbow))
    {
        int bowBreakCheck = GetRandomNumber(0, 0xfff);
        if ((bowBreakCheck % 50) >= newCondition)
        {
            newCondition = 0;
        }
    }

    item->SetRepairable(true);
    item->SetUsed(true);

    if (newCondition < object.mMinCondition)
    {
        newCondition = object.mMinCondition;
    }

    if (newCondition <= 0)
    {
        newCondition = 0;
        item->SetBroken(true);
    }

    item->SetCondition(newCondition);
}

void PoisonCombatant(Character& combatant, CombatState& state)
{
    auto monsterIx = combatant.GetMonsterIndex().mValue;
    assert(monsterIx < sMonsterResistanceArr.size());

    if ((sMonsterResistanceArr[monsterIx] & std::to_underlying(ModifierFlags::Poison)) != 0)
    {
        return;
    }

    auto* armor = combatant.GetArmor();
    if (armor && armor->IsPoisoned())
    {
        return;
    }

    state.mIsPoisoned = true;

    auto poisonAmount = (GetRandomNumber(0, 0xfff) % 50) + 10;

    combatant.AdjustCondition(BAK::Condition::Poisoned, poisonAmount);
}

[[nodiscard]] bool ApplyPoisonAtEndOfTurn(Character& character, CombatState& combatState)
{
    int damageAmount = GetRandomNumber(1, 2);
    bool useArmor = false;
    int damageType = 2;
    std::uint16_t modifierFlags = std::to_underlying(ModifierFlags::Poison);
    // this is kinda surprising, test this...
    bool ignoreShields = false;

    return DamageCombatant(
        character,
        character.GetMonsterIndex(),
        combatState,
        damageAmount,
        useArmor,
        damageType,
        modifierFlags,
        ignoreShields);
}

[[nodiscard]] bool DamageCombatant(
    Character& character,
    MonsterIndex monster,
    CombatState& combatState,
    int originalDamage,
    bool useArmor,
    unsigned damageType,
    std::uint16_t modifierFlags,
    bool ignoreShields)
{
    auto& skills = character.GetSkills();
    {
        auto health = skills.GetSkill(SkillType::Health);
        auto stamina = skills.GetSkill(SkillType::Stamina);

        Logging::LogDebug(__FUNCTION__) << "Mods: " << std::hex << modifierFlags << std::dec
            << " Before: \nHealth " << health << "\nStamina " << stamina << "\n";
    }

    if (monster == sWindElemental)
    {
        return false;
    }

    if (GetSpellEffect(combatState, sDannonsDelusions))
    {
        return false;
    }

    if (combatState.mIsDead)
    {
        return false;
    }

    if (originalDamage < 1)
    {
        return false;
    }

    auto damage = originalDamage;

    if (useArmor)
    {
        auto reduction = CalculateArmorReduction(character);
        damage = damage * (100 - reduction) / 100;
        if (damage <= 0)
        {
            damage = GetRandomNumber(1, 2);
        }
        Logging::LogDebug(__FUNCTION__) << " After armor ("
            << reduction << "% reduction): " << damage << "\n";
    }

    auto* shieldEffect = GetSpellEffect(combatState, sHochosHaven);
    if (!ignoreShields && shieldEffect)
    {
        shieldEffect->mAmount -= damage;
        damage = 0;

        int shieldLeft = shieldEffect->mAmount;
        if (shieldLeft < 0)
        {
            damage = -shieldLeft;
        }

        if (damage == 0)
        {
            return false;
        }
    }

    if (!ignoreShields)
    {
        auto* skinEffect = GetSpellEffect(combatState, sSkinOfTheDragon);
        if (skinEffect)
        {
            damage = 0;
        }
    }

    Logging::LogDebug(__FUNCTION__) << " After shield/skin: " << damage << "\n";

    damage = CalculateMonsterWeakness(character.GetMonsterIndex(), modifierFlags, damage);
    damage = CalculateMonsterResistance(character.GetMonsterIndex(), modifierFlags, damage);

    Logging::LogDebug(__FUNCTION__) << " After weakness/resistance: " << damage << "\n";

    if (damage > 0 && (modifierFlags & std::to_underlying(ModifierFlags::Poison)))
    {
        PoisonCombatant(character, combatState);
    }

    auto& stamina = character.GetSkills().GetSkill(SkillType::Stamina);
    if (stamina.mTrueSkill < damage)
    {
        auto excessDamage = damage - stamina.mTrueSkill;
        stamina.mTrueSkill = 0;
        auto& health = character.GetSkills().GetSkill(SkillType::Health);

        if (health.mTrueSkill < excessDamage)
        {
            health.mTrueSkill = 0;
        }
        else
        {
            health.mTrueSkill -= excessDamage;
        }
    }
    else
    {
        stamina.mTrueSkill -= damage;
    }

    if (originalDamage > 0 && damageType != 0)
    {
        unsigned displayDamage = 1;
        if (originalDamage < 1000)
        {
            displayDamage = originalDamage;
        }
    }
    else if (damageType != 0)
    {
        unsigned displayDamage = 1;
    }

    if ((modifierFlags & std::to_underlying(ModifierFlags::Fire)) != 0)
    {
    }

    if (modifierFlags & std::to_underlying(ModifierFlags::Frost))
    {
    }

    if (modifierFlags & std::to_underlying(ModifierFlags::Poison))
    {
    }

    auto& health = character.GetSkills().GetSkill(SkillType::Health);
    if (health.mTrueSkill == 0)
    {
        CleanCharacterStateOnDeath(character, combatState);
        combatState.mIsExorcised = false;
    }

    character.GetSkill(SkillType::TotalHealth);

    {
        auto health = skills.GetSkill(SkillType::Health);
        auto stamina = skills.GetSkill(SkillType::Stamina);

        Logging::LogDebug(__FUNCTION__) << " Dead: " << combatState.mIsDead << " Before: \nHealth "
            << health << "\nStamina " << stamina << "\n";
    }

    return combatState.mIsDead;
}

}
