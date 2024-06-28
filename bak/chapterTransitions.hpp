#pragma once

#include "bak/types.hpp"
#include "bak/dialogAction.hpp"

#include <optional>

namespace BAK {

class GameState;

std::optional<BAK::Teleport> TransitionToChapter(Chapter chapter, GameState& gs);

}
