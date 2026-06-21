#include "game/combat/combatManager.hpp"

#include <algorithm>
#include <optional>
#include <sstream>

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

Combatant* CombatManager::GetCombatant(glm::uvec2 gridPos)
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
    SetCurrentCombatant(true);
}

Combatant& CombatManager::GetCurrentCombatant()
{
    assert(mCurrentCombatant < mCombatants.size());
    return mCombatants[mCurrentCombatant];
}

void CombatManager::GridCellClicked(glm::uvec2 targetGrid)
{
    auto& combatant = GetCurrentCombatant();
    mLogger.Debug() << "Moving combatant: " << mCurrentCombatant 
        << " " << GetCurrentCombatant().mCharacter->GetName()
        << " eid: " << combatant.mEntityIndex << "\n";

    if (CanAttack(combatant, targetGrid))
    {
        auto* target = GetCombatant(targetGrid);
        assert(target);
        mStage.SetCombatantAction(target->mEntityIndex, BAK::AnimationType::ParryHigh);
        mStage.AnimateCombatant(target->mEntityIndex, []{});

        auto entityIndex = combatant.mEntityIndex;
        auto sourceGrid = combatant.mGridPos;
        mStage.SetCombatantAction(entityIndex, BAK::AnimationType::Slash);
        mStage.AnimateCombatant(entityIndex, [this]{ FinishTurn(); });
    }

    if (!CanMoveTo(combatant, targetGrid))
    {
        return;
    }

    auto entityIndex = combatant.mEntityIndex;
    auto sourceGrid = combatant.mGridPos;
    mStage.MoveCombatant(entityIndex, sourceGrid, targetGrid,
        [this, entityIndex, targetGrid]()
        {
            CompleteMove(entityIndex, targetGrid);
        });
}

void CombatManager::EndCombat()
{
    mCombatants.clear();
    ClearGrid();
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
            auto distance = ChebyshevDistance(me.mGridPos, glm::uvec2{x, y});
            if (distance <= speed)
            {
                cell.mState = SetBit(cell.mState, StateFlags::Reachable, true);
            }
        }
    }

    for (auto& combatant : mCombatants)
    {
        auto& cell = mGrid.Get(combatant.mGridPos.x, combatant.mGridPos.y);
        cell.mElement = &combatant;
        if (!combatant.mIsDead)
        {
            cell.mState = SetBit(cell.mState, StateFlags::Reachable, false);
        }

        if (me.mCharacter->IsEnemy() != combatant.mCharacter->IsEnemy())
        {
            if (!combatant.mIsDead)
            {
                cell.mState = SetBit(cell.mState, StateFlags::Attackable, true);
            }
        }
    }

    PrintGridState();
}

void CombatManager::PrintGridState()
{
    std::stringstream ss{};
    ss << std::hex;
    for (unsigned _y = mGrid.GetRows(); _y > 0; _y--)
    {
        auto y = _y - 1;
        for (unsigned x = 0; x < mGrid.GetCols(); x++)
        {
            auto& cell = mGrid.Get(x, y);
            ss << cell.mState << " ";
        }
        ss << "\n";
    }
    mLogger.Debug() << "GridState: \n" << ss.str() << "\n";
}

bool CombatManager::CanMoveTo(const Combatant& combatant, glm::uvec2 target) const
{
    if (!mGrid.WithinBounds(target.x, target.y)) return false;
    auto& cell = mGrid.Get(target.x, target.y);
    return CheckBitSet(cell.mState, StateFlags::Reachable);
}

bool CombatManager::CanAttack(const Combatant& combatant, glm::uvec2 target) const
{
    if (!mGrid.WithinBounds(target.x, target.y)) return false;
    auto& cell = mGrid.Get(target.x, target.y);
    return CheckBitSet(cell.mState, StateFlags::Attackable);
}

void CombatManager::CompleteMove(BAK::EntityIndex entityIndex, glm::uvec2 target)
{
    auto* combatant = GetCombatant(entityIndex);
    if (!combatant) return;

    auto& oldCell = mGrid.Get(combatant->mGridPos.x, combatant->mGridPos.y);
    oldCell.mElement = nullptr;

    combatant->mGridPos = target;

    auto& newCell = mGrid.Get(target.x, target.y);
    newCell.mElement = combatant;

    FinishTurn();
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
