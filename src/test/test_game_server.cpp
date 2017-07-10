#include "../../src/3rd/log/glog.h"
#include "../commom/pre_header.h"
#include "../extend/leaf_node/leaf_node.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"
#include "../data_packet_pool.hpp"


gnet::service_mgr mgr;
int main()
{
	gnet::glog::newInstance("test_game_server", "log");
	mgr.init(8);

	class main_game_server_01 :public gnet::service
	{
	public:
		std::atomic<bool> m_is_connected = false;
		bool m_last_connect_state = false;
		gnet::leaf_node  my_leaf_node;
		main_game_server_01(const char* service_name) :
			gnet::service(service_name),
			my_leaf_node("127.0.0.1", 9527, 10000)
		{
			std::function<void(gnet::data_packet*)> handler = std::bind(&main_game_server_01::net_msg_handler, this, std::placeholders::_1);
			my_leaf_node.set_net_msg_handler(handler);
			std::function<void(bool)> handler01 = std::bind(&main_game_server_01::session_state_change_handler, this, std::placeholders::_1);
			my_leaf_node.set_session_state_change_handler(handler01);
			my_leaf_node.start();
		}
		void net_msg_handler(gnet::data_packet* net_msg)
		{
			net_msg->start_read();
			uint32 op = net_msg->get_op();
			if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_END)
			{
				uint32 sid = net_msg->get_uint32();
				if (op == OP_CS_LOGIN)
				{
					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SG_CLIENT_AUTH);
						packet->put_uint32(sid);
						packet->put_bool(true);
						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}

					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SC_LOGIN);
						packet->put_uint32(sid);
						packet->put_bool(true);
						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}
				}
				else if (op == OP_CS_GET_BASE_INFO)
				{
					gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
					packet->start_write();
					packet->set_op(OP_SC_GET_BASE_INFO);
					packet->put_uint32(sid);
					packet->put_string("da ge");
					packet->put_string("man");
					packet->flip();

					my_leaf_node.get_session()->do_write(packet);
				}
			}
			else if (op > OP_SS_BEGIN && op < OP_SS_END)
			{
				if (op == OP_SS_HELLO)
				{
					printf("recv msg %s...\n", net_msg->get_string().c_str());
				}
			}
			else if (op > OP_SG_GS_BEGIN && op < OP_SG_GS_END)
			{
			}
			g_data_packet_pool.put_data_packet(net_msg);
		}
		void session_state_change_handler(bool now_is_connect)
		{//网络线程
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
					/**/
					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SG_AUTH);
						packet->put_string("auth");
						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}
					
					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SG_REG_LISTEN_OP);
						packet->put_uint32(3);
						packet->put_uint32(OP_CS_LOGIN);
						packet->put_uint32(OP_CS_GET_BASE_INFO);
						packet->put_uint32(OP_SS_HELLO);

						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}
					
				}
				else
				{
				}
			}

			static int  looper = 0;
			looper++;
			if (false && (looper%1000 == 0))
			{
				gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
				packet->start_write();
				packet->set_op(OP_SS_HELLO);
				packet->put_string("Hello");
				packet->flip();

				my_leaf_node.get_session()->do_write(packet);
			}
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	class main_game_server_02 :public gnet::service
	{
	public:
		std::atomic<bool> m_is_connected = false;
		bool m_last_connect_state = false;
		gnet::leaf_node  my_leaf_node;
		main_game_server_02(const char* service_name) :
			gnet::service(service_name),
			my_leaf_node("127.0.0.1", 9527, 10000)
		{
			std::function<void(gnet::data_packet*)> handler = std::bind(&main_game_server_02::net_msg_handler, this, std::placeholders::_1);
			my_leaf_node.set_net_msg_handler(handler);
			std::function<void(bool)> handler01 = std::bind(&main_game_server_02::session_state_change_handler, this, std::placeholders::_1);
			my_leaf_node.set_session_state_change_handler(handler01);
			my_leaf_node.start();
		}
		void net_msg_handler(gnet::data_packet* net_msg)
		{
			net_msg->start_read();
			uint32 op = net_msg->get_op();
			if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_END)
			{

			}
			else if (op > OP_SS_BEGIN && op < OP_SS_END)
			{
				if (op == OP_SS_HELLO)
				{
					printf("recv msg %s...\n", net_msg->get_string().c_str());
				}
			}
			else if (op > OP_SG_GS_BEGIN && op < OP_SG_GS_END)
			{
			}
			g_data_packet_pool.put_data_packet(net_msg);
		}
		void session_state_change_handler(bool now_is_connect)
		{//网络线程
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
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SG_AUTH);
						packet->put_string("auth");
						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}

					{
						gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
						packet->start_write();
						packet->set_op(OP_SG_REG_LISTEN_OP);
						packet->put_uint32(1);
						packet->put_uint32(OP_SS_HELLO);
						packet->flip();

						my_leaf_node.get_session()->do_write(packet);
					}
				}
				else
				{
				}
			}

			static int  looper = 0;
			looper++;
			if (false && (looper % 2000 == 0))
			{
				gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
				packet->start_write();
				packet->set_op(OP_SS_HELLO);
				packet->put_string("Hello");
				packet->flip();

				my_leaf_node.get_session()->do_write(packet);
			}
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	mgr.add_service(std::make_shared<main_game_server_01>("main_game_server_01"));
	mgr.add_service(std::make_shared<main_game_server_02>("main_game_server_02"));
	mgr.start();

	getchar();
	gnet::glog::deleteInstance();
}
