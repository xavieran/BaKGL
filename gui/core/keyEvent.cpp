#include "gui/core/keyEvent.hpp"

namespace Gui {

std::ostream& operator<<(std::ostream& os, const KeyPress& event)
{
    return os << "KeyPress {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const Character& event)
{
    return os << "Character {" << event.mValue << "}";
}

std::ostream& operator<<(std::ostream& os, const KeyEvent& event)
{
    return std::visit([&](const auto& e) -> std::ostream&
        {
            os << e;
            return os;
        },
        event);
}

}
