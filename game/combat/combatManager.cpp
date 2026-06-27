#include "game/combat/combatManager.hpp"

#include "game/combat/gridAlgorithms.hpp"

#include "bak/combat/calculations.hpp"

#include "com/bits.hpp"
#include "com/visit.hpp"
#include "com/ostream.hpp"

#include <algorithm>
#include <optional>

namespace Game::Combat {

CombatManager::CombatManager(ICombatStage& stage, BAK::ICombatUI& ui)
:
    mStage{stage},
    mCombatUI{ui},
    mLogger{Logging::LogState::GetLogger("Game::CombatManager")}
{}

void CombatManager::SetCastingSpell(BAK::SpellIndex) {}

void CombatManager::SetUsingCrossbow() {}

void CombatManager::AddCombatant(Combatant combatant)
{
    mCombatants.emplace_back(combatant);
}

Combatant* CombatManager::GetCombatant(BAK::CharIndex character)
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

Combatant* CombatManager::GetCombatant(BAK::EntityIndex entityIndex)
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

Combatant* CombatManager::GetCombatant(GridPos gridPos)
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

std::vector<Combatant> CombatManager::GetCombatants()
{
    return mCombatants;
}

void CombatManager::BeginCombat()
{
    mCombatActive = true;
    SetCurrentCombatant(true);
}

Combatant& CombatManager::GetCurrentCombatant()
{
    assert(mCurrentCombatant < mCombatants.size());
    return mCombatants[mCurrentCombatant];
}

void CombatManager::GridCellClicked(GridPos targetCell, bool isRightClick)
{
    auto& combatant = GetCurrentCombatant();
    mLogger.Debug() << "Cell clicked: " << targetCell
        << " " << GetCurrentCombatant().mCharacter->GetName()
        << " eid: " << combatant.mEntityIndex << "\n";

    auto myPos = combatant.mGridPos;

    if (mGrid.CanAttack(myPos, targetCell))
    {
        auto moveTo = SelectBestAttackPosition(myPos, targetCell, mGrid);
        mLogger.Debug() << "Attacking: " << targetCell << " should move to: " << moveTo << "\n";
        if (!moveTo)
        {
            return;
        }
        else if (*moveTo != myPos)
        {
            // If we had to move to attack then we can't slash
            if (isRightClick)
            {
                return;
            }

            auto moves = CalculatePath(myPos, *moveTo, mGrid);
            if (moves.empty())
            {
                return;
            }
            for (auto move : moves) mActions.Push(Move{move});
        }

        auto attackType = isRightClick ? AttackType::Slash : AttackType::Thrust;

        mActions.Push(Attack(targetCell, attackType));
        mLogger.Debug() << "Queued Actions: " << mActions << "\n";

        ExecuteAction();

        return;
    }

    if (!mGrid.CanMoveTo(targetCell))
    {
        return;
    }

    auto moves = CalculatePath(combatant.mGridPos, targetCell, mGrid);
    mLogger.Debug() << "Path to target: " << moves << "\n";
    if (moves.empty())
    {
        return;
    }

    for (auto move : moves) mActions.Push(Move{move});
    ExecuteAction();
}

void CombatManager::EndCombat()
{
    mCombatActive = false;
    mCombatants.clear();
    ClearGrid();
}

void CombatManager::OnHoverChanged(std::optional<GridPos> gridPos)
{
    mLogger.Debug() << "Hover changed to: " << gridPos << "\n";
    auto* hovered = gridPos ? GetCombatant(*gridPos) : nullptr;
    if (hovered
        && GetCurrentCombatant().mCharacter->IsEnemy() != hovered->mCharacter->IsEnemy())
    {
        auto meleeInfo = BAK::CalculateMeleeInfo(*GetCurrentCombatant().mCharacter);
        if (!IsAdjacent(GetCurrentCombatant().mGridPos, hovered->mGridPos))
        {
            meleeInfo.mSlashChance = 0;
            meleeInfo.mSlashDamage = 0;
        }
        mCombatUI.DisplayMeleeInfo(meleeInfo);
    }
    else
    {
        mCombatUI.ResetDisplay();
    }
}

bool CombatManager::IsCombatActive() const
{
    return mCombatActive;
}

void CombatManager::SetCurrentCombatant(bool onlyParty)
{
    std::optional<unsigned> bestSpeed{};
    auto combatantIndex = 0;
    for (unsigned i = 0; i < mCombatants.size(); i++)
    {
        auto& combatant = mCombatants[i];
        auto* character = combatant.mCharacter;
        if (!character)
        {
            continue;
        }

        if (onlyParty && character->IsEnemy())
        {
            continue;
        }

        if (!combatant.mTurnPending)
        {
            continue;
        }

        auto speed = character->GetSkill(BAK::SkillType::Speed);

        if (!bestSpeed || speed > *bestSpeed)
        {
            bestSpeed = speed;
            combatantIndex = i;
        }
    }

    mCurrentCombatant = combatantIndex;
    auto& character = *GetCurrentCombatant().mCharacter;
    if (!character.IsEnemy())
    {
        mCombatUI.SetSelectedCharacter(character.GetIndex());
    }

    ComputeGrid();

    mLogger.Debug() << "Current combatant set to: " << combatantIndex << " " << GetCurrentCombatant().mCharacter->GetName()
        << " charIndex: " << character.GetIndex().mValue << "\n";
}

void CombatManager::ComputeGrid()
{
    ClearGrid();

    auto& me = GetCurrentCombatant();
    auto speed = me.mCharacter->GetSkill(BAK::SkillType::Speed);

    for (unsigned x = 0; x < mGrid.GetCols(); x++)
    {
        for (unsigned y = 0; y < mGrid.GetRows(); y++)
        {
            auto& cell = mGrid.Get(x, y);
            cell.mState = SetBit(cell.mState, StateFlags::Reachable, true);
        }
    }
  
    for (auto& combatant : mCombatants)
    {
        auto& cell = mGrid.Get(combatant.mGridPos);
        cell.mElement = &combatant;
        if (!combatant.mIsDead)
        {
            cell.mState = SetBit(cell.mState, StateFlags::Reachable, false);
        }

        if (!combatant.mIsDead)
        {
            auto isEnemy = me.mCharacter->IsEnemy() != combatant.mCharacter->IsEnemy();
            auto state = isEnemy ? StateFlags::Attackable : StateFlags::IsAlly;
            cell.mState = SetBit(cell.mState, state, true);
        }
    }

    for (unsigned x = 0; x < mGrid.GetCols(); x++)
    {
        for (unsigned y = 0; y < mGrid.GetRows(); y++)
        {
            auto cellPos = GridPos{static_cast<int>(x), static_cast<int>(y)};
            if (!mGrid.CanMoveTo(cellPos))
            {
                continue;
            }

            auto moves = CalculatePath(me.mGridPos, cellPos, mGrid);
            if (moves.size() > speed)
            {
                auto& cell = mGrid.Get(cellPos);
                cell.mState = SetBit(cell.mState, StateFlags::Reachable, false);
            }
        }
    }

    mLogger.Debug() << "GridState: \n" << mGrid << "\n";
}

void CombatManager::CompleteMove(GridPos target)
{
    auto& combatant = GetCurrentCombatant();

    auto& oldCell = mGrid.Get(combatant.mGridPos.x, combatant.mGridPos.y);
    oldCell.mElement = nullptr;

    combatant.mGridPos = target;

    auto& newCell = mGrid.Get(target.x, target.y);
    newCell.mElement = &combatant;

    if (mActions.HasAction())
    {
        ExecuteAction();
    }
    else
    {
        FinishTurn();
    }
}

void CombatManager::CompleteAttack(GridPos target)
{
    auto& combatant = GetCurrentCombatant();

    auto& enemy = mGrid.Get(target);

    mStage.SetCombatantAction(enemy.mElement->mEntityIndex, BAK::AnimationType::Idle);
    mStage.SetCombatantAction(combatant.mEntityIndex, BAK::AnimationType::Idle);

    FinishTurn();
}

void CombatManager::ExecuteAction()
{
    assert(mActions.HasAction());

    auto action = mActions.Pop();
    std::visit(
        overloaded{
            [&](const Move& move){
                Execute(move);
            },
            [&](const Attack& attack){
                Execute(attack);
            }},
        action);
}

void CombatManager::Execute(const Move& move)
{
    mLogger.Debug() << "Execute: " << move << "\n";
    auto entityIndex = GetCurrentCombatant().mEntityIndex;
    auto sourceGrid = GetCurrentCombatant().mGridPos;
    mStage.MoveCombatant(entityIndex, sourceGrid, move.mTarget);
}

void CombatManager::Execute(const Attack& attack)
{
    mLogger.Debug() << "Execute: " << attack << "\n";

    auto& me = GetCurrentCombatant();
    auto* target = GetCombatant(attack.mTarget);
    assert(target);
    auto defenseDirection = BAK::GetDirectionBetween(attack.mTarget, me.mGridPos);
    mStage.SetCombatantDirection(target->mEntityIndex, defenseDirection);
    auto defense = attack.mType == AttackType::Slash
        ? BAK::AnimationType::ParryHigh
        : BAK::AnimationType::ParryLow;
    mStage.SetCombatantAction(target->mEntityIndex, defense);
    mStage.AnimateCombatant(target->mEntityIndex);

    auto entityIndex = me.mEntityIndex;
    auto sourceGrid = me.mGridPos;
    auto attackDirection = BAK::GetDirectionBetween(me.mGridPos, attack.mTarget);
    auto attackAnim = attack.mType == AttackType::Slash
        ? BAK::AnimationType::Slash
        : BAK::AnimationType::Thrust;
    mStage.SetCombatantDirection(entityIndex, attackDirection);
    mStage.SetCombatantAction(entityIndex, attackAnim);
    mStage.AnimateAttack(entityIndex, attack.mTarget);
}

void CombatManager::FinishTurn()
{
    auto& combatant = GetCurrentCombatant();
    combatant.mTurnPending = false;

    auto it = std::find_if(mCombatants.begin(), mCombatants.end(), [](auto& c){ return c.mTurnPending; });
    if (it == mCombatants.end())
    {
        for (auto& combatant : mCombatants)
        {
            combatant.mTurnPending = true;
        }
    }

    SetCurrentCombatant(false);
}

void CombatManager::ClearGrid()
{
    for (unsigned y = 0; y < mGrid.GetRows(); y++)
    {
        for (unsigned x = 0; x < mGrid.GetCols(); x++)
        {
            auto& gridCell = mGrid.Get(x, y);
            gridCell.mElement = nullptr;
            gridCell.mState = 0;
        }
    }
}

}
