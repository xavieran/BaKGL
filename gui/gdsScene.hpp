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

    void SetTempleSeen();
    void EnterGDSScene();
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;
    auto GetSong() const { return mSong; }

public:
    enum class State
    {
        Idle,
        Inn,
        Repair,
        Container,
        Goto,
        Bard,
        Teleport,
        Dialog
    };

    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot);
    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot);

    void StartDialog(BAK::Target target, bool isTooltip);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>&) override;
    void EnterContainer();
    void DoBard();

    const Font& mFont;
    BAK::HotspotRef mReference;
    BAK::GameState& mGameState;
    BAK::SceneHotspots mSceneHotspots;
    unsigned mSong;
    BAK::Target mFlavourText;

    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::SpriteManager& mSpriteManager;

    // Frame to surround the scene
    Widget mFrame;
    std::vector<StaticTTM> mStaticTTMs;

    std::vector<Hotspot> mHotspots;

    Cursor& mCursor;
    IGuiManager& mGuiManager;
    DialogDisplay mDialogDisplay;

    State mState;
    std::optional<BAK::Hotspot> mPendingInn;
    std::optional<BAK::HotspotRef> mPendingGoto;
    // e.g. when you fail barding
    bool mKickedOut;
    
    Temple mTemple;
    Repair mRepair;

    static constexpr auto mMaxSceneNesting = 4;

    const Logging::Logger& mLogger;
};

}
