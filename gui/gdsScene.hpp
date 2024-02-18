#pragma once

#include "bak/hotspot.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/sprites.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/dialogDisplay.hpp"
#include "gui/hotspot.hpp"
#include "gui/staticTTM.hpp"
#include "gui/temple.hpp"
#include "gui/repair.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class GDSScene : public Widget, public IDialogScene
{
public:

    GDSScene(
        Cursor& cursor,
        BAK::HotspotRef hotspotRef,
        Graphics::SpriteManager& spriteManager,
        const Actors& actors,
        const Backgrounds& backgrounds,
        const Font& font,
        BAK::GameState& gameState,
        IGuiManager& guiManager);
    
    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

    void EnterGDSScene();
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;
    const auto& GetSceneHotspots() const { return mSceneHotspots; }

public:
    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot, bool);
    void HandleHotspotLeftClicked2(const BAK::Hotspot& hotspot, bool);
    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot);

    void StartDialog(BAK::Target target, bool isTooltip);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>&) override;
    void AddStaticTTM(BAK::Scene scene1, BAK::Scene scene2);

    void EvaluateHotspotAction();

    void EnterContainer();
    void DoInn();
    void DoBard();
    void DoRepair();
    void DoTeleport();
    void DoTemple(BAK::KeyTarget);
    void DoGoto();
    void DoExit();

    const Font& mFont;
    BAK::HotspotRef mReference;
    BAK::GameState& mGameState;
    BAK::SceneHotspots mSceneHotspots;
    BAK::KeyTarget mFlavourText;

    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::SpriteManager& mSpriteManager;

    // Frame to surround the scene
    Widget mFrame;
    std::vector<StaticTTM> mStaticTTMs;

    std::vector<Hotspot> mHotspots;
    std::vector<bool> mHotspotClicked;

    Cursor& mCursor;
    IGuiManager& mGuiManager;
    DialogDisplay mDialogDisplay;

    std::optional<BAK::HotspotAction> mPendingAction;
    std::optional<BAK::HotspotRef> mPendingGoto;
    // e.g. when you fail barding
    bool mKickedOut;
    bool mBarding;
    
    Temple mTemple;
    Repair mRepair;

    static constexpr auto mMaxSceneNesting = 4;

    const Logging::Logger& mLogger;
};

}
