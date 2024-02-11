#pragma once

#include "gui/core/widget.hpp"


namespace Gui
{

class Line
{
public:
    Line(glm::vec2 p1, glm::vec2 p2)
    :
        mStart{p1},
        mEnd{p2}
    {
    }

private:
    glm::vec2 mStart;
    glm::vec2 mEnd;
};

}
