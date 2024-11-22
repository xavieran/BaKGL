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

class Combatant : public IInteractable
{
private:

public:
    Combatant(
        Gui::IGuiManager& guiManager,
        BAK::Target target);

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType) override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);

private:
    Gui::IGuiManager& mGuiManager;
    Gui::DynamicDialogScene mDialogScene;
    BAK::Target mDefaultDialog;
    BAK::GenericContainer* mContainer;
    BAK::EntityType mEntityType;
};

}
