#include "game/interactable/ladder.hpp"

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

Ladder::Ladder(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mCurrentLadder{nullptr},
    mState{State::Idle}
{}

void Ladder::BeginInteraction(BAK::GenericContainer& ladder, BAK::EntityType)
{
    ASSERT(mState == State::Idle);

    mCurrentLadder = &ladder;
    mGameState.SetDialogContext_7530(3);

    // All ladders should have dialog and a lock
    ASSERT(mCurrentLadder->HasLock())

    StartDialog(BAK::DialogSources::mChooseUnlock);
}

void Ladder::LockFinished()
{
    ASSERT(!mCurrentLadder->GetLock().IsFairyChest())

    if (mGuiManager.IsLockOpened())
    {
        // Unlockable ladders must always have a dialog
        ASSERT(mCurrentLadder->HasDialog());
        mState = State::Done;
        StartDialog(mCurrentLadder->GetDialog().mDialog);
    }
    else
    {
        mState = State::Idle;
    }
}

void Ladder::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mCurrentLadder);

    if (mState == State::Done)
    {
        mState = State::Idle;
        if (mGameState.GetTransitionChapter_7541())
        {
            mGameState.SetTransitionChapter_7541(false);
            mGuiManager.DoChapterTransition();
        }
        return;
    }

    ASSERT(choice);

    if (choice->mValue == BAK::Keywords::sYesIndex)
    {
        mGuiManager.ShowLock(
            mCurrentLadder,
            [this]{ LockFinished(); });
    }
}

void Ladder::EncounterFinished()
{
}

void Ladder::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
