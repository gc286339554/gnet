#include "../../src/3rd/log/glog.h"
#include "../commom/pre_header.h"
#include "../extend/leaf_node/leaf_node.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"
#include "../data_packet_pool.hpp"


gnet::service_mgr mgr;
int main()
{
	gnet::glog::newInstance("leaf_node", "log");
	mgr.init(8);
	
	class main_service_client :public gnet::service
	{
	public:
		gnet::leaf_node  my_leaf_node;
		main_service_client(const char* service_name) :
			gnet::service(service_name),
			my_leaf_node("127.0.0.1", 9527, 10)
			//my_leaf_node("192.168.7.132", 7001, 5)//192.168.7.164
		{
			std::function<void(gnet::data_packet*)> handler = std::bind(&main_service_client::net_msg_handler, this, std::placeholders::_1);
			my_leaf_node.set_net_msg_handler(handler);
			std::function<void(bool)> handler01 = std::bind(&main_service_client::session_state_change_handler, this, std::placeholders::_1);
			my_leaf_node.set_session_state_change_handler(handler01);
			my_leaf_node.start();
		}
		void net_msg_handler(gnet::data_packet* net_msg)
		{
			//main_service_client*
			net_msg->start_read();
			
			printf("get a full packet from server num is: %d ...\n", net_msg->get_uint32());
			g_data_packet_pool.put_data_packet(net_msg);
		}
		void session_state_change_handler(bool now_is_connect)
		{//ÍøÂçÏß³Ì
			if (now_is_connect)
				printf("session connected ...\n");
			else
				printf("session disconnected ...\n");
		}
		virtual void update(uint32 ms_delay)
		{
			static uint32 index = 0;
			index++;
			if (index % 10000 == 0)
			{
				gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
				packet->start_write();
				packet->set_op(100);
				packet->put_uint32(index);
				packet->flip();

				my_leaf_node.get_session()->do_write(packet);
			}
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	mgr.add_service(std::make_shared<main_service_client>("main_service_client"));
	mgr.start();

	getchar();
	gnet::glog::deleteInstance();
}
