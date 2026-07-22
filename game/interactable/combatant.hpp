#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"

#include "gui/IDialogScene.hpp"

#include <glm/glm.hpp>

#include <optional>

namespace BAK {
class GameState;
}

namespace Gui {
class IGuiManager;
}

namespace Game::Interactable {

class Combatant : public IInteractable
{
public:
    Combatant(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState);

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType, BAK::EntityIndex) override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mContainer;
    BAK::EntityType mEntityType;
};

}
