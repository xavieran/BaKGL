#pragma once

#include "gui/IDialogScene.hpp"
#include "gui/screenStack.hpp"

#include "gui/animator.hpp"

#include "bak/dialog.hpp"
#include "bak/hotspot.hpp"
#include "bak/types.hpp"

namespace Gui {

class IGuiManager
{
public:
    virtual void EnterMainView() = 0;
    virtual void EnterMainMenu(bool gameRunning) = 0;

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

    virtual void ShowLock(BAK::IContainer*, std::function<void()>&& finished) = 0;
    virtual void ShowFullMap() = 0;
    virtual void ExitLock() = 0;
    virtual bool IsLockOpened() const = 0;
    virtual bool IsWordLockOpened() const = 0;

    virtual void AddAnimator(LinearAnimator&&) = 0;
    virtual ScreenStack& GetScreenStack() = 0;
};

}
