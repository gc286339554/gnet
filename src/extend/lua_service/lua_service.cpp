#include "lua_service.h"
#include "../3rd/lua_tinker/lua_tinker.h"

static void dp_start_write(uint64 msg_p);
static void dp_start_read(uint64 msg_p);
static uint32 dp_get_op(uint64 msg_p);
static void dp_set_op(uint64 msg_p, f64 val);
static void dp_flip(uint64 msg_p);
static uint64 dp_malloc_data_packet();
static void dp_free_data_packet(uint64 msg_p);
static void dp_put_num(uint64 msg_p, f64 val);
static f64 dp_get_num(uint64 msg_p);
static void dp_put_bool(uint64 msg_p, bool val);
static bool dp_get_bool(uint64 msg_p);
static const char* dp_get_string(uint64 msg_p, uint64 g_service_this);
static void dp_put_string(uint64 msg_p, const char* val);
static void dp_send_lua_msg(const char* service_name, const char* msg_table, uint64 g_service_this);
static void dp_send_binary_msg(const char* service_name, uint64 msg_p, uint64 g_service_this);

using namespace gnet;
lua_service::lua_service(const char* service_name, const char* lua_enter_file)
:service(service_name),
m_lua_enter_file(lua_enter_file)
{
}
lua_service::~lua_service()
{
	if (L)
	{
		lua_close(L);
		L = nullptr;
	}
}
void lua_service::init()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	
	lua_tinker::def(L, "dp_start_write", dp_start_write);
	lua_tinker::def(L, "dp_start_read", dp_start_read);
	lua_tinker::def(L, "dp_get_op", dp_get_op);
	lua_tinker::def(L, "dp_set_op", dp_set_op);
	lua_tinker::def(L, "dp_flip", dp_flip);
	lua_tinker::def(L, "dp_malloc_data_packet", dp_malloc_data_packet);
	lua_tinker::def(L, "dp_free_data_packet", dp_free_data_packet);
	lua_tinker::def(L, "dp_put_num", dp_put_num);
	lua_tinker::def(L, "dp_get_num", dp_get_num);
	lua_tinker::def(L, "dp_put_bool", dp_put_bool);
	lua_tinker::def(L, "dp_get_bool", dp_get_bool);
	lua_tinker::def(L, "dp_put_string", dp_put_string);
	lua_tinker::def(L, "dp_get_string", dp_get_string);
	lua_tinker::def(L, "dp_send_lua_msg", dp_send_lua_msg);
	lua_tinker::def(L, "dp_send_binary_msg", dp_send_binary_msg);
	
	uint64 t_this = reinterpret_cast<uint64>(this);
	lua_tinker::decl(L, "g_service_this", t_this);
	lua_tinker::dofile(L, m_lua_enter_file.c_str());
	
}
void lua_service::update(uint32 ms_delay)
{
	lua_tinker::call<void, uint32>(L, "update", ms_delay);
}
void lua_service::service_msg_handler(std::shared_ptr<service_msg>& msg_sp)
{
	msg_sp->get_data_packet_p()->start_read();
	if (msg_sp->get_msg_type() == msg_type_lua)
	{
		lua_tinker::call<void, uint32, uint64>(L, "dp_lua_service_msg_handler", msg_sp->get_source_service_id(), reinterpret_cast<uint64>(msg_sp->get_data_packet_p()));
	}
	else if (msg_sp->get_msg_type() == msg_type_binary)
	{
		lua_tinker::call<void, uint32, uint64>(L, "dp_binary_service_msg_handler", msg_sp->get_source_service_id(), reinterpret_cast<uint64>(msg_sp->get_data_packet_p()));
	}
}
static void dp_start_write(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->start_write();
}
static void dp_start_read(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->start_read();
}
static uint32 dp_get_op(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	return msg->get_op();
}
static void dp_set_op(uint64 msg_p, f64 val)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->set_op(val);
}
static void dp_flip(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->flip();
}
static uint64 dp_malloc_data_packet()
{
	return reinterpret_cast<uint64>(g_data_packet_pool.get_data_packet());
}
static void dp_free_data_packet(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	g_data_packet_pool.put_data_packet(msg);
}
static void dp_put_num(uint64 msg_p, f64 val)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->put_f64(val);
}
static f64 dp_get_num(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	return msg->get_f64();
}
static void dp_put_bool(uint64 msg_p, bool val)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->put_bool(val);
}
static bool dp_get_bool(uint64 msg_p)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	return msg->get_bool();
}
static void dp_put_string(uint64 msg_p, const char* val)
{
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	msg->put_string(val);
}
static const char* dp_get_string(uint64 msg_p, uint64 g_service_this)
{
	lua_service* t_service_this = reinterpret_cast<lua_service*>(g_service_this);
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	t_service_this->m_lua_bridge_str = msg->get_string();
	return t_service_this->m_lua_bridge_str.c_str();
}
static void dp_send_lua_msg(const char* service_name, const char* msg_table, uint64 g_service_this)
{
	lua_service* t_service_this = reinterpret_cast<lua_service*>(g_service_this);
	std::shared_ptr<gnet::service_msg> msg_sp = std::make_shared<gnet::service_msg>();
	msg_sp->set_msg_type(msg_type_lua);
	msg_sp->get_data_packet_p()->start_write();
	msg_sp->get_data_packet_p()->put_string(msg_table);
	msg_sp->get_data_packet_p()->flip();
	t_service_this->send_msg(msg_sp, service_name);
}
static void dp_send_binary_msg(const char* service_name, uint64 msg_p, uint64 g_service_this)
{
	lua_service* t_service_this = reinterpret_cast<lua_service*>(g_service_this);
	data_packet* msg = reinterpret_cast<data_packet*>(msg_p);
	std::shared_ptr<gnet::service_msg> msg_sp = std::make_shared<gnet::service_msg>(msg);
	msg_sp->get_data_packet_p()->start_write();
	msg_sp->get_data_packet_p()->flip();
	t_service_this->send_msg(msg_sp, service_name);
}