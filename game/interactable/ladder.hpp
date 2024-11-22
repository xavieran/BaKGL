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

class Ladder : public IInteractable
{
private:

    enum class State
    {
        Idle,
        Done,
    };

public:
    Ladder(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState);

    void BeginInteraction(BAK::GenericContainer& ladder, BAK::EntityType) override;
    

    void LockFinished();

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void EncounterFinished() override;
    void StartDialog(BAK::Target target);

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentLadder;
    State mState;
};

}
