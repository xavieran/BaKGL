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

class Tomb : public IInteractable
{
private:

    enum class State
    {
        Idle,
        Done,
    };

public:
    Tomb(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState,
        const EncounterCallback& encounterCallback);

    void BeginInteraction(BAK::GenericContainer& tomb, BAK::EntityType) override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void DigTomb();
    void DoEncounter();
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);
    void ShowTombContents();

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentTomb;
    State mState;
    const EncounterCallback& mEncounterCallback;
};

}
