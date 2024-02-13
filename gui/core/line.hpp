#pragma once

#include "gui/core/widget.hpp"

#include "graphics/glm.hpp"

#include "com/logger.hpp"

#include <cmath>

namespace Gui
{

class Line : public Widget
{
public:
    Line(glm::vec2 p1, glm::vec2 p2, float width)
    :
        Widget{
            RectTag{},
            {},
            {},
            {},
            true},
        mStart{p1},
        mEnd{p2},
        mWidth{width}
    {
        CalculateLine();
    }

    void SetPoints(glm::vec2 start, glm::vec2 end)
    {
        mStart = start;
        mEnd = end;
        CalculateLine();
    }

private:
    void CalculateLine()
    {
        auto length = glm::distance(mStart, mEnd);
        auto rotation = -atanf((mEnd.y  - mStart.y) / (mEnd.x - mStart.x));
        if (std::abs(mEnd.x - mStart.x) < .01)
            rotation = -1.5708;
        SetColor(glm::vec4{1, 0, 0, 1});
        SetPosition((mStart.x > mEnd.x) ? mEnd : mStart);
        SetDimensions(glm::vec2{length, mWidth});
        SetRotation(rotation);
    }

    glm::vec2 mStart;
    glm::vec2 mEnd;
    float mWidth;
};

}
