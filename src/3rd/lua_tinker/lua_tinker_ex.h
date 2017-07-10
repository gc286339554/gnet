#pragma once

extern "C"
{
#include "../lua/lua.h"
#include "../lua/lualib.h"
#include "../lua/lauxlib.h"
};
#include "lua_tinker.h"
#include <string>

namespace lua_tinker
{
	template<>	void push(lua_State *L, std::string ret);
	template<>	void push(lua_State *L, std::string* ret);
	template<>	void push(lua_State *L, const std::string* ret);
	template<>	std::string read(lua_State *L, int index);
}
