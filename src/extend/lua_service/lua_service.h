#pragma once
#include "../commom/service/service.h"
extern "C"
{
#include "../3rd/lua/lua.h"
#include "../3rd/lua/lualib.h"
#include "../3rd/lua/lauxlib.h"
};
namespace gnet {
class lua_service : public service
{
public:
	lua_service(const char* service_name, const char* lua_enter_file);
	~lua_service();

	virtual void init();
	virtual void update(uint32 ms_delay);
	virtual void service_msg_handler(std::shared_ptr<service_msg>& msg_sp);
public:
	std::string m_lua_bridge_str;//通信中转桥梁作用的string,临时方案  不太优雅
protected:
	lua_State* L = nullptr;
	std::string m_lua_enter_file;
	
};
};