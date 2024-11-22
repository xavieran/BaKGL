#pragma once

#include "gui/core/widget.hpp"

namespace Gui
{

class Line : public Widget
{
public:
    Line(glm::vec2 p1, glm::vec2 p2, float width, glm::vec4 color);
    void SetPoints(glm::vec2 start, glm::vec2 end);

    glm::vec2 GetStart() const;
    glm::vec2 GetEnd() const;

private:
    void CalculateLine();

    glm::vec2 mStart;
    glm::vec2 mEnd;
    float mWidth;
};

}
