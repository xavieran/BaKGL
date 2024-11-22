#include "gui/button.hpp"

#include "gui/core/widget.hpp"

namespace Gui {

Button::Button(
    glm::vec2 pos,
    glm::vec2 dim,
    glm::vec4 mainColor,
    glm::vec4 highlight,
    glm::vec4 shadow,
    glm::vec4 dropShadow)
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

void Button::SetDimensions(glm::vec2 dims)
{
    Widget::SetDimensions(dims);
    mTopRightEdge.SetDimensions(dims - glm::vec2{1});
    mCenter.SetDimensions(dims - glm::vec2{2});
}

}
