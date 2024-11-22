#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"

#include <glm/glm.hpp>

#include <optional>

namespace BAK {
class GameState;
class GenericContainer;
}

namespace Gui {
class IGuiManager;
}

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
        const EncounterCallback& encounterCallback);

    void BeginInteraction(BAK::GenericContainer& building, BAK::EntityType) override;
    void LockFinished();
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void TryDoEncounter();
    void TryDoLock();
    void TryDoGDS();
    void TryDoInventory();
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentBuilding;
    State mState;
    const EncounterCallback& mEncounterCallback;
};

}
