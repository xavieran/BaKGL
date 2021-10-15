#pragma once

#include "bak/coordinates.hpp"

#include "com/logger.hpp"

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
            ClipRegionTag{},
            pos,
            dims,
            true
        },
        mCompassLeft{
            Graphics::DrawMode::Sprite,
            spriteSheet,
            texture,
            Graphics::ColorMode::Texture,
            Color::debug,
            glm::vec2{-compassSheetDims.x, 0},
            compassSheetDims,
            true},
        mCompassCenter{
            Graphics::DrawMode::Sprite,
            spriteSheet,
            texture,
            Graphics::ColorMode::Texture,
            Color::debug,
            glm::vec2{0},
            compassSheetDims,
            true},
        mCompassDims{compassSheetDims},
        mHeading{0}
    {
        AddChildBack(&mCompassLeft);
        AddChildBack(&mCompassCenter);
    }

    void SetHeading(BAK::GameHeading gameHeading)
    {
        const double heading = static_cast<double>(gameHeading) / static_cast<double>(0xff);
        mHeading = heading;
        UpdateCompassHeading(mCompassLeft, -mCompassDims.x);
        UpdateCompassHeading(mCompassCenter, 0);
    }

    void UpdateCompassHeading(Widget& compass, double zeroedXPos)
    {
        const auto newX = mHeading * compass.GetPositionInfo().mDimensions.x;
        Logging::LogDebug("Compass") << "Heading: " << mHeading << " pos: " << newX << "\n";
        compass.SetPosition(
            glm::vec2{
                newX + zeroedXPos,
                0});
    }

private:
    Widget mCompassLeft;
    Widget mCompassCenter;

    glm::vec2 mCompassDims;

    double mHeading;
};

}
