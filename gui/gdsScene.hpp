#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/dialogRunner.hpp"
#include "gui/hotspot.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"
#include "gui/staticTTM.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GDSScene : public Widget
{
public:

    GDSScene(
        Cursor& cursor,
        BAK::HotspotRef hotspotRef,
        Graphics::SpriteManager& spriteManager,
        DialogRunner& dialogRunner);
    
    void SetDisplayScene();
    void SetDisplayBackground();

    void LeftMousePress(glm::vec2 pos) override;
    void MouseMoved(glm::vec2 pos) override;

    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot);
    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot);

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

public:
//private:
    struct DialogState
    {
        void ActivateDialog()
        {
            mDialogActive = true;
        }

        template <typename F>
        bool RunDialog(F&& f)
        {
            if (mDialogActive)
            {
                mDialogActive = f();
            }
            return mDialogActive;
        }

        void ActivateTooltip(glm::vec2 pos)
        {
            mTooltipPos = pos;
            mTooltipActive = true;
        }

        template <typename F>
        void DeactivateTooltip(glm::vec2 pos, F&& f)
        {
            constexpr auto tooltipSensitivity = 15;
            if (mTooltipActive 
                && glm::distance(pos, mTooltipPos) > tooltipSensitivity)
            {
                f();
                mTooltipActive = false;
            }
        }

        bool mDialogActive;
        bool mTooltipActive;
        glm::vec2 mTooltipPos;
    };

    BAK::HotspotRef mReference;
    BAK::SceneHotspots mSceneHotspots;
    BAK::Target mFlavourText;

    // Frame to surround the scene
    Graphics::SpriteSheetIndex mSpriteSheet;
    Graphics::SpriteManager& mSpriteManager;
    Widget mFrame;
    std::vector<StaticTTM> mStaticTTMs;

    glm::vec2 mMousePos;

    std::vector<Hotspot> mHotspots;

    Cursor& mCursor;
    DialogRunner& mDialogRunner;
    DialogState mDialogState;

    static constexpr auto mMaxSceneNesting = 4;

    const Logging::Logger& mLogger;
};

}
