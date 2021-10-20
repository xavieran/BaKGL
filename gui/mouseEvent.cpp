#include "gui/mouseEvent.hpp"

#include "graphics/glm.hpp"

namespace Gui {

std::ostream& operator<<(std::ostream& os, const LeftMousePress& event)
{
    return os << "LeftMousePress {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const LeftMouseRelease& event)
{
    return os << "LeftMouseRelease {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const RightMousePress& event)
{
    return os << "RightMousePress {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const RightMouseRelease& event)
{
    return os << "RightMouseRelease {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const MouseMove& event)
{
    return os << "MouseMove {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const MouseEvent& event)
{
    return std::visit([&](const auto& e) -> std::ostream&
        {
            os << e;
            return os;
        },
        event);
}

const glm::vec2& GetValue(const MouseEvent& event)
{
    return std::visit([&](const auto& e) -> const glm::vec2&
        {
            return e.mValue;
        },
        event);
}

}
