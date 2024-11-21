#include "gui/core/line.hpp"

#include <cmath>

namespace Gui {

Line::Line(glm::vec2 p1, glm::vec2 p2, float width, glm::vec4 color)
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
    SetColor(color);
}

void Line::SetPoints(glm::vec2 start, glm::vec2 end)
{
    mStart = start;
    mEnd = end;
    CalculateLine();
}

glm::vec2 Line::GetStart() const { return mStart; }
glm::vec2 Line::GetEnd() const { return mEnd; }

void Line::CalculateLine()
{
    auto length = glm::distance(mStart, mEnd);
    auto rotation = -atanf((mEnd.y  - mStart.y) / (mEnd.x - mStart.x));
    if (std::abs(mEnd.x - mStart.x) < .001)
        rotation = -1.5708;
    SetPosition((mStart.x > mEnd.x) ? mEnd : mStart);
    SetDimensions(glm::vec2{length, mWidth});
    SetRotation(rotation);
}

}
