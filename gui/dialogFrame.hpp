#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/widget.hpp"

namespace Gui {

class WorldDialogFrame : public Widget
{
public:
    WorldDialogFrame(
        const Backgrounds& backgrounds)
    :
        Widget{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Color::black,
            glm::vec2{0,0},
            glm::vec2{320, 200},
            true
        },
        mBackground{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("DIALOG.SCX"),
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0,0},
            glm::vec2{320, 200},
            true},
        mTop{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0,0},
            glm::vec2{320, 11},
            false},
        mBottom{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0, 110},
            glm::vec2{320, 200},
            false},
        mLeft{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{0, 0},
            glm::vec2{14, 200},
            false},
        mRight{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::Texture,
            Color::black,
            glm::vec2{290, 0},
            glm::vec2{320, 200},
            false}
    {
        AddChildBack(&mTop);
        AddChildBack(&mBottom);
        AddChildBack(&mLeft);
        AddChildBack(&mRight);
        mTop.AddChildBack(&mBackground);
        mBottom.AddChildBack(&mBackground);
        mLeft.AddChildBack(&mBackground);
        mRight.AddChildBack(&mBackground);
    }

private:
    Widget mBackground;
    Widget mTop;
    Widget mBottom;
    Widget mLeft;
    Widget mRight;
};


}
