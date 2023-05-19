#pragma once

#include <ostream>
#include <variant>

namespace Gui {

struct KeyPress
{
    bool operator==(const KeyPress&) const = default;
    int mValue;
};

struct Character
{
    bool operator==(const Character&) const = default;
    char mValue;
};

using KeyEvent = std::variant<
    KeyPress,
    Character>;

std::ostream& operator<<(std::ostream& os, const KeyEvent&);

}
