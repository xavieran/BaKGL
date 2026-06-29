#pragma once

#include "bak/types.hpp"

#include <cstdint>
#include <vector>

namespace BAK { enum class Modifier; }

namespace BAK {

class Character;
struct CombatState;
struct SpellEffect;
struct MeleeInfo;
enum class MeleeResult;
enum class ItemType;

static constexpr auto sDullFull = 0x100;
static constexpr auto sDullHalf = 0x80;

enum class ModifierFlags
{
    Poison   = 0x1,
    Frost    = 0x2,
    Fire     = 0x4,
    Enhance1 = 0x10,
    Enhance2 = 0x20,
    Bless    = 0x800,
};

MeleeInfo CalculateMeleeInfo(Character&);
MeleeResult CalculateMeleeResult(
    Character& attacker,
    Character& defender,
    CombatState& defenderState,
    int weaponAccuracy);

int CalculateMeleeDamage(
    Character& attacker,
    Character& defender,
    bool isThrust);

std::uint16_t GetMeleeModifierFlags(Character& attacker);
int CalculateMonsterWeakness(MonsterIndex monster, std::uint16_t modifierFlags, int damage);
int CalculateMonsterResistance(MonsterIndex monster, std::uint16_t modifierFlags, int damage);

int CalculateArmorReduction(Character& defender);

unsigned GetAttackSound(MonsterIndex, bool hasStaff);
unsigned GetDefenseSound(bool attackerHasStaff, bool defenderHasStaff);

void UseCombatItemAndDull(Character& character, ItemType item, int factor);

int CalculateParry(Character& defender, CombatState& state);
int CalculateAccuracyBonus(int raceEffect, int condition, int accuracy);
int CalculateBlessingEffect(int value, std::vector<Modifier> modifiers);
int CalculateArmorModReduction(Modifier modifier, const std::vector<Modifier>& modifiers, int armor);
void PoisonCombatant(Character&, CombatState&);

bool IsCombatantActive(CombatState& combatState, bool checkPending);
bool TickCombatEffects(CombatState& combatState);
SpellEffect* GetSpellEffect(CombatState& combatState, unsigned effectId);
void RemoveSpellEffect(CombatState& combatState, unsigned effectId);

}
