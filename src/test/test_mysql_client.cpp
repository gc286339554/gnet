#include "../../src/commom/pre_header.h"
#include "../../src/commom/net/data_packet.hpp"
#include "../../src/extend/leaf_node/leaf_node.h"
#include "../../src/commom/service/service.h"
#include "../../src/commom/service/service_mgr.h"


enum {
	SD_QUERY_TIME = 0,
	SD_INSERT_DATA,
	SD_DELETE_DATA,
	SD_UPDATE_DATA,
	SD_QUERY_DATA,
};

class mysql_service_client :public gnet::service
{
public:
	gnet::leaf_node  my_leaf_node;
	mysql_service_client(const char* service_name) :
		gnet::service(service_name),
		my_leaf_node("127.0.0.1", 6954, 100)
	{
	}
	void init()
	{
		std::function<void(gnet::data_packet*)> handler = std::bind(&mysql_service_client::net_msg_handler, this, std::placeholders::_1);
		my_leaf_node.set_net_msg_handler(handler);
		std::function<void(bool)> handler01 = std::bind(&mysql_service_client::session_state_change_handler, this, std::placeholders::_1);
		my_leaf_node.set_session_state_change_handler(handler01);
		my_leaf_node.start();
		//my_leaf_node.set_do_heart_beat(false);
	}
	void net_msg_handler(gnet::data_packet* net_msg)
	{
		std::lock_guard<std::mutex> lock(m_msg_back_lock);
		m_msg_back.push(net_msg);
	}
	void session_state_change_handler(bool now_is_connect)
	{//网络线程
		static std::mutex	t_lock;
		std::lock_guard<std::mutex> lock(t_lock);
		static uint32 con_count = 0;
		static uint32 dis_con_count = 0;
		m_is_connect = now_is_connect;
		if (now_is_connect)
			printf("session connected %d...\n", ++con_count);
		else
		{
			printf("session disconnected %d...\n", ++dis_con_count);
			m_callback_map.clear();
		}
			
	}
	void mysql_query_time()
	{
		gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->start_write();
		packet->set_op(SD_QUERY_TIME);
		packet->put_uint32(++m_cur_callback_index);
		packet->put_string("select NOW() as time;");
		packet->flip();
		m_callback_map[m_cur_callback_index] = [](gnet::data_packet* net_msg) {
			std::string sql_result = net_msg->get_string();
			printf("time is : %s \n", sql_result.c_str());
		};
		my_leaf_node.get_session()->do_write(packet);
	}
	void mysql_insert_data()
	{
		gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->start_write();
		packet->set_op(SD_INSERT_DATA);
		packet->put_uint32(++m_cur_callback_index);
		int random_variable = std::rand();
		random_variable = random_variable % 256;
		char buff[128];
		sprintf(buff, "insert into test(`label`) VALUE(\"%d\");", random_variable);
		packet->put_string(buff);
		packet->flip();
		m_callback_map[m_cur_callback_index] = [](gnet::data_packet* net_msg) {
			bool sql_result = net_msg->get_bool();
			if (sql_result)
			{
				printf("insert is succeed \n");
			}
			else
			{
				printf("insert is failed \n");
			}
		};
		my_leaf_node.get_session()->do_write(packet);
	}
	void mysql_delete_data()
	{
		gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->start_write();
		packet->set_op(SD_DELETE_DATA);
		packet->put_uint32(++m_cur_callback_index);
		int random_variable = std::rand();
		random_variable = random_variable % 256;
		char buff[128];
		sprintf(buff, "delete from test where label = \"%d\";", random_variable);
		packet->put_string(buff);
		packet->flip();
		m_callback_map[m_cur_callback_index] = [](gnet::data_packet* net_msg) {
			int32 sql_result = net_msg->get_int32();
			printf("delete row is %d \n", sql_result);
		};
		my_leaf_node.get_session()->do_write(packet);
	}
	void mysql_update_data()
	{
		gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->start_write();
		packet->set_op(SD_UPDATE_DATA);
		packet->put_uint32(++m_cur_callback_index);
		int random_variable = std::rand();
		random_variable = random_variable % 256;
		int random_variable01 = std::rand();
		random_variable01 = random_variable01 % 256;
		char buff[128];
		sprintf(buff, "update test set label = \"%d\" where label = \"%d\";", random_variable, random_variable01);
		packet->put_string(buff);
		packet->flip();
		m_callback_map[m_cur_callback_index] = [](gnet::data_packet* net_msg) {
			int32 sql_result = net_msg->get_int32();
			printf("update row is %d \n", sql_result);
		};
		my_leaf_node.get_session()->do_write(packet);
	}
	void mysql_query_data()
	{
		gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->start_write();
		packet->set_op(SD_QUERY_DATA);
		packet->put_uint32(++m_cur_callback_index);
		packet->put_string("select count(*) from test;");
		packet->flip();
		m_callback_map[m_cur_callback_index] = [](gnet::data_packet* net_msg) {
			int32 sql_result = net_msg->get_int32();
			printf("row count is %d \n", sql_result);
		};
		my_leaf_node.get_session()->do_write(packet);
	}
		
	virtual void update(uint32 ms_delay)
	{
		static bool do_it = true;
		int random_variable = std::rand();
		random_variable = random_variable % 10000;
		if (m_is_connect && do_it)
		{
			if (random_variable == 0)
			{
				//do_it = false;
				mysql_query_time();
			}
			else if (random_variable == 1)
			{
				///do_it = false;
				mysql_insert_data();
			}
			else if (random_variable == 2)
			{
				//do_it = false;
				mysql_delete_data();
			}
			else if (random_variable == 3)
			{
				//do_it = false;
				mysql_update_data();
			}
			else if (random_variable == 4)
			{
				//do_it = false;
				mysql_query_data();
			}
		}

		do 
		{// deal  back msg
			std::queue<gnet::data_packet*> t_msg_back;
			do 
			{
				std::lock_guard<std::mutex> lock(m_msg_back_lock);
				std::swap(m_msg_back, t_msg_back);
			} while (0);
			
			do 
			{
				if (!t_msg_back.empty())
				{
					gnet::data_packet* net_msg = t_msg_back.front();
					t_msg_back.pop();
					net_msg->start_read();
					uint32 index = net_msg->get_uint32();
					auto iter = m_callback_map.find(index);
					if (iter != m_callback_map.end())
					{
						iter->second(net_msg);
						m_callback_map.erase(iter);
					}
					g_data_packet_pool.put_data_packet(net_msg);
				}
				else
				{
					break;
				}
			} while (true);
		} while (0);
	};
	virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
	{

	};
private:
	bool  m_is_connect = false;

	uint32 m_cur_callback_index = 0;
	std::map<uint32, std::function<void(gnet::data_packet*)> > m_callback_map;

	std::mutex	m_msg_back_lock;
	std::queue<gnet::data_packet*> m_msg_back;
};
//查询的有序性由mysql_client自己保证
int main()
{
	std::srand(std::time(0));
	gnet::service_mgr mgr;
	mgr.init(8);
	char buff[256];
	for (int i = 0; i<500; i++)
	{
		sprintf(buff, "mysql_service_client_%d", i);
		mgr.add_service(std::make_shared<mysql_service_client>(buff));
	}
	mgr.start();

	getchar();
	return 0;
}

