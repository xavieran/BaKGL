#include "game/combat/combatManager.hpp"

#include "audio/audio.hpp"

#include "game/combat/gridAlgorithms.hpp"

#include "bak/combat/calculations.hpp"
#include "bak/sounds.hpp"

#include "com/bits.hpp"
#include "com/ostream.hpp"
#include "com/random.hpp"
#include "com/visit.hpp"

#include <algorithm>
#include <optional>
#include <utility>

namespace Game::Combat {

CombatManager::CombatManager(ICombatStage& stage, BAK::ICombatUI& ui)
:
    mStage{stage},
    mCombatUI{ui},
    mLogger{Logging::LogState::GetLogger("Game::CombatManager")}
{}

void CombatManager::SetCastingSpell(BAK::SpellIndex) {}

void CombatManager::SetUsingCrossbow() {}

void CombatManager::DoDefend()
{
    GetCurrentCombatant().IsDefending() = true;
    FinishTurn();
}

void CombatManager::DoRest()
{
    FinishTurn();
}

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
    assert(combatant.mCharacter);

    mLogger.Debug() << "Cell clicked: " << targetCell
        << " " << GetCurrentCombatant().mCharacter->GetName()
        << " eid: " << combatant.mEntityIndex << "\n";

    auto myPos = combatant.mGridPos;

    bool isSlash = isRightClick;

    if (isSlash && combatant.mCharacter->GetSkill(BAK::SkillType::TotalHealth) <= 1)
    {
        // Slash consumes 1 stamina per hit.
        return;
    }

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
            if (isSlash)
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

        auto attackType = isRightClick ? BAK::AttackType::Slash : BAK::AttackType::Thrust;

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

    if (gridPos)
    {
        auto& me = GetCurrentCombatant();
        auto lookDirection = BAK::GetDirectionBetween(me.mGridPos, *gridPos);
        mStage.SetCombatantDirection(me.mEntityIndex, lookDirection);
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

        if (!combatant.GetTurnPending())
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
        if (!combatant.IsDead())
        {
            cell.mState = SetBit(cell.mState, StateFlags::Reachable, false);
        }

        if (!combatant.IsDead())
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
    assert(me.mCharacter);
    assert(GetCombatant(attack.mTarget));
    auto& target = *GetCombatant(attack.mTarget);
    assert(target.mCharacter);

    me.mCharacter->ImproveSkill(BAK::SkillType::Melee, BAK::SkillChange::FractionOfSkill, 3);
    target.mCharacter->ImproveSkill(BAK::SkillType::Defense, BAK::SkillChange::FractionOfSkill, 3);

    auto* weapon = me.mCharacter->GetMeleeWeapon();
    assert(weapon);

    auto accuracy = attack.mType == BAK::AttackType::Thrust
        ? weapon->GetObject().mAccuracyThrust
        : weapon->GetObject().mAccuracySwing;

    auto attackResult = BAK::CalculateMeleeResult(
        *me.mCharacter,
        *target.mCharacter,
        target.mCombatState,
        accuracy);

    unsigned sound = 0;

    auto attackerHasStaff = weapon && weapon->GetObject().mType == BAK::ItemType::Staff;
    auto targetIsDefending = target.IsDefending();
    bool isThrust = attack.mType == BAK::AttackType::Thrust;
    auto swordDullFactor = isThrust ? BAK::sDullHalf : BAK::sDullFull;

    if (!isThrust)
    {
        //damageCombatant(target, 1, false, false, true, false);
        // attacker.StateFlags &= (~0x40) -- not sure what 0x40 is used for yet
    }

    if (attackResult == BAK::MeleeResult::Hit)
    {
        me.mCharacter->ImproveSkill(BAK::SkillType::Melee, BAK::SkillChange::FractionOfSkill, 3);
        me.mCharacter->ImproveSkill(BAK::SkillType::Strength, BAK::SkillChange::FractionOfSkill, 3);
        if (!attackerHasStaff)
        {
            BAK::UseCombatItemAndDull(*me.mCharacter, BAK::ItemType::Sword, swordDullFactor);
        }

        BAK::UseCombatItemAndDull(*target.mCharacter, BAK::ItemType::Armor, BAK::sDullFull);

        sound = BAK::GetAttackSound(me.mMonster, attackerHasStaff);
        auto damage = BAK::CalculateMeleeDamage(*me.mCharacter, *target.mCharacter, isThrust);
        bool useArmor = true;
        bool damageTypeMelee = true;
        auto modifierFlags = BAK::GetMeleeModifierFlags(*me.mCharacter);
        bool skipDirectDamage = false;
        //damageCombatant(target, damage, useArmor, damageTypeMelee, modifierFlags, skipDirectDamage);
    }
    else
    {
        target.mCharacter->ImproveSkill(BAK::SkillType::Defense, BAK::SkillChange::FractionOfSkill, 3);

        if (targetIsDefending) // && isActive
        {
            target.IsDefending() = false; // true??

            auto* defenderWeapon = target.mCharacter->GetMeleeWeapon();
            auto defenderHasStaff = defenderWeapon && defenderWeapon->GetObject().mType == BAK::ItemType::Staff;
            sound = BAK::GetDefenseSound(attackerHasStaff, defenderHasStaff);

            target.mCharacter->ImproveSkill(BAK::SkillType::Defense, BAK::SkillChange::FractionOfSkill, 3);
        }
        else
        {
            // I don't understand why we would improve defense more if the target wasn't defending,
            // but this is what the game does. It also makes no sense to dull the attacker's weapon
            // if they missed, right? I wonder if these paths are flipped?
            target.mCharacter->ImproveSkill(BAK::SkillType::Defense, BAK::SkillChange::FractionOfSkill, 3);
            if (!attackerHasStaff)
            {

                BAK::UseCombatItemAndDull(*me.mCharacter, BAK::ItemType::Sword, swordDullFactor);
            }
            // play sound 25 -> doesn't exist?
        }

        // TODO: for some reason miss sound thrust is missing in my files.
        //sound = isThrust ? BAK::sMissSoundThrust : BAK::sMissSoundSwing;
        sound = BAK::sMissSoundSwing;
    }

    if (sound != 0)
    {
        AudioA::GetAudioManager().PlaySound(AudioA::SoundIndex{sound});
    }

    auto defenseDirection = BAK::GetDirectionBetween(attack.mTarget, me.mGridPos);
    mStage.SetCombatantDirection(target.mEntityIndex, defenseDirection);

    if (targetIsDefending)
    {
        auto defense = isThrust 
            ? BAK::AnimationType::ParryLow
            : BAK::AnimationType::ParryHigh;
        mStage.SetCombatantAction(target.mEntityIndex, defense);
        mStage.AnimateCombatant(target.mEntityIndex);
    }

    auto entityIndex = me.mEntityIndex;
    auto sourceGrid = me.mGridPos;
    auto attackDirection = BAK::GetDirectionBetween(me.mGridPos, attack.mTarget);
    auto attackAnim = isThrust
        ? BAK::AnimationType::Thrust
        : BAK::AnimationType::Slash;
    mStage.SetCombatantDirection(entityIndex, attackDirection);
    mStage.SetCombatantAction(entityIndex, attackAnim);
    mStage.AnimateAttack(entityIndex, attack.mTarget);
}

void CombatManager::FinishTurn()
{
    auto& combatant = GetCurrentCombatant();
    combatant.GetTurnPending() = false;

    // TODO: Current combatant should always end their turn facing the nearest enemy/their target
    // Unless they are fleeing or dead...
    //if (!combatant.IsDead())
    //{
    //    auto nearestEnemy = 
    //    auto lookDirection = BAK::GetDirectionBetween(me.mGridPos, *gridPos);
    //    mStage.SetCombatantDirection(me.mEntityIndex, lookDirection);
    //}


    auto it = std::find_if(mCombatants.begin(), mCombatants.end(), [](auto& c){ return c.GetTurnPending(); });
    if (it == mCombatants.end())
    {
        for (auto& combatant : mCombatants)
        {
            combatant.GetTurnPending() = true;
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

void CombatManager::DamageCombatant(
    Combatant& victim,
    int damage,
    bool useArmor,
    bool damageTypeMelee,
    std::uint16_t modifierFlags,
    bool skipDirectDamage
)
{
    assert(victim.mCharacter);

    if (useArmor)
    {
        auto reduction = BAK::CalculateArmorReduction(*victim.mCharacter);
        damage = damage * (100 - reduction) / 100;
        if (damage <= 0)
        {
            damage = GetRandomNumber(1, 2);
        }
    }

    if (!skipDirectDamage)
    {
    }

    damage = BAK::CalculateMonsterWeakness(victim.mCharacter->GetMonsterIndex(), damage, modifierFlags);
    damage = BAK::CalculateMonsterResistance(victim.mCharacter->GetMonsterIndex(), damage, modifierFlags);

    if (damage > 0 && (modifierFlags & std::to_underlying(BAK::ModifierFlags::Poison)))
    {
        //PoisonCombatant(victim);
    }
    // ...
}

}
