#pragma once
#include "bak/scene/scene.hpp"

#include "com/logger.hpp"

#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"

#include <optional>

namespace Gui {

/*
 * Display a static TTM, e.g. Lamut, Inn, etc.
 * */
class StaticTTM 
{
public:
    StaticTTM(
        Graphics::SpriteManager& spriteManager,
        const BAK::Script& sceneInit,
        const BAK::Script& sceneContent);

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
