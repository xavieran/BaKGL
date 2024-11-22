
#include "game/interactable/tomb.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/entityType.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Tomb::Tomb(
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
    mCurrentTomb{nullptr},
    mState{State::Idle},
    mEncounterCallback{encounterCallback}
{}

void Tomb::BeginInteraction(BAK::GenericContainer& tomb, BAK::EntityType)
{
    mCurrentTomb = &tomb;
    mGameState.SetDialogContext_7530(0);
    ASSERT(mState == State::Idle);

    // All tombs should have dialog
    ASSERT(mCurrentTomb->HasDialog());

    StartDialog(mCurrentTomb->GetDialog().mDialog);
}

void Tomb::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mCurrentTomb);

    if (mState == State::Done || !choice)
    {
        mState = State::Idle;
        return;
    }

    if (choice->mValue == BAK::Keywords::sYesIndex)
    {
        if (!mGameState.GetParty().HaveItem(BAK::sShovel))
        {
            // Don't have a shovel
            mState = State::Done;
            StartDialog(BAK::DialogSources::mTombNoShovel);
        }
        else
        {
            if (mCurrentTomb->HasEncounter())
            {
                DoEncounter();
            }
            else
            {
                DigTomb();
            }
        }
    }
}

void Tomb::DigTomb()
{
    const auto dialogOrder = mCurrentTomb->GetDialog().mDialogOrder;
    mGameState.GetParty().RemoveItem(BAK::sShovel.mValue, 1);

    // Just show this dialog and do nothing else
    if (CheckBitSet(dialogOrder, 1))
    {
        mState = State::Idle;
        ShowTombContents();
    }
    else if (CheckBitSet(dialogOrder, 2))
    {
        mState = State::Done;
        // Just a body
        StartDialog(BAK::DialogSources::mTombJustABody);
    }
    else if (CheckBitSet(dialogOrder, 3))
    {
        mState = State::Done;
        // No body
        StartDialog(BAK::DialogSources::mTombNoBody);
    }
    else
    {
        ASSERT(false);
    }
}

void Tomb::DoEncounter()
{
    Logging::LogInfo("Tomb") << __FUNCTION__ << " " 
        << mCurrentTomb->GetEncounter() << "\n";
    std::invoke(
        mEncounterCallback,
        *mCurrentTomb->GetEncounter().mEncounterPos);
}

void Tomb::EncounterFinished()
{
    DigTomb();
}

void Tomb::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

void Tomb::ShowTombContents()
{
    if (mCurrentTomb->HasEncounter()
        && mCurrentTomb->GetEncounter().mSetEventFlag != 0)
    {
        mGameState.SetEventValue(
            mCurrentTomb->GetEncounter().mSetEventFlag,
            1);
    }

    mGuiManager.ShowContainer(mCurrentTomb, BAK::EntityType::TOMBSTONE);
}

}
