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
        ScreenStack& screenStack,
        IGuiManager& guiManager);
    
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;

    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot);
    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot);

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

public:
    void StartDialog(BAK::Target target, bool isTooltip);
    void FinishedDialog();

    BAK::HotspotRef mReference;
    BAK::SceneHotspots mSceneHotspots;
    BAK::Target mFlavourText;

    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::SpriteManager& mSpriteManager;

    // Frame to surround the scene
    Widget mFrame;
    std::vector<StaticTTM> mStaticTTMs;

    std::vector<Hotspot> mHotspots;

    Cursor& mCursor;
    ScreenStack& mScreenStack;
    IGuiManager& mGuiManager;
    DialogDisplay mDialogDisplay;
    DialogRunner mDialogRunner;

    static constexpr auto mMaxSceneNesting = 4;

    const Logging::Logger& mLogger;
};

}
