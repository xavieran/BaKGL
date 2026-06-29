#pragma once

namespace BAK {

class Character;
struct CombatState;
struct SpellEffect;

[[nodiscard]] bool TickCombatEffects(CombatState& combatState);
SpellEffect* GetSpellEffect(CombatState& combatState, unsigned effectId);
void RemoveSpellEffect(CombatState& combatState, unsigned effectId);
void CleanCharacterStateOnDeath(Character& character, CombatState& combatState);

}
