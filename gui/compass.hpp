#pragma once

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

namespace Gui {

class Compass : public Widget
{
public:
    Compass(
        glm::vec2 pos,
        glm::vec2 dims,
        glm::vec2 compassSheetDims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex texture)
    :
        Widget{
            Graphics::DrawMode::ClipRegion,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Color::black,
            pos,
            dims,
            true
        },
        mCompass{
            Graphics::DrawMode::Sprite,
            spriteSheet,
            texture,
            Graphics::ColorMode::Texture,
            Color::debug,
            glm::vec2{0},
            compassSheetDims,
            true},
        mHeading{0}
    {
        AddChildBack(&mCompass);
    }

    void SetHeading(double heading)
    {
        assert(0.0 <= heading && heading <= 1.0);
        mHeading = heading;
        UpdateCompassHeading();
    }

    void UpdateCompassHeading()
    {
    }

private:
    Widget mCompass;

    double mHeading;
};

}
