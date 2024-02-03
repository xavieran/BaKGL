#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class WorldDialogFrame : public Widget
{
public:
    WorldDialogFrame(
        const Backgrounds& backgrounds)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("DIALOG_BG_MAIN.SCX"),
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0,0},
            glm::vec2{320, 200},
            true}
    {
    }
};


}
