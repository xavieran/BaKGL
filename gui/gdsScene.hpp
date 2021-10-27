#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/colors.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/hotspot.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"
#include "gui/screenStack.hpp"
#include "gui/staticTTM.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

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
    
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

public:
    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot);
    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot);

    void StartDialog(BAK::Target target, bool isTooltip);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>&) override;
    void EnterContainer();

    BAK::HotspotRef mReference;
    BAK::GameState& mGameState;
    BAK::SceneHotspots mSceneHotspots;
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

    std::optional<BAK::Hotspot> mPendingInn;
    std::optional<BAK::HotspotRef> mPendingGoto;

    static constexpr auto mMaxSceneNesting = 4;

    const Logging::Logger& mLogger;
};

}
