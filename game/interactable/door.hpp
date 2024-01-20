#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

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
        BAK::Target target,
        const EncounterCallback& encounterCallback)
    :
        mGuiManager{guiManager},
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

        StartDialog(mDefaultDialog);
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
    Gui::DynamicDialogScene mDialogScene;
    BAK::Target mDefaultDialog;
    BAK::GenericContainer* mContainer;
    const EncounterCallback& mEncounterCallback;
};

}
