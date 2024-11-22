#pragma once

#include "bak/coordinates.hpp"

#include "gui/core/widget.hpp"

namespace Gui {

class Compass : public Widget
{
public:
    Compass(
        glm::vec2 pos,
        glm::vec2 dims,
        glm::vec2 compassSheetDims,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex texture);
    
    void SetHeading(BAK::GameHeading gameHeading);
private:
    void UpdateCompassHeading(Widget& compass, double zeroedXPos);

    Widget mCompassLeft;
    Widget mCompassCenter;

    glm::vec2 mCompassDims;

    double mHeading;
};

}
