#pragma once

#include "game/combat/grid.hpp"
#include "game/combat/actionQueue.hpp"
#include "game/combat/types.hpp"
#include "game/combat/ICombatStage.hpp"

#include "bak/combat/ICombatManager.hpp"
#include "bak/combat/ICombatUI.hpp"

#include "com/logger.hpp"

#include <cassert>
#include <optional>
#include <vector>

namespace Game::Combat {
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
    CombatManager(ICombatStage& stage, BAK::ICombatUI& ui);

    void SetCastingSpell(BAK::SpellIndex) override;
    void SetUsingCrossbow() override;
    void DoDefend() override;
    void DoRest() override;

    void AddCombatant(Combatant combatant);

    void BeginCombat();

    void GridCellClicked(GridPos targetGrid, bool isRightClick);

    void EndCombat();

    void CompleteMove(GridPos target);
    void CompleteAttack(GridPos target);

    void OnHoverChanged(std::optional<GridPos> gridPos);
    bool IsCombatActive() const;

private:
    Combatant* GetCombatant(BAK::CharIndex character);
    Combatant* GetCombatant(BAK::EntityIndex entityIndex);
    Combatant* GetCombatant(GridPos gridPos);

    std::vector<Combatant> GetCombatants();

    Combatant& GetCurrentCombatant();

    void ComputeGrid();

    bool CanMoveTo(const Combatant& combatant, GridPos target) const;
    bool CanAttack(const Combatant& combatant, GridPos target) const;

    void ExecuteAction();
    void Execute(const Move&);
    void Execute(const Attack&);
    void Execute(const AnimateDeath& death);

    void FinishTurn();
    void StartNextTurn();
    std::optional<BAK::CombatResult> CheckCombatFinished();
    void ResurrectNighthawks();
    void TickCombatEffectsAtEndOfTurn();
    void SetCurrentCombatant(unsigned index);

    std::vector<Combatant>::iterator SelectNextCombatantForTurn(bool onlyPlayer);

    void ClearGrid();

    std::vector<Combatant> mCombatants{};

    unsigned mCurrentCombatant{0};
    ActionQueue mActions{};
    Grid mGrid{8, 13};
    ICombatStage& mStage;
    BAK::ICombatUI& mCombatUI;
    const Logging::Logger& mLogger;
    bool mCombatActive{false};
};

}

