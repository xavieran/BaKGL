#pragma once

#include "gui/IDialogScene.hpp"

#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"
#include "bak/types.hpp"

namespace Gui {

class IGuiManager
{
public:
    virtual void EnterGDSScene(
        const BAK::HotspotRef&,
        std::function<void()>&& finished) = 0;
    virtual void ExitGDSScene() = 0;

    virtual void StartDialog(BAK::Target, bool tooltip, bool drawWorldFrame, IDialogScene*) = 0;

    virtual void ShowCharacterPortrait(BAK::ActiveCharIndex) = 0;
    virtual void ExitCharacterPortrait() = 0;

    virtual void ShowInventory(BAK::ActiveCharIndex) = 0;
    virtual void ShowContainer(BAK::IContainer*) = 0;
    virtual void ExitInventory() = 0;

    virtual void ShowLock(BAK::IContainer*) = 0;
    virtual void ExitLock() = 0;
};

class NullGuiManager : public IGuiManager
{
    void EnterGDSScene(
        const BAK::HotspotRef&,
        std::function<void()>&&) override {}
    void ExitGDSScene() override {}

    void StartDialog(BAK::Target, bool, bool, IDialogScene*) override {}

    void ShowCharacterPortrait(BAK::ActiveCharIndex) override {}
    void ExitCharacterPortrait() override {}

    void ShowInventory(BAK::ActiveCharIndex) override {}
    void ShowContainer(BAK::IContainer*) override {}
    void ExitInventory() override {}

    void ShowLock(BAK::IContainer*) override {}
    void ExitLock() override {}
};

}
