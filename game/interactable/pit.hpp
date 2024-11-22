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

class Pit : public IInteractable
{
public:
    Pit(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState);

    void BeginInteraction(BAK::GenericContainer& pit, BAK::EntityType) override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void StartDialog(BAK::Target target);
    void EncounterFinished() override;

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentPit;
};

}
