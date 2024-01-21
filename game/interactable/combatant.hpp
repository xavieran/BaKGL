#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

/*
 * These containers have an optional dialog and inventory encounter
 */
class Combatant : public IInteractable
{
private:

public:
    Combatant(
        Gui::IGuiManager& guiManager,
        BAK::Target target)
    :
        mGuiManager{guiManager},
        mDialogScene{
            []{},
            []{},
            [&](const auto& choice){ DialogFinished(choice); }},
        mDefaultDialog{target},
        mContainer{nullptr}
    {}

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType) override
    {
        mContainer = &container;
        mEntityType = entityType;

        StartDialog(BAK::DialogSources::mBody);
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mContainer);
        if (mContainer->HasInventory())
        {
            mGuiManager.ShowContainer(mContainer, BAK::EntityType::DEADBODY1);
        }
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
    BAK::EntityType mEntityType;
};

}
