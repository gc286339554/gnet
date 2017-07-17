#include "../../src/3rd/log/glog.h"
#include "../commom/pre_header.h"
#include "../extend/leaf_node/leaf_node.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"
#include "../data_packet_pool.hpp"


gnet::service_mgr mgr;
int main()
{
	gnet::glog::newInstance("test_game_client", "log");
	mgr.init(8);

	class main_game_client :public gnet::service
	{
	public:
		std::atomic<bool> m_is_connected = false;
		std::atomic<bool> m_is_auth = false;
		bool m_last_connect_state = false;
		gnet::leaf_node  my_leaf_node;
		main_game_client(const char* service_name) :
			gnet::service(service_name),
			my_leaf_node("127.0.0.1", 9528, 5)
		{
			std::function<void(gnet::data_packet*)> handler = std::bind(&main_game_client::net_msg_handler, this, std::placeholders::_1);
			my_leaf_node.set_net_msg_handler(handler);
			std::function<void(bool)> handler01 = std::bind(&main_game_client::session_state_change_handler, this, std::placeholders::_1);
			my_leaf_node.set_session_state_change_handler(handler01);
			my_leaf_node.start();
		}
		void net_msg_handler(gnet::data_packet* net_msg)
		{
			net_msg->start_read();
			uint32 op = net_msg->get_op();
			if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_END)
			{
				if (op == OP_SC_LOGIN)
				{
					if (net_msg->get_bool())
					{
						printf("login res is suc\n");
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write().set_op(OP_CS_GET_BASE_INFO).end_write();

						my_leaf_node.get_session()->do_write(packet);

						m_is_auth = true;

					}	
					else
						printf("login res is fail\n");
				}
				else if (op == OP_SC_GET_BASE_INFO )
				{
					printf("base name is %s\n", net_msg->get_string().c_str());
					printf("base sex is %s\n", net_msg->get_string().c_str());
				}
			}
			g_data_packet_pool.put_data_packet(net_msg);
		}
		void session_state_change_handler(bool now_is_connect)
		{//ÍøÂçÏß³Ì
			if (now_is_connect)
			{
				m_is_connected = true;
				printf("session connected ...\n");
			}
			else
			{
				m_is_connected = false;
				printf("session disconnected ...\n");
			}
		}
		virtual void update(uint32 ms_delay)
		{
			if (m_last_connect_state != m_is_connected)
			{
				m_last_connect_state = m_is_connected;
				if (m_is_connected)
				{
					{
						{
							gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
							packet->start_write().set_op(OP_CS_LOGIN).put("test_account").put("test_password").end_write();

							my_leaf_node.get_session()->do_write(packet);
						}
					}
				}
				else
				{
				}
			}

			if (m_is_connected && m_is_auth)
			{
				if (rand() % 100 == rand() % 100)
				{
					if (my_leaf_node.get_session()->get_send_list_size() < 10)
					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write().set_op(OP_CS_GET_BASE_INFO).end_write();
						my_leaf_node.get_session()->do_write(packet);
					}
				}
			}
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	char buff[125] = {0};
	for (size_t i = 0; i < 1000; i++)
	{
		sprintf(buff,"main_game_client_%d",i);
		mgr.add_service(std::make_shared<main_game_client>(buff));
	}
	//mgr.add_service(std::make_shared<main_game_client>("main_game_client"));
	mgr.start();

	getchar();
	gnet::glog::deleteInstance();
}
