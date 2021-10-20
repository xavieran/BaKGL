#pragma once

#include <glm/glm.hpp>

#include <ostream>
#include <variant>

namespace Gui {

class Widget;

struct DragStarted
{
    bool operator==(const DragStarted&) const = default;
    Widget* mWidget;
    glm::vec2 mValue;
};

struct Dragging
{
    bool operator==(const Dragging&) const = default;
    Widget* mWidget;
    glm::vec2 mValue;
};

struct DragEnded
{
    bool operator==(const DragEnded&) const = default;
    Widget* mWidget;
    glm::vec2 mValue;
};

using DragEvent = std::variant<
    DragStarted,
    Dragging,
    DragEnded
    >;

std::ostream& operator<<(std::ostream& os, const DragEvent&);

const glm::vec2& GetValue(const DragEvent& event);


}
