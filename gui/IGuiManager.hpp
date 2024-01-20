#pragma once

#include "gui/IDialogScene.hpp"
#include "gui/screenStack.hpp"

#include "gui/IAnimator.hpp"

#include "bak/dialog.hpp"
#include "bak/entityType.hpp"
#include "bak/hotspot.hpp"
#include "bak/saveManager.hpp"
#include "bak/types.hpp"

namespace Gui {

class IGuiManager
{
public:
    virtual void DoFade(double duration, std::function<void()>&&) = 0;
    virtual bool InMainView() const = 0;
    virtual void EnterMainView() = 0;
    virtual void EnterMainMenu(bool gameRunning) = 0;

    virtual void EnterGDSScene(
        const BAK::HotspotRef&,
        std::function<void()>&& finished) = 0;
    virtual void ExitGDSScene() = 0;

    virtual void StartDialog(BAK::Target, bool tooltip, bool drawWorldFrame, IDialogScene*) = 0;

    virtual void ShowCharacterPortrait(BAK::ActiveCharIndex) = 0;
    virtual void ExitSimpleScreen() = 0;

    virtual void ShowInventory(BAK::ActiveCharIndex) = 0;
    virtual void ShowContainer(BAK::IContainer*, BAK::EntityType containerType) = 0;
    virtual void SelectItem(std::function<void(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>>)>&&) = 0;
    virtual void ExitInventory() = 0;

    virtual void ShowLock(BAK::IContainer*, std::function<void()>&& finished) = 0;
    virtual void ShowCamp(bool isInn) = 0;
    virtual void ShowFullMap() = 0;
    virtual void ShowTeleport(unsigned sourceTemple) = 0;
    virtual void ShowCureScreen(
        unsigned templeNumber,
        unsigned cureFactor,
        std::function<void()>&& finished) = 0;
    virtual void ExitLock() = 0;
    virtual bool IsLockOpened() const = 0;
    virtual bool IsWordLockOpened() const = 0;

    virtual void AddAnimator(std::unique_ptr<IAnimator>&&) = 0;
    virtual ScreenStack& GetScreenStack() = 0;

    virtual void LoadGame(std::string) = 0;
    virtual void SaveGame(const BAK::SaveFile&) = 0;
    virtual void DoTeleport(BAK::TeleportIndex) = 0;
};

}
