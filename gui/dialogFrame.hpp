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
            ClipRegionTag{},
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
        mFrame{
            RectTag{},
            glm::vec2{14, 10},
            glm::vec2{291, 103},
            Color::frameMaroon,
            false},
        mTop{
            ClipRegionTag{},
            glm::vec2{0,0},
            glm::vec2{320, 11},
            false},
        mBottom{
            ClipRegionTag{},
            glm::vec2{0, 112},
            glm::vec2{320, 200},
            false},
        mLeft{
            ClipRegionTag{},
            glm::vec2{0, 0},
            glm::vec2{15, 200},
            false},
        mRight{
            ClipRegionTag{},
            glm::vec2{304, 0},
            glm::vec2{320, 200},
            false}
    {
        AddChildBack(&mTop);
        AddChildBack(&mBottom);
        AddChildBack(&mLeft);
        AddChildBack(&mRight);
        mTop.AddChildBack(&mBackground);
        mTop.AddChildBack(&mFrame);
        mBottom.AddChildBack(&mBackground);
        mBottom.AddChildBack(&mFrame);
        mLeft.AddChildBack(&mBackground);
        mLeft.AddChildBack(&mFrame);
        mRight.AddChildBack(&mBackground);
        mRight.AddChildBack(&mFrame);
    }

private:
    Widget mBackground;
    Widget mFrame;
    Widget mTop;
    Widget mBottom;
    Widget mLeft;
    Widget mRight;
};


}
