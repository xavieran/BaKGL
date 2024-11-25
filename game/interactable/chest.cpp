#include "game/interactable/chest.hpp"

#include "game/interactable/IInteractable.hpp"

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"

#include "com/logger.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Chest::Chest(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mCurrentChest{nullptr},
    mState{State::Idle}
{}

void Chest::BeginInteraction(BAK::GenericContainer& chest, BAK::EntityType)
{
    ASSERT(mState == State::Idle);

    mCurrentChest = &chest;

    mGameState.SetDialogContext_7530(0);

    mState = State::OpenChest;
    if (!chest.HasLock())
    {
        // If no lock, just open the box
        StartDialog(BAK::DialogSources::mOpenUnlockedBox);
    }
    else if (chest.GetLock().IsFairyChest())
    {
        // If word lock, show flavour text then transition
        // into word lock screen
        StartDialog(BAK::DialogSources::mWordlockIntro);
    }
    else if (!chest.GetLock().IsTrapped())
    {
        // If normal lock, ask if user wants to open lock
        StartDialog(BAK::DialogSources::mChooseUnlock);
    }
    else
    {
        // Scent of sarig active and chest is trapped
        if (mGameState.GetSpellActive(BAK::StaticSpells::ScentOfSarig)
            && chest.GetLock().mLockFlag == 4 && chest.GetLock().mTrapDamage > 0)
        {
            StartDialog(BAK::DialogSources::mOpenTrappedBox);
        }
        // Scent of sarig not active and chest was trapped, chest is incinerated
        else if (chest.GetLock().mLockFlag == 4 && chest.GetLock().mTrapDamage == 0)
        {
            StartDialog(BAK::DialogSources::mOpenExplodedChest);
        }
        // Chest is not trapped
        else if (chest.GetLock().mLockFlag == 1)
        {
            ASSERT(chest.GetLock().mLockFlag == 1);
            StartDialog(BAK::DialogSources::mChooseUnlock);
        }
        else
        {
            StartDialog(BAK::DialogSources::mChooseUnlock);
        }
    }
}

void Chest::EncounterFinished() {}

void Chest::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    if (mState == State::OpenChest)
    {
        ASSERT(mCurrentChest);
        TryOpenChest(!choice
            || (choice->mValue == BAK::Keywords::sYesIndex));
    }
    else if (mState == State::DisarmedTrap)
    {
        ShowChestContents();
        mState = State::Idle;
    }
    else if (mState == State::Exploded)
    {
        mState = State::Idle;
    }
    else
    {
        return;
    }
}

void Chest::TryOpenChest(bool openChest)
{
    ASSERT(mCurrentChest);
    ASSERT(mState != State::Idle);

    if (!mCurrentChest->HasLock())
    {
        mState = State::Idle;
        ShowChestContents();
    }
    else if (mCurrentChest->GetLock().IsFairyChest())
    {
        TryUnlockChest();
    }
    else if (!mCurrentChest->GetLock().IsTrapped())
    {
        if (openChest)
            TryUnlockChest();
        else
            mState = State::Idle;
    }
    else
    {
        if (openChest)
        {
            auto& lock = mCurrentChest->GetLock();
            if (mGameState.GetSpellActive(BAK::StaticSpells::ScentOfSarig)
                && lock.mLockFlag == 4 && lock.mTrapDamage > 0)
            {
                TryDisarmTrap();
            }
            else if ((lock.mLockFlag == 4 && lock.mTrapDamage == 0)
                || lock.mLockFlag == 1)
            {
                mState = State::Idle;
                ShowChestContents();
            }
            else
            {
                Explode();
            }
        }
        else
            mState = State::Idle;
    }
}

void Chest::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

void Chest::TryDisarmTrap()
{
    ASSERT(mState == State::OpenChest);
    ASSERT(mCurrentChest);
    auto& lock = mCurrentChest->GetLock();
    const auto& [character, skill] = mGameState
        .GetPartySkill(BAK::SkillType::Lockpick, true);
    if (skill > lock.mRating)
    {
        mGameState.GetParty()
            .GetCharacter(character)
            .ImproveSkill(
                BAK::SkillType::Lockpick,
                BAK::SkillChange::ExercisedSkill,
                2);
        lock.mTrapDamage = 0;
        mState = State::DisarmedTrap;
        StartDialog(BAK::DialogSources::mDisarmedTrappedBox);
    }
    else
    {
        Explode();
    }
}

void Chest::Explode()
{
    ASSERT(mCurrentChest->HasLock());
    mGameState.GetParty().ImproveSkillForAll(
        BAK::SkillType::TotalHealth,
        BAK::SkillChange::HealMultiplier_100,
        -(mCurrentChest->GetLock().mTrapDamage << 8));
    mCurrentChest->GetLock().mTrapDamage = 0;
    StartDialog(BAK::DialogSources::mTrappedChestExplodes);
    mState = State::Exploded;
    constexpr auto sExploded = AudioA::SoundIndex{0x39};
    AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sExploded);
}

void Chest::ShowChestContents()
{
    if (mCurrentChest->HasEncounter()
        && mCurrentChest->GetEncounter().mSetEventFlag != 0)
    {
        mGameState.SetEventValue(
            mCurrentChest->GetEncounter().mSetEventFlag,
            1);
    }

    mGuiManager.ShowContainer(mCurrentChest, BAK::EntityType::CHEST);
}

void Chest::TryUnlockChest()
{
    mState = State::UnlockChest;
    mGuiManager.ShowLock(
        mCurrentChest,
        [this]{ LockFinished(); });
}

void Chest::LockFinished()
{
    ASSERT(mState == State::UnlockChest);
    mState = State::Idle;
    if (mCurrentChest->GetLock().IsFairyChest())
    {
        if (mGuiManager.IsWordLockOpened())
            ShowChestContents();
    }
    else
    {
        if (mGuiManager.IsLockOpened())
            ShowChestContents();
    }
}

}
