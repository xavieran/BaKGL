#include "bak/lua/core.hpp"
#include "bak/lua/types.hpp"

#include "bak/bard.hpp"
#include "bak/condition.hpp"
#include "bak/entityType.hpp"
#include "bak/gameState.hpp"
#include "bak/IContainer.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"
#include "bak/skills.hpp"
#include "com/logger.hpp"

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include <filesystem>
#include <fstream>

namespace BAK::Lua {

namespace {

void Sandbox(lua_State* luaState)
{
    luaL_requiref(luaState, "_G", luaopen_base, 1);
    lua_pop(luaState, 1);
    luaL_requiref(luaState, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(luaState, 1);
    luaL_requiref(luaState, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(luaState, 1);
    luaL_requiref(luaState, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(luaState, 1);

    lua_pushnil(luaState); lua_setglobal(luaState, "dofile");
    lua_pushnil(luaState); lua_setglobal(luaState, "loadfile");
    lua_pushnil(luaState); lua_setglobal(luaState, "require");
    lua_pushnil(luaState); lua_setglobal(luaState, "package");
    lua_pushnil(luaState); lua_setglobal(luaState, "io");
    lua_pushnil(luaState); lua_setglobal(luaState, "os");
}

}

lua_State*& GetLuaState()
{
    static lua_State* luaState = nullptr;
    return luaState;
}

void Initialize(const std::string& modsDir)
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    auto& log = Logging::LogState::GetLogger("Lua");

    auto& lua = GetLuaState();
    if (!std::filesystem::exists(modsDir))
    {
        log.Info() << "No mods directory: " << modsDir << "\n";
        return;
    }

    lua = luaL_newstate();
    Sandbox(lua);
    RegisterTypes(lua);

    for (const auto& entry : std::filesystem::directory_iterator(modsDir))
    {
        if (!entry.is_regular_file())
            continue;
        auto path = entry.path();
        if (path.extension() != ".lua")
            continue;

        if (luaL_loadfile(lua, path.string().c_str()) != LUA_OK
            || lua_pcall(lua, 0, 0, 0) != LUA_OK)
        {
            auto err = lua_tostring(lua, -1);
            log.Error() << "Failed to load mod: " << path.filename().string()
                << ": " << (err ? err : "unknown error") << "\n";
            lua_pop(lua, 1);
        }
        else
        {
            log.Info() << "Loaded mod: " << path.filename().string() << "\n";
        }
    }
}

}
