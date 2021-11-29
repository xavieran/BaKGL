#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

class Building : public IInteractable
{
private:

    enum class State
    {
        Idle,
        SkipFirstDialog,
        DoFirstDialog,
        TryUnlock,
        Unlocking,
        ShowInventory,
        TryDoGDS,
        Done,
    };

public:
    Building(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState,
        const EncounterCallback& encounterCallback)
    :
        mGuiManager{guiManager},
        mGameState{gameState},
        mDialogScene{
            []{},
            []{},
            [&](const auto& choice){ DialogFinished(choice); }},
        mCurrentBuilding{nullptr},
        mState{State::Idle},
        mEncounterCallback{encounterCallback}
    {}

    void BeginInteraction(BAK::GenericContainer& building) override
    {
        ASSERT(mState == State::Idle);
        mCurrentBuilding = &building;

        ASSERT(mCurrentBuilding->HasDialog());

        mGameState.SetDialogContext(2);

        if (!CheckBitSet(mCurrentBuilding->GetDialog().mDialogOrder, 5))
        {
            // Skip dialog
            mState = State::SkipFirstDialog;
            TryDoEncounter();
        }
        else
        {
            mState = State::DoFirstDialog;
            StartDialog(mCurrentBuilding->GetDialog().mDialog);
        }
    }

    void LockFinished()
    {
        ASSERT(!mCurrentBuilding->GetLock().IsFairyChest())

        if (mGuiManager.IsLockOpened())
        {
            // Unlockable buildings must always have a dialog
            ASSERT(mCurrentBuilding->HasDialog());
            mState = State::ShowInventory;
            if (!CheckBitSet(mCurrentBuilding->GetDialog().mDialogOrder, 5))
            {
                StartDialog(mCurrentBuilding->GetDialog().mDialog);
            }
            else
            {
                TryDoInventory();
            }
        }
        else
        {
            mState = State::Idle;
        }
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mState != State::SkipFirstDialog);

        if (mState == State::DoFirstDialog)
        {
            TryDoEncounter();
            return;
        }
        else if (mState == State::TryUnlock)
        {
            ASSERT(choice);
            if (choice->mValue == BAK::Keywords::sYesIndex)
            {
                mState = State::Unlocking;
                mGuiManager.ShowLock(
                    mCurrentBuilding,
                    [this]{ LockFinished(); });
            }
            else
            {
                mState = State::Idle;
            }
        }
        else if (mState == State::ShowInventory)
        {
            TryDoInventory();
        }
        else if (mState == State::TryDoGDS)
        {
            if (!choice || choice->mValue == BAK::Keywords::sYesIndex)
                TryDoGDS();
            else
                mState = State::Idle;
        }
        else
        {
            ASSERT(false);
        }
    }

    void TryDoEncounter()
    {
        if (mCurrentBuilding->HasEncounter()
        // Hack... probably there's a nicer way
            && !mCurrentBuilding->GetEncounter().mHotspotRef)
        {
            Logging::LogInfo("Building") << __FUNCTION__ << " " 
                << mCurrentBuilding->GetEncounter() << "\n";
            std::invoke(
                mEncounterCallback,
                *mCurrentBuilding->GetEncounter().mEncounterPos);
        }
        else
        {
            TryDoLock();
        }
    }

    void TryDoLock()
    {
        if (mCurrentBuilding->HasLock())
        {
            mState = State::TryUnlock;
            mGameState.SetDialogContext(2);
            StartDialog(BAK::DialogSources::mChooseUnlock);
        }
        else
        {
            mState = State::TryDoGDS;
            if (!CheckBitSet(mCurrentBuilding->GetDialog().mDialogOrder, 5))
                StartDialog(mCurrentBuilding->GetDialog().mDialog);
            else
                TryDoGDS();

        }
    }

    void TryDoGDS()
    {
        ASSERT(mState == State::TryDoGDS);
        if (mCurrentBuilding->HasEncounter()
            && mCurrentBuilding->GetEncounter().mHotspotRef)
        {
            // Do GDS
            mState = State::Idle;
            mGuiManager.EnterGDSScene(
                *mCurrentBuilding->GetEncounter().mHotspotRef,
                []{});
        }
        else
        {
            mState = State::ShowInventory;
            TryDoInventory();
        }
    }

    void TryDoInventory()
    {
        ASSERT(mState == State::ShowInventory);
        if (mCurrentBuilding->HasInventory())
        {
            mGuiManager.ShowContainer(mCurrentBuilding);
        }
        mState = State::Idle;
    }

    void EncounterFinished() override
    {
        TryDoLock();
    }

    void StartDialog(BAK::Target target)
    {
        mGuiManager.StartDialog(
            target,
            false,
            false,
            &mDialogScene);
    }

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentBuilding;
    State mState;
    const EncounterCallback& mEncounterCallback;
};

}
