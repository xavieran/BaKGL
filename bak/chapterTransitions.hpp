#pragma once

#include "bak/types.hpp"
#include <optional>

namespace BAK {

class GameState;
struct Teleport;

std::optional<Teleport> TransitionToChapter(Chapter chapter, GameState& gs);

}
