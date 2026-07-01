#include "game/combat/combatManager.hpp"


#include "game/combat/gridAlgorithms.hpp"

#include "audio/audio.hpp"

#include "bak/combat/types.hpp"
#include "bak/dialogSources.hpp"
#include "bak/monster.hpp"

#include "bak/combat/calculations.hpp"
#include "bak/combat/spellEffects.hpp"
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

    auto it = SelectNextCombatantForTurn(true);
    assert(it != mCombatants.end());

    auto index = std::distance(mCombatants.begin(), it);
    SetCurrentCombatant(index);

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
    mLogger.Spam() << "Hover changed to: " << gridPos << "\n";
    auto* hovered = gridPos ? GetCombatant(*gridPos) : nullptr;
    assert(!hovered || hovered->mCharacter);
    auto& me = GetCurrentCombatant();
    assert(me.mCharacter);

    if (hovered
        && me.mCharacter->IsEnemy() != hovered->mCharacter->IsEnemy())
    {
        BAK::MeleeInfo meleeInfo{
            me.mCharacter->GetSkill(BAK::SkillType::Melee),
            me.mCharacter->GetSkill(BAK::SkillType::Strength),
            0,
            0};

        if (me.mCharacter->GetMeleeWeapon() != nullptr)
        {
            meleeInfo = BAK::CalculateMeleeInfo(*GetCurrentCombatant().mCharacter);
        }

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
    if (mPendingText)
    {
        if (mPendingText->mSound != 0)
        {
            AudioA::GetAudioManager().PlaySound(
                AudioA::SoundIndex{mPendingText->mSound});
        }

        mStage.DisplayText(
            mPendingText->mTarget,
            mPendingText->mText,
            mPendingText->mColor);
        mPendingText.reset();
    }

    auto& combatant = GetCurrentCombatant();
    mStage.SetCombatantAction(combatant.mEntityIndex, BAK::AnimationType::Idle);

    auto& enemy = mGrid.Get(target);
    assert(enemy.mElement);

    if (enemy.mElement->IsDead())
    {
        mActions.Push(AnimateDeath{target});
        ExecuteAction();
    }
    else
    {
        mStage.SetCombatantAction(enemy.mElement->mEntityIndex, BAK::AnimationType::Idle);
        FinishTurn();
    }
}

void CombatManager::ExecuteAction()
{
    assert(mActions.HasAction());

    auto action = mActions.Pop();
    std::visit(
        overloaded{
            [&](const auto& action){
                Execute(action);
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

    auto& me = GetCurrentCombatant();
    assert(me.mCharacter);
    assert(GetCombatant(attack.mTarget));
    auto& target = *GetCombatant(attack.mTarget);
    assert(target.mCharacter);

    //mLogger.Debug() << "Execute: " << attack << " Me: " << *me.mCharacter
    //    << "\nThem: " << *target.mCharacter << "\n";

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
        int damage = 1;
        bool useArmor = false;
        int damageTypeMelee = 0;
        std::uint16_t modifierFlags = 1;
        bool ignoreShields = false;
        bool died = BAK::DamageCombatant(
            *target.mCharacter,
            target.mMonster,
            target.mCombatState,
            damage,
            useArmor,
            damageTypeMelee,
            modifierFlags,
            ignoreShields);
        assert(!died);
        // attacker.StateFlags &= (~0x40) // i.e. don't display the hit effect
    }

    bool targetDied = false;

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

        mPendingText = PendingTextInfo{
            target.mEntityIndex,
            std::to_string(damage),
            sHitTextColor};

        bool useArmor = true;
        bool damageTypeMelee = true;
        auto modifierFlags = BAK::GetMeleeModifierFlags(*me.mCharacter);
        bool ignoreShields = false;
        targetDied = BAK::DamageCombatant(
            *target.mCharacter,
            target.mMonster,
            target.mCombatState,
            damage,
            useArmor,
            damageTypeMelee,
            modifierFlags,
            ignoreShields);
    }
    else
    {
        mPendingText = PendingTextInfo{
            target.mEntityIndex,
            "miss",
            sMissTextColor};

        target.mCharacter->ImproveSkill(BAK::SkillType::Defense, BAK::SkillChange::FractionOfSkill, 3);

        if (targetIsDefending && BAK::IsCombatantActive(target.mCombatState, false))
        {
            // need to check if we reset this or not, I don't thikn we do
            //target.IsDefending() = false; // true??

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
        mPendingText->mSound = sound;
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

void CombatManager::Execute(const AnimateDeath& death)
{
    auto& cell = mGrid.Get(death.mTarget);
    assert(cell.mElement);
    auto& target = *cell.mElement;
    mStage.SetCombatantUpdateIdle(target.mEntityIndex, false);
    mStage.SetCombatantAction(target.mEntityIndex, BAK::AnimationType::Dead);
    mStage.AnimateCombatant(
        target.mEntityIndex,
        [this]{ FinishTurn(); });
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

    if (!combatant.IsDead() && combatant.IsPoisoned())
    {
        bool died = BAK::ApplyPoisonAtEndOfTurn(*combatant.mCharacter, combatant.mCombatState);
        if (died)
        {
            mActions.Push(AnimateDeath{combatant.mGridPos});
            ExecuteAction();
        }
    }

    if (auto result = CheckCombatFinished())
    {
        mLogger.Debug() << "Combat finished: " << ToString(result->mOutcome) << "\n";
        //Cleanup();
        mStage.CombatFinished(*result);
        return;
    }

    auto it = SelectNextCombatantForTurn(false);

    if (it == mCombatants.end())
    {
        mLogger.Debug() << "Turn completed\n";
        StartNextTurn();
    }

    it = SelectNextCombatantForTurn(false);
    auto index = std::distance(mCombatants.begin(), it);
    SetCurrentCombatant(index);
}

void CombatManager::SetCurrentCombatant(unsigned index)
{
    mCurrentCombatant = index;
    auto& character = *GetCurrentCombatant().mCharacter;
    if (!character.IsEnemy())
    {
        mCombatUI.SetSelectedCharacter(character.GetIndex());
    }

    ComputeGrid();

    mLogger.Debug() << "Current combatant set to: " << index << " " << GetCurrentCombatant().mCharacter->GetName()
        << " charIndex: " << character.GetIndex().mValue << "\n";
}

std::optional<BAK::CombatResult> CombatManager::CheckCombatFinished()
{
    bool anyAIAlive = false;
    bool anyPCAlive = false;
    bool anyGhosts = false;
    unsigned enemyCount = 0;
    for (const auto& combatant : mCombatants)
    {
        if (!combatant.mCharacter)
        {
            continue;
        }

        if (!combatant.mCharacter->IsEnemy() && !anyPCAlive)
        {
            anyPCAlive = !combatant.IsDead();
        }

        if (combatant.mCharacter->IsEnemy() && !anyAIAlive)
        {
            anyAIAlive = !combatant.IsDead();
            if (combatant.IsDead())
            {
                enemyCount++;
            }
        }
    }

    if (!anyPCAlive)
    {
        return BAK::CombatResult{BAK::CombatOutcome::Dead};
    }

    if (anyAIAlive)
    {
        return std::nullopt;
    }

    auto result = BAK::CombatResult{BAK::CombatOutcome::Won};
    if (enemyCount > 1)
    {
        if (anyGhosts)
        {
            result.mDialog = BAK::DialogSources::mWonVersusGhosts;
        }
        else if (BAK::IsSpecialBattle(BAK::CombatIndex{0})) // mCombatIndex
        {
            result.mDialog = BAK::DialogSources::mWonSpecialBattle;
        }
        else
        {
            result.mDialog = BAK::DialogSources::mWonBattle;
        }

        return result;
    }

    if (anyGhosts)
    {
        result.mDialog = BAK::DialogSources::mWonVersusGhost;
    }
    else
    {
        result.mDialog = BAK::DialogSources::mDefeatedOneEnemy;
    }

    // FIXME THESE
    // BAK::DialogSources::mSolvedTrap
    // else if (NoCombatantsRemaining)
    // BAK::DialogSources::mEnemyFled
    return result;
}

void CombatManager::StartNextTurn()
{
    for (auto& combatant : mCombatants)
    {
        if (combatant.IsDead() || combatant.IsExorcised())
        {
            continue;
        }

        combatant.GetTurnPending() = true;
        //combatant.Get(bit 4) = false
        assert(combatant.mCharacter);
        auto character = *combatant.mCharacter;
        auto& speed = character.GetSkills().GetSkill(BAK::SkillType::Speed);
        speed.mTrueSkill = speed.mMax;
        character.GetSkill(BAK::SkillType::Health);
        character.GetSkill(BAK::SkillType::Stamina);
        character.GetSkill(BAK::SkillType::Speed);
        character.GetSkill(BAK::SkillType::Strength);
        character.GetSkill(BAK::SkillType::Crossbow);
        character.GetSkill(BAK::SkillType::Melee);
        character.GetSkill(BAK::SkillType::Casting);

        if (speed.mCurrent == 0)
        {
            speed.mCurrent = 1;
        }
    }

    TickCombatEffectsAtEndOfTurn();
    //DamageAllCombatantsByWrathOfKillian();
    ResurrectNighthawks();

    // for each AI
    //   mark as pending
    //   clear bit 2 of state flags
    //   if current target dead clear current target

}

void CombatManager::ResurrectNighthawks()
{
    unsigned blackslayers = 0;
    for (const auto& combatant : mCombatants)
    {
        if (combatant.mCharacter && combatant.mCharacter->GetMonsterIndex() == BAK::sBlackslayer)
        {
            blackslayers++;
        }
    }

    if (blackslayers == 0)
    {
        return;
    }

    for (auto& combatant : mCombatants)
    {
        if (!combatant.mCharacter)
        {
            continue;
        }

        if (combatant.IsExorcised() || !combatant.IsDead())
        {
            continue;
        }

        auto monsterIndex = combatant.mCharacter->GetMonsterIndex();

        if (!(monsterIndex == BAK::sBlackslayer || monsterIndex == BAK::sNighthawk))
        {
            continue;
        }

        // ResurrectCombatant
    }
}

void CombatManager::TickCombatEffectsAtEndOfTurn()
{
    for (auto& combatant : mCombatants)
    {
        bool expiredCharacter = BAK::TickCombatEffects(combatant.mCombatState);
        if (expiredCharacter)
        {
            // remove the combatant (illusion) from the combat
        }
    }
}

std::vector<Combatant>::iterator CombatManager::SelectNextCombatantForTurn(bool onlyPlayer)
{
    std::vector<Combatant>::iterator best = mCombatants.end();
    std::optional<unsigned> bestSpeed;

    for (auto it = mCombatants.begin(); it != mCombatants.end(); ++it)
    {
        if (!BAK::IsCombatantActive(it->mCombatState, true))
        {
            continue;
        }

        auto* character = it->mCharacter;
        if (!character)
        {
            continue;
        }

        if (onlyPlayer && character->IsEnemy())
        {
            continue;
        }

        auto speed = character->GetSkill(BAK::SkillType::Speed);
        if (speed == 0)
        {
            speed = 1;
        }

        if (!bestSpeed || speed >= *bestSpeed)
        {
            bestSpeed = speed;
            best = it;
        }
    }

    return best;
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
