#pragma once

#include "gui/core/widget.hpp"
#include "gui/colors.hpp"

namespace Gui {

class Button : public Widget

{
public:
    Button(
        glm::vec2 pos,
        glm::vec2 dim,
        glm::vec4 mainColor,
        glm::vec4 highlight,
        glm::vec4 shadow,
        glm::vec4 dropShadow=Color::black);
    
    void SetDimensions(glm::vec2 dims) override;

    Widget mTopRightEdge;
    Widget mCenter;
};

}
