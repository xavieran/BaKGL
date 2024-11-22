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

/*
 * These containers have an optional dialog and inventory encounter
 */
class Generic : public IInteractable
{
private:

public:
    Generic(
        Gui::IGuiManager& guiManager,
        BAK::Target target,
        const EncounterCallback& encounterCallback);

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType) override;
    void DoEncounter();
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);

private:
    Gui::IGuiManager& mGuiManager;
    Gui::DynamicDialogScene mDialogScene;
    BAK::Target mDefaultDialog;
    BAK::GenericContainer* mContainer;
    BAK::EntityType mEntityType;
    const EncounterCallback& mEncounterCallback;
};

}
