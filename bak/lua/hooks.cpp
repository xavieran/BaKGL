#include "bak/lua/hooks.hpp"
#include "bak/lua/core.hpp"

#include "bak/gameState.hpp"
#include "com/logger.hpp"

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include <utility>

namespace BAK::Lua {

namespace {

template <typename... Args>
std::optional<KeyTarget> CallHook(
    const std::string& hookName,
    Args&&... args)
{
    auto& lua = GetLuaState();
    if (!lua) return std::nullopt;

    auto& log = Logging::LogState::GetLogger("Lua");

    auto hook = luabridge::getGlobal(lua, hookName.c_str());
    if (!hook.isFunction()) return std::nullopt;

    auto result = hook.call<int>(std::forward<Args>(args)...);
    if (result)
        return KeyTarget{static_cast<std::uint32_t>(result.value())};

    return std::nullopt;
}

}

std::optional<KeyTarget> OnCantHaggleScroll(BAK::GameState& gameState)
{
    return CallHook("on_cant_haggle_scroll", gameState);
}

std::optional<KeyTarget> OnHaggleFail(
    BAK::GameState& gameState,
    BAK::ItemType itemType)
{
    return CallHook("on_haggle_fail", gameState, std::to_underlying(itemType));
}

std::optional<KeyTarget> OnHaggleSuccess(
    BAK::GameState& gameState,
    unsigned discountPct)
{
    return CallHook("on_haggle_success", gameState, discountPct);
}

}
