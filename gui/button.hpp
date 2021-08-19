#pragma once

#include "gui/widget.hpp"
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
        glm::vec4 dropShadow=Color::black)
    :
        // Bottom left edge
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            shadow,
            pos,
            dim,
            true
        },
        mTopRightEdge{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            highlight,
            glm::vec2{1, 0},
            dim - glm::vec2{1, 1},
            true
        },
        mCenter{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            mainColor,
            glm::vec2{1, 1},
            dim - glm::vec2{2, 2},
            true
        }
    {
        // Top Right edge
        this->AddChildBack(&mTopRightEdge);
        this->AddChildBack(&mCenter);
    }

    Widget mTopRightEdge;
    Widget mCenter;
};

}
