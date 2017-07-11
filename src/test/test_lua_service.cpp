#include "../commom/pre_header.h"
#include "../extend/lua_service/lua_service.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"
#include "../commom/service/service_msg.hpp"
#include "../data_packet_pool.hpp"
#include "performance/execute_timer.hpp"

gnet::service_mgr mgr;
int main()
{
	do 
	{
		execute_timer t_execute_timer("vector");
		std::vector<int> sss;
		for (int i = 0; i < 2000; i++)
		{
			sss.push_back(i);
		}
		t_execute_timer.print_time();
		for (int i = 0; i < 2000; i++)
		{
			sss.pop_back();
		}
		t_execute_timer.print_time();
	} while (0);

	mgr.init(8);
	class second_service : public gnet::service
	{
	public:

		second_service(const char* service_name) :
			gnet::service(service_name)
		{

		}
		~second_service()
		{
		}

		virtual void update(uint32 ms_delay)
		{
			static uint32 index = 0;
			index++;
			if (index % 1000 == 0 && index < 15000)
			{
				std::shared_ptr<gnet::service_msg> msg_sp = std::make_shared<gnet::service_msg>();
				msg_sp->get_data_packet_p()->start_write().put_uint32(index).end_write();
				send_msg(msg_sp, "lua_service");
				char buff[128] = { 0 };
				sprintf(buff, "%d send_msg to lua", get_service_id());
				printf(buff);
			}
			gnet::data_packet_pool* p = gnet::data_packet_pool::instance();
			p = p;
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	mgr.add_service(std::make_shared<gnet::lua_service>("lua_service","lua_script/test_lua_service.lua"));
	mgr.add_service(std::make_shared<gnet::lua_service>("lua_service_01", "lua_script/test_lua_service_01.lua"));
	char buff[128] = {0};
	for (int i=0; i<200; i++)
	{
		//sprintf(buff, "second_service_%d",i);
		//mgr.add_service(std::make_shared<second_service>(buff));
	}
	mgr.start();

	getchar();

}