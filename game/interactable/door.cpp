#include "game/interactable/door.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"
#include "bak/state/door.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"

#include "graphics/glm.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include <glm/geometric.hpp>


namespace Game::Interactable {

Door::Door(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mContainer{nullptr}
{}

void Door::BeginInteraction(BAK::GenericContainer& container, BAK::EntityType)
{
    mContainer = &container;

    assert(mContainer->HasDoor());
    const auto doorIndex = mContainer->GetDoor();
    const auto doorState = BAK::State::GetDoorState(mGameState, doorIndex.mValue);
    Logging::LogInfo("Door") << "DoorIndex: " << doorIndex << " DoorOpen? " << std::boolalpha << doorState << " locked? " << (mContainer->HasLock() ? mContainer->GetLock().mRating : 0) << "\n";

    const auto playerPos = glm::cast<float>(mGameState.GetLocation().mPosition);
    const auto doorPos = glm::cast<float>(container.GetHeader().GetPosition());
    if (glm::distance(playerPos, doorPos) < 800)
    {
        StartDialog(BAK::DialogSources::mDoorTooClose);
    }
    else if (doorState)
    {
        // Door opened, can always close it
        CloseDoor();
    }
    else if (mContainer->HasLock() && mContainer->GetLock().mRating > 0)
    {
        mGameState.SetDialogContext_7530(1);
        StartDialog(BAK::DialogSources::mChooseUnlock);
    }
    else
    {
        OpenDoor();
    }
}

void Door::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mContainer);
    ASSERT(choice);

    if (choice->mValue == BAK::Keywords::sYesIndex)
    {
        mGuiManager.ShowLock(
            mContainer,
            [this]{ LockFinished(); });
    }
}

void Door::LockFinished()
{
    if (mGuiManager.IsLockOpened())
    {
        OpenDoor();
    }
}

void Door::OpenDoor()
{
    const auto doorIndex = mContainer->GetDoor().mValue;
    mGameState.Apply(BAK::State::SetDoorState, doorIndex, true);
    Logging::LogInfo(__FUNCTION__) << " index; " << doorIndex << "\n";
}

void Door::CloseDoor()
{
    const auto doorIndex = mContainer->GetDoor().mValue;
    mGameState.Apply(BAK::State::SetDoorState, doorIndex, false);
    Logging::LogInfo(__FUNCTION__) << " index; " << doorIndex << "\n";
}

void Door::EncounterFinished()
{
}

void Door::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
