#pragma once

#include <glm/glm.hpp>

#include <ostream>
#include <variant>

namespace Gui {

struct LeftMousePress
{
    bool operator==(const LeftMousePress&) const = default;
    glm::vec2 mValue;
};

struct LeftMouseRelease
{
    bool operator==(const LeftMouseRelease&) const = default;
    glm::vec2 mValue;
};

struct RightMousePress
{
    bool operator==(const RightMousePress&) const = default;
    glm::vec2 mValue;
};

struct RightMouseRelease
{
    bool operator==(const RightMouseRelease&) const = default;
    glm::vec2 mValue;
};

struct MouseMove
{
    bool operator==(const MouseMove&) const = default;
    glm::vec2 mValue;
};

struct MouseScroll
{
    bool operator==(const MouseScroll&) const = default;
    glm::vec2 mValue;
};

using MouseEvent = std::variant<
    LeftMousePress,
    LeftMouseRelease,
    RightMousePress,
    RightMouseRelease,
    MouseMove,
    MouseScroll>;

std::ostream& operator<<(std::ostream& os, const MouseEvent&);

const glm::vec2& GetValue(const MouseEvent& event);

}
