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

class Chest : public IInteractable
{
private:
    enum class State
    {
        Idle,
        OpenChest,
        UnlockChest,
        DisarmedTrap,
        Exploded
    };

public:
    Chest(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState);

    void BeginInteraction(BAK::GenericContainer& chest, BAK::EntityType) override;
    void EncounterFinished() override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);

    void TryOpenChest(bool openChest);
    void StartDialog(BAK::Target target);
    void TryDisarmTrap();
    void Explode();
    void ShowChestContents();
    void TryUnlockChest();
    void LockFinished();

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentChest;
    State mState;
};

}
