#include "../../src/3rd/log/glog.h"
#include "../commom/pre_header.h"
#include "../extend/gateway/gateway.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"
#include "../commom/service/service_msg.hpp"
#include "../data_packet_pool.hpp"


gnet::service_mgr mgr;
int main()
{
	gnet::glog::newInstance("gateway_test", "log");
	mgr.init(8);

	class main_service_gateway : public gnet::service
	{
	public:
		std::mutex m_data_packet_queue_lock;
		std::queue<std::pair<int, gnet::data_packet*> >	m_data_packet_queue;
		gnet::gateway  m_my_gateway;
		uint32 m_last_session = 0;
		main_service_gateway(const char* service_name) :
			gnet::service(service_name),
			m_my_gateway(9527,15)//超时时间  需比客户端长
		{
			
		}
		~main_service_gateway()
		{
		}
		void init()
		{
			std::function<void(gnet::data_packet*, uint32)> handler = std::bind(&main_service_gateway::net_msg_handler, this, std::placeholders::_1, std::placeholders::_2);
			m_my_gateway.set_net_msg_handler(handler);
			std::function<void(uint32)> handler01 = std::bind(&main_service_gateway::session_close_handler, this, std::placeholders::_1);
			m_my_gateway.set_session_close_handler(handler01);

			m_my_gateway.start();
		}
		void net_msg_handler(gnet::data_packet* net_msg, uint32 sid)
		{//网络线程
			m_last_session = sid;
			do
			{
				std::unique_lock<std::mutex> lock(m_data_packet_queue_lock);
				m_data_packet_queue.push(std::make_pair(sid,net_msg));
			} while (0);
			printf("client_%d net_msg_handler ...\n", sid);
		}
		void session_close_handler(uint32 sid)
		{//网络线程
			LOG(INFO) << "client_" << sid << "is closed ...\n";
		}
		void msg_execute()
		{
			std::queue<std::pair<int, gnet::data_packet*> >	data_packet_queue_tmp;
			do
			{
				std::unique_lock<std::mutex> lock(m_data_packet_queue_lock);
				std::swap(data_packet_queue_tmp, m_data_packet_queue);
			} while (0);

			do
			{
				if (data_packet_queue_tmp.empty())
				{
					break;
				}
				else
				{
					auto msg = data_packet_queue_tmp.front();
					msg.second->start_read();

					uint32 val = msg.second->get_int32();
					printf("client_%d num is: %d ...\n", msg.first, val);
					
					data_packet_queue_tmp.pop();
					g_data_packet_pool.put_data_packet(msg.second);
				}
			} while (true);
		}
		virtual void update(uint32 ms_delay)
		{
			msg_execute();
			static uint32 index = 0;
			index++;
			if (index % 5000 == 0)
			{
				gnet::data_packet* packet = g_data_packet_pool.get_data_packet();
				packet->start_write().set_op(100).put_uint32(index).end_write();

				m_my_gateway.send_msg_to_client(m_last_session,packet);
			}
			if (index == 5000000)
			{
				//mgr.remove_service(get_service_id());
			}
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
			msg_sp->get_data_packet_p()->start_read();
			//printf("recv %d,  service id is: %d ...\n", msg_sp->get_data_packet_p()->get_uint32(), msg_sp->get_source_service_id());
		};
	};
	class second_service_gateway : public gnet::service
	{
	public:
	
		second_service_gateway(const char* service_name) :
			gnet::service(service_name)
		{

		}
		~second_service_gateway()
		{
		}
		
		virtual void update(uint32 ms_delay)
		{
			static uint32 index = 0;
			index++;
			if (index % 5000 == 0)
			{
				std::shared_ptr<gnet::service_msg> msg_sp = std::make_shared<gnet::service_msg>();
				msg_sp->get_data_packet_p()->start_write().put_uint32(index).end_write();
				send_msg(msg_sp, "main_service_gateway");
			}
			
		};
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};
	
	mgr.add_service(std::make_shared<main_service_gateway>("main_service_gateway"));
	//mgr.add_service(std::make_shared<second_service_gateway>("second_service_gateway"));
	mgr.start();

	getchar();
	gnet::glog::deleteInstance();
}

