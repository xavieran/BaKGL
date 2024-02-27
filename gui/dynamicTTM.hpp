#pragma once

#include "bak/scene.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"

#include "gui/scene.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class DynamicTTM 
{
public:
    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        const BAK::Scene& sceneInit,
        const BAK::Scene& sceneContent);

    Widget* GetScene();
    Widget* GetBackground();

private:
    Graphics::SpriteManager::TemporarySpriteSheet mSpriteSheet;
    Widget mSceneFrame;
    std::optional<Widget> mDialogBackground;

    std::vector<Widget> mSceneElements;
    std::optional<Widget> mClipRegion;

    const Logging::Logger& mLogger;
};

}
