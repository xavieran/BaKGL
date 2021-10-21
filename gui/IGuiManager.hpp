#pragma once

#include "gui/IDialogScene.hpp"

#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"

namespace Gui {

class IGuiManager
{
public:
    virtual void EnterGDSScene(
        const BAK::HotspotRef&,
        std::function<void()>&& finished) = 0;
    virtual void ExitGDSScene() = 0;
    virtual void StartDialog(BAK::Target, bool tooltip, IDialogScene*) = 0;
    virtual void ShowCharacterPortrait(unsigned character) = 0;
    virtual void ExitCharacterPortrait() = 0;
    virtual void ShowInventory(unsigned character) = 0;
    virtual void ShowContainer(BAK::IContainer*) = 0;
    virtual void ExitInventory() = 0;
};

class NullGuiManager : public IGuiManager
{
    void EnterGDSScene(
        const BAK::HotspotRef&,
        std::function<void()>&&) override {}
    void ExitGDSScene() override {}
    void StartDialog(BAK::Target, bool, IDialogScene*) override {}
    void ShowCharacterPortrait(unsigned character) override {}
    void ExitCharacterPortrait() override {}
    void ShowInventory(unsigned character) override {}
    void ShowContainer(BAK::IContainer*) override {}
    void ExitInventory() override {}
};

}
