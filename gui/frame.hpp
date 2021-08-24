#pragma once

#include "gui/widget.hpp"
#include "gui/colors.hpp"

namespace Gui {

class Frame : public Widget
{
public:
    Frame(
        glm::vec2 pos,
        glm::vec2 dims)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            //Color::debug,
            glm::vec4{0},
            pos,
            dims,
            true
        }
    {
    }
};

}
