#pragma once

#include "gui/IAnimator.hpp"
#include "gui/IDialogScene.hpp"
#include "gui/screenStack.hpp"

#include "bak/container.hpp"
#include "bak/cutscenes.hpp"
#include "bak/entityType.hpp"
#include "bak/encounter/teleport.hpp"
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
    virtual void PlayCutscene(std::vector<BAK::CutsceneAction> actions, std::function<void()>&&) = 0;
    virtual void DoChapterTransition() = 0;

    virtual void ShowCharacterPortrait(BAK::ActiveCharIndex) = 0;
    virtual void ExitSimpleScreen() = 0;

    virtual void ShowInventory(BAK::ActiveCharIndex) = 0;
    virtual void ShowContainer(BAK::GenericContainer*, BAK::EntityType containerType) = 0;
    virtual void SelectItem(std::function<void(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>>)>&&) = 0;
    virtual void ExitInventory() = 0;

    virtual void ShowLock(BAK::IContainer*, std::function<void()>&& finished) = 0;
    virtual void ShowCamp(bool isInn, BAK::ShopStats* inn) = 0;
    virtual void ShowCast(bool inCombat) = 0;
    virtual void ShowFullMap() = 0;
    virtual void ShowGameStartMap() = 0;
    virtual void ShowTeleport(unsigned sourceTemple, BAK::ShopStats* temple) = 0;
    virtual void ShowCureScreen(
        unsigned templeNumber,
        unsigned cureFactor,
        std::function<void()>&& finished) = 0;
    virtual void ExitLock() = 0;
    virtual bool IsLockOpened() const = 0;
    virtual bool IsWordLockOpened() const = 0;

    virtual void AddAnimator(std::unique_ptr<IAnimator>&&) = 0;
    virtual ScreenStack& GetScreenStack() = 0;

    virtual void LoadGame(std::string, std::optional<BAK::Chapter>) = 0;
    virtual void SaveGame(const BAK::SaveFile&) = 0;
    virtual void DoTeleport(BAK::Encounter::Teleport) = 0;
};

}
