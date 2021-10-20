#include "gui/dragEvent.hpp"

#include "graphics/glm.hpp"

namespace Gui {

std::ostream& operator<<(std::ostream& os, const DragStarted& event)
{
    return os << "DragStarted { " << std::hex << event.mWidget << std::dec << " " << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const Dragging& event)
{
    return os << "Dragging { " << std::hex << event.mWidget << std::dec << " " << event.mValue << "}";
}


std::ostream& operator<<(std::ostream& os, const DragEnded& event)
{
    return os << "DragEnded { " << std::hex << event.mWidget << std::dec << " " << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const DragEvent& event)
{
    return std::visit([&](const auto& e) -> std::ostream&
        {
            os << e;
            return os;
        },
        event);
}

const glm::vec2& GetValue(const DragEvent& event)
{
    return std::visit([&](const auto& e) -> const glm::vec2&
        {
            return e.mValue;
        },
        event);
}

}
