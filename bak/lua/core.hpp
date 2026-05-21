#pragma once

#include <optional>
#include <string>

struct lua_State;

namespace BAK::Lua {

lua_State*& GetLuaState();
void Initialize(const std::string& modsDir);

}
