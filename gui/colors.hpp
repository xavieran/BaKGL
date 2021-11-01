#pragma once


#include <glm/glm.hpp>

namespace Gui::Color {

constexpr auto black            = glm::vec4{0.00, 0.00, 0.00, 1};

constexpr auto debug            = glm::vec4{0.00, 0.00, 0.00, .4};

constexpr auto frameMaroon      = glm::vec4{.300, .110, .094, 1};

constexpr auto fontHighlight    = glm::vec4{.859, .780, .475, 1};
constexpr auto fontLowlight     = glm::vec4{.302, .110, .094, 1};
constexpr auto fontUnbold       = glm::vec4{.333, .271, .173, 1};
constexpr auto fontEmphasis     = glm::vec4{.094, .125, .204, 1};

constexpr auto fontWhiteHighlight = glm::vec4{.969, .780, .651, 1};
constexpr auto fontRedHighlight   = glm::vec4{.620, .188, .188, 1};
constexpr auto fontRedLowlight    = glm::vec4{.255, .016, .031, 1};

constexpr auto infoBackground   = glm::vec4{.125, .110, .094, 1};

constexpr auto buttonBackground = glm::vec4{.604, .427, .220, 1};
constexpr auto buttonPressed    = glm::vec4{.573, .380, .204, 1};
constexpr auto buttonShadow     = glm::vec4{.333, .271, .173, 1};
constexpr auto buttonHighlight  = glm::vec4{.651, .573, .255, 1};

constexpr auto itemHighlighted  = glm::vec4{.490, .063, .031, .5};

}
