#include "lua_tinker_ex.h"

namespace lua_tinker
{
	template<>
	void push(lua_State *L, std::string ret)
	{
		lua_pushlstring(L, ret.c_str(), ret.size());
	}
	template<>
		void push(lua_State *L, std::string* ret)
	{
		lua_pushlstring(L, ret->c_str(), ret->size());
	}
	template<>
	void push(lua_State *L, const std::string* ret)
	{
		lua_pushlstring(L, ret->c_str(), ret->size());
	}
	template<>
	std::string read(lua_State *L, int index)
	{
		size_t len = 0;
		const char* sz = lua_tolstring(L, index, &len);
		return std::string(sz, len);
	}
}
