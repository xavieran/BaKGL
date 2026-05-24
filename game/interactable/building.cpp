#include "game/interactable/building.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"

#include "com/logger.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include <utility>

namespace Game::Interactable {

Building::Building(
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

void Building::BeginInteraction(BAK::GenericContainer& building, BAK::EntityType)
{
    ASSERT(mState == State::Idle);
    mCurrentBuilding = &building;

    ASSERT(mCurrentBuilding->HasDialog());

    mShowDialogFirst = CheckBitSet(mCurrentBuilding->GetDialog().mDialogOrder, 5); // test 0x20

    if (!mShowDialogFirst)
    {
        mState = State::SkipFirstDialog;
        Logging::LogDebug("Building") << "State: SkipFirstDialog\n";
        TryDoEncounter();
    }
    else
    {
        mState = State::DoFirstDialog;
        Logging::LogDebug("Building") << "State: DoFirstDialog\n";
        StartDialog(mCurrentBuilding->GetDialog().mDialog);
    }
}

void Building::LockFinished()
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

void Building::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mState != State::SkipFirstDialog);

    if (mState == State::DoFirstDialog)
    {
        Logging::LogDebug("Building") << "State: DoneFirstDialog: \n";
        if (mGameState.GetEndOfDialogState() == -1)
        {
            Logging::LogDebug("Building") << "EndOfDialogState -1 -> exiting interaction early\n";
            mState = State::Done;
            return;
        }
        TryDoEncounter();
        StartDialog(mCurrentBuilding->GetDialog().mDialog);
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
        Logging::LogDebug("Building") << "State: ShowInv\n";
        TryDoInventory();
    }
    else if (mState == State::TryDoGDS)
    {
        Logging::LogDebug("Building") << "State: TryDoGDS\n";
        if ((!choice || choice->mValue == BAK::Keywords::sYesIndex)
            && mGameState.GetEndOfDialogState() != -1)
            TryDoGDS();
        else
            mState = State::Idle;
    }
    else if (mState == State::Done)
    {
        mState = State::Idle;
    }
    else
    {
        ASSERT(false);
    }
}

void Building::TryDoEncounter()
{
    if (mCurrentBuilding->HasEncounter()
        && mCurrentBuilding->GetEncounter().mEncounterPos)
    {
        Logging::LogInfo("Building") << __FUNCTION__ << " " 
            << mCurrentBuilding->GetEncounter() << "\n";

        // Combats triggered from buildings should be unavoidable
        mGameState.SetCombatTriggeredFromInteractable(true);
        auto encounterActive = std::invoke(
            mEncounterCallback,
            *mCurrentBuilding->GetEncounter().mEncounterPos + glm::uvec2{600, 600});
        mGameState.SetCombatTriggeredFromInteractable(false);

        if (!encounterActive)
        {
            if (mCurrentBuilding->GetDialog().mDialog != BAK::Target{BAK::KeyTarget{0}})
            {
                StartDialog(mCurrentBuilding->GetDialog().mDialog);
                // is this the right state?
                mState = State::Done;
            }
        }
    }
    else
    {
        TryDoLock();
    }
}

void Building::TryDoLock()
{
    if (mCurrentBuilding->HasLock())
    {
        mState = State::TryUnlock;
        mGameState.SetDialogContext_7530(2);
        StartDialog(BAK::DialogSources::mChooseUnlock);
    }
    else
    {
        auto forceDayContext = !CheckBitSet(mCurrentBuilding->GetDialog().mDialogOrder, 0);
        auto isDayTime = mGameState.ReadEvent(std::to_underlying(BAK::ActiveStateFlag::DayTime));

        mGameState.SetDialogContext_7530(
            (forceDayContext || isDayTime) ? 0 : 1);

        mState = State::TryDoGDS;
        if (!mShowDialogFirst)
        {
            StartDialog(mCurrentBuilding->GetDialog().mDialog);
        }
        else
        {
            TryDoGDS();
        }

    }
}

void Building::TryDoGDS()
{
    ASSERT(mState == State::TryDoGDS);
    if (mCurrentBuilding->HasEncounter()
        && mCurrentBuilding->GetEncounter().mHotspotRef)
    {
        // Do GDS
        mState = State::Idle;
        mGuiManager.DoFade(.8, [this]{
            mGuiManager.EnterGDSScene(
                *mCurrentBuilding->GetEncounter().mHotspotRef,
                []{});
            });
    }
    else
    {
        mState = State::ShowInventory;
        TryDoInventory();
    }
}

void Building::TryDoInventory()
{
    ASSERT(mState == State::ShowInventory);
    if (mCurrentBuilding->HasInventory())
    {
        mGuiManager.ShowContainer(mCurrentBuilding, BAK::EntityType::BUILDING);
    }
    mState = State::Idle;
}

void Building::EncounterFinished()
{
    TryDoLock();
}

void Building::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
