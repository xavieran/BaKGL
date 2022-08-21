#pragma once

#include "game/interactable/IInteractable.hpp"

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

class Chest : public IInteractable
{
private:
    enum class State
    {
        Idle,
        OpenChest,
        UnlockChest,
        DisarmedTrap,
        Exploded
    };

    static constexpr auto sExploded = AudioA::SoundIndex{0x39};

public:
    Chest(
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

    void BeginInteraction(BAK::GenericContainer& chest) override
    {
        ASSERT(mState == State::Idle);

        mCurrentChest = &chest;

        mGameState.SetDialogContext(0);

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
            // Scent of sarig aactive and chest is trapped
            if (mGameState.GetSpellActive(5) && chest.GetLock().mLockFlag == 4)
            {
                // Box is trapped, do we want to open it?
                StartDialog(BAK::DialogSources::mOpenTrappedBox);
            }
            // Chest is not trapped
            else if (chest.GetLock().mLockFlag == 1)
            {
                ASSERT(chest.GetLock().mLockFlag == 1);
                StartDialog(BAK::DialogSources::mOpenExplodedChest);
            }
            else
            {
                StartDialog(BAK::DialogSources::mChooseUnlock);
            }
        }
    }

    void EncounterFinished() override {}

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mCurrentChest);
        if (mState == State::OpenChest)
        {
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
            ASSERT(false);
        }
    }

    void TryOpenChest(bool openChest)
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
                if (mGameState.GetSpellActive(5) && lock.mLockFlag == 4)
                {
                    TryDisarmTrap();
                }
                else if (lock.mLockFlag == 1)
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

    void StartDialog(BAK::Target target)
    {
        mGuiManager.StartDialog(
            target,
            false,
            false,
            &mDialogScene);
    }

    void TryDisarmTrap()
    {
        ASSERT(mState == State::OpenChest);
        ASSERT(mCurrentChest);
        auto& lock = mCurrentChest->GetLock();
        const auto& [character, skill] = mGameState
            .GetParty().GetSkill(BAK::SkillType::Lockpick, true);
        if (skill > lock.mRating)
        {
            mGameState.GetParty()
                .GetCharacter(character)
                .ImproveSkill(BAK::SkillType::Lockpick, 3, 2);
            lock.mLockFlag = 1;
            lock.mTrapDamage = 0;
            mState = State::DisarmedTrap;
            StartDialog(BAK::KeyTarget{0xbf});
        }
        else
        {
            Explode();
        }
    }

    void Explode()
    {
        // Do TrapDamage amount of damage to party
        mCurrentChest->GetLock().mTrapDamage = 0;
        StartDialog(BAK::KeyTarget{0xc0});
        mState = State::Exploded;
        AudioA::AudioManager::Get().PlaySound(sExploded);
    }

    void ShowChestContents()
    {
        if (mCurrentChest->HasEncounter()
            && mCurrentChest->GetEncounter().mSetEventFlag != 0)
        {
            mGameState.SetEventValue(
                mCurrentChest->GetEncounter().mSetEventFlag,
                1);
        }

        mGuiManager.ShowContainer(mCurrentChest);
    }

    void TryUnlockChest()
    {
        mState = State::UnlockChest;
        mGuiManager.ShowLock(
            mCurrentChest,
            [this]{ LockFinished(); });
    }

    void LockFinished()
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

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentChest;
    State mState;
};

}
