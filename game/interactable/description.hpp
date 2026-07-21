#pragma once

#include "game/interactable/IInteractable.hpp"

#include "bak/dialogTarget.hpp"

#include "gui/IDialogScene.hpp"

namespace BAK {
class GenericContainer;
}

namespace Gui {
class IGuiManager;
}

namespace Game::Interactable {

class Description : public IInteractable
{
public:
    Description(Gui::IGuiManager& guiManager, BAK::Target target);

    void BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType) override;
    void EncounterFinished() override;

private:
    Gui::IGuiManager& mGuiManager;
    Gui::DynamicDialogScene mDialogScene;
    BAK::Target mDialog;
};

}
