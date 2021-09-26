#pragma once

#include <glm/glm.hpp>
#include <variant>

namespace Gui {

struct LeftMousePress { glm::vec2 mValue; };
struct LeftMouseRelease { glm::vec2 mValue; };
struct RightMousePress { glm::vec2 mValue; };
struct RightMouseRelease { glm::vec2 mValue; };
struct MouseMove { glm::vec2 mValue; };

using MouseEvent = std::variant<
    LeftMousePress,
    LeftMouseRelease,
    RightMousePress,
    RightMouseRelease,
    MouseMove>;

}
