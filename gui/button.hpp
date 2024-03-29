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
        glm::vec4 dropShadow=Color::black)
    :
        // Bottom left edge
        Widget{
            RectTag{},
            pos,
            dim,
            shadow,
            true
        },
        mTopRightEdge{
            RectTag{},
            glm::vec2{1, 0},
            dim - glm::vec2{1, 1},
            highlight,
            true
        },
        mCenter{
            RectTag{},
            glm::vec2{1, 1},
            dim - glm::vec2{2, 2},
            mainColor,
            true
        }
    {
        // Top Right edge
        AddChildBack(&mTopRightEdge);
        AddChildBack(&mCenter);
    }

    void SetDimensions(glm::vec2 dims) override
    {
        Widget::SetDimensions(dims);
        mTopRightEdge.SetDimensions(dims - glm::vec2{1});
        mCenter.SetDimensions(dims - glm::vec2{2});
    }

    Widget mTopRightEdge;
    Widget mCenter;
};

}
