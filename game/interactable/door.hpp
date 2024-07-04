#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"
#include "bak/state/door.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "graphics/glm.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include <glm/geometric.hpp>


namespace Game::Interactable {

/*
 * These containers have an optional dialog and inventory encounter
 */
class Door : public IInteractable
{
private:

public:
    Door(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState,
        BAK::Target target,
        const EncounterCallback& encounterCallback)
    :
        mGuiManager{guiManager},
        mGameState{gameState},
        mDialogScene{
            []{},
            []{},
            [&](const auto& choice){ DialogFinished(choice); }},
        mDefaultDialog{target},
        mContainer{nullptr},
        mEncounterCallback{encounterCallback}
    {}

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType) override
    {
        mContainer = &container;

        assert(mContainer->HasDoor());
        const auto& doorStats = mContainer->GetDoor();

        Logging::LogInfo("Door") << "DoorIndex: " << doorStats.mDoorIndex << " State: " << std::boolalpha << BAK::State::GetDoorState(mGameState, doorStats.mDoorIndex) << " locked? " << doorStats.mLockRating << "\n";

        const auto playerPos = glm::cast<float>(mGameState.GetLocation().mPosition);
        const auto doorPos = glm::cast<float>(container.GetHeader().GetPosition());
        if (glm::distance(playerPos, doorPos) < 800)
        {
            StartDialog(BAK::DialogSources::mDoorTooClose);
        }
        else
        {
        }
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mContainer);
    }

    void EncounterFinished() override
    {
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
    BAK::Target mDefaultDialog;
    BAK::GenericContainer* mContainer;
    const EncounterCallback& mEncounterCallback;
};

}
