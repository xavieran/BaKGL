#pragma once

#include "bak/IZoneLoader.hpp"
#include "bak/chapterTransitions.hpp"
#include "bak/cutscenes.hpp"
#include "bak/combat/combat.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/entityType.hpp"
#include "bak/saveManager.hpp"

#include "com/scopeGuard.hpp"

#include "gui/IGuiManager.hpp"

#include "gui/actors.hpp"
#include "gui/animatorStore.hpp"
#include "gui/camp/campScreen.hpp"
#include "gui/cast/castScreen.hpp"
#include "gui/combat/combatScreen.hpp"
#include "gui/temple/cureScreen.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/fadeScreen.hpp"
#include "gui/fontManager.hpp"
#include "gui/fullMap.hpp"
#include "gui/gdsScene.hpp"
#include "gui/icons.hpp"
#include "gui/info/infoScreen.hpp"
#include "gui/inventory/inventoryScreen.hpp"
#include "gui/lock/lockScreen.hpp"
#include "gui/lock/moredhelScreen.hpp"
#include "gui/mainMenuScreen.hpp"
#include "gui/mainView.hpp"
#include "gui/teleportScreen.hpp"
#include "gui/cutscenePlayer.hpp"
#include "gui/core/widget.hpp"
#include "gui/IDialogScene.hpp"

#include "bak/partyChangeCache.hpp"
#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <utility>

namespace BAK {
class GameState;
class ICombatManager;
class ICombatUI;
}

namespace Gui {

enum class OnExit { Discard, Run };

class IDialogScene;
class Cursor;

class GuiManager final : public Widget, public IGuiManager
{
public:

    GuiManager(
        Cursor& cursor,
        Graphics::SpriteManager& spriteManager,
        BAK::GameState& gameState);
        
    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override;

    ScreenStack& GetScreenStack() override;
    void LoadGame(std::string save, std::optional<BAK::Chapter> chapter) override;
    void SaveGame(const BAK::SaveFile& saveFile) override;
    void SaveBookmark() override;

    void SetZoneLoader(BAK::IZoneLoader* zoneLoader);
    void SetDebugDisableFades(bool disable);
    void DoFade(double duration, std::function<void()>&& fadeFunction) override;
    void PlayCutscene(
        std::vector<BAK::CutsceneAction> actions,
        std::function<void()>&& cutsceneFinished) override;
    void CutsceneFinished();
    bool InMainView() const override;
    bool InCombatView() const override;
    void EnterMainView() override;
    void EnterMainMenu(bool gameRunning) override;
    void PartyDied(BAK::Target dialog) override;

    void TeleportToGDS(
        const BAK::HotspotRef& hotspot);

    void OnTimeDelta(double delta);
    void AddAnimator(std::unique_ptr<IAnimator>&& animator) override;

    void EnterGDSScene(
        const BAK::HotspotRef& hotspot,
        std::function<void()>&& finished) override;

    void ExitGDSScene() override;
    void RemoveGDSScene(bool runFinished=false);

    void StartDialog(
        BAK::Target dialog,
        bool isTooltip,
        bool drawWorldFrame,
        IDialogScene* scene) override;
    
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice);
    void DoChapterTransition() override;
    void DoTeleport(BAK::Encounter::Teleport teleport) override;
    void ShowCharacterPortrait(BAK::ActiveCharIndex character) override;
    void ExitSimpleScreen() override;
    void ShowInventory(BAK::ActiveCharIndex character) override;
    void ShowContainer(BAK::GenericContainer* container, BAK::EntityType containerType) override;
    void EnterCombat(std::function<void(BAK::CombatResult)>&& finished) override;
    void ExitCombat(BAK::CombatResult) override;
    void SelectItem(
        std::function<void(
            std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>>)>&& itemSelected) override;
    void ExitInventory() override;
    void ShowLock(
        BAK::IContainer* container,
        std::function<void()>&& finished) override;
    void ShowCamp(bool isInn, BAK::ShopStats* inn) override;
    void ShowCast(bool inCombat) override;
    void ShowFullMap() override;
    void ShowGameStartMap() override;
    void ShowCureScreen(
        unsigned templeNumber,
        unsigned cureFactor,
        std::function<void()>&& finished) override;
    void ShowTeleport(unsigned sourceTemple, BAK::ShopStats* temple) override;
    void ExitLock() override;
    bool IsLockOpened() const override;
    bool IsWordLockOpened() const override;

    void PushScreen(Widget* screen);
    ScopeGuard<std::function<void()>> PopScreen();

    void SetCombatSequenceActive(bool isActive) override { mCombatSequenceActive = isActive; }
    bool GetCombatSequenceActive() { return mCombatSequenceActive; }
    void PopOnExitCallback(OnExit action);

    void SetCombatManager(BAK::ICombatManager& combatManager) override;
    BAK::ICombatUI& GetCombatUI() override;
    BAK::ICombatManager& GetCombatManager() override;

private:
    void CacheState();
    bool NotifyPartyChanges();
    void FadeInDone();
    void FadeOutDone();

    FontManager mFontManager;
    Actors mActors;
    Backgrounds mBackgrounds;
    Icons mIcons;

    Cursor& mCursor;
    BAK::GameState& mGameState;
    ScreenStack mScreenStack;
    DialogRunner mDialogRunner;

    Graphics::SpriteManager& mSpriteManager;

    CutscenePlayer mCutscenePlayer;
public:
    MainView mMainView;
private:
    MainMenuScreen mMainMenu;
    InfoScreen mInfoScreen;
    InventoryScreen mInventoryScreen;
    Camp::CampScreen mCampScreen;
    Cast::CastScreen mCastScreen;
    Combat::CombatScreen mCombatScreen;
    CureScreen mCureScreen;
    LockScreen mLockScreen;
public:
    FullMap mFullMap;
private:
    MoredhelScreen mMoredhelScreen;
    TeleportScreen mTeleportScreen;
    FadeScreen mFadeScreen;
    bool mDebugDisableFades{false};
    std::vector<std::function<void()>> mFadeFunction;
    std::function<void()> mEndFadeFunction;
    std::function<void()> mCutsceneFinished;
    std::function<void()> mOnEnterMainView;
    std::function<void(BAK::CombatResult)> mCombatFinishedCallback;
    std::vector<std::unique_ptr<GDSScene>> mGdsScenes;

    IDialogScene* mDialogScene;
    std::stack<std::function<void()>> mOnExitCallbacks;

    AnimatorStore mAnimatorStore;
    BAK::IZoneLoader* mZoneLoader;
    BAK::ICombatManager* mCombatManager{nullptr};
    Widget* mPreviousScreen{nullptr};

    BAK::Encounter::TeleportFactory mTeleportFactory{};

    bool mAmInMainView{false};
    bool mCombatSequenceActive{false};

    NullDialogScene mNullDialogScene{};
    DynamicDialogScene mPartyDiedScene;
    const Logging::Logger& mLogger;
};

}
