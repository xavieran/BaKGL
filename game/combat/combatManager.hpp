#pragma once

#include "game/combat/grid.hpp"
#include "game/combat/gridAlgorithms.hpp"
#include "game/combat/ICombatStage.hpp"

#include "bak/combat/ICombatManager.hpp"
#include "bak/combat/ICombatUI.hpp"

#include "bak/character.hpp"
#include "bak/combat/mechanics.hpp"
#include "bak/coordinates.hpp"

#include "com/bits.hpp"
#include "com/logger.hpp"

#include <cassert>
#include <vector>

namespace Game::Combat {

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

enum class StateFlags
{
    Reachable     = 0,
    Attackable    = 1,
    LOSAttackable = 2,
    HasZap        = 3,
    HasMine       = 4,
    HasCrystal    = 5
};

struct GridElem
{
    BAK::GamePositionAndHeading mPos;
    std::uint16_t mState;
    Combatant* mElement{nullptr};
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
    CombatManager(ICombatStage& stage, BAK::ICombatUI& ui);

    void SetCastingSpell(BAK::SpellIndex) override;
    void SetUsingCrossbow() override;

    void AddCombatant(Combatant combatant);

    void BeginCombat();

    void GridCellClicked(glm::uvec2 targetGrid);

    void EndCombat();

private:
    Combatant* GetCombatant(BAK::CharIndex character);
    Combatant* GetCombatant(BAK::EntityIndex entityIndex);
    Combatant* GetCombatant(glm::uvec2 gridPos);

    std::vector<Combatant> GetCombatants();

    void SetCurrentCombatant(bool onlyParty);
    Combatant& GetCurrentCombatant();

    void ComputeGrid();

    void PrintGridState();

    bool CanMoveTo(const Combatant& combatant, glm::uvec2 target) const;
    bool CanAttack(const Combatant& combatant, glm::uvec2 target) const;

    void CompleteMove(BAK::EntityIndex entityIndex, glm::uvec2 target);

    void FinishTurn();

    void ClearGrid();

    std::vector<Combatant> mCombatants{};

    unsigned mCurrentCombatant{0};
    Grid<GridElem> mGrid;
    ICombatStage& mStage;
    BAK::ICombatUI& mCombatUI;
    const Logging::Logger& mLogger;
};

}

