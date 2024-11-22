#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"

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

class Door : public IInteractable
{
private:

public:
    Door(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState);

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType) override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void LockFinished();
    void OpenDoor();
    void CloseDoor();
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);
private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mContainer;
};

}
