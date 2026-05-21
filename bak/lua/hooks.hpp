#pragma once

#include "bak/dialogTarget.hpp"

#include <optional>

namespace BAK { class GameState; enum class ItemType; }

namespace BAK::Lua {

std::optional<KeyTarget> OnCantHaggleScroll(BAK::GameState& gameState);
std::optional<KeyTarget> OnHaggleFail(
    BAK::GameState& gameState,
    BAK::ItemType itemType);
std::optional<KeyTarget> OnHaggleSuccess(
    BAK::GameState& gameState,
    unsigned discountPct);

}
