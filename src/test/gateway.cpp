#include "../commom/pre_header.h"
#include "../gateway/gateway.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"

gnet::service_mgr mgr;
int main()
{
	mgr.init(8);
	gnet::io_service_holder::newInstance();
	gnet::io_service_holder::instance()->start_net_work_threads(8);


	class main_service_gateway : public gnet::service
	{
	public:
		std::mutex m_net_packet_queue_lock;
		std::queue<std::pair<int, gnet::net_packet*> >	m_net_packet_queue;
		gnet::gateway  m_my_gateway;
		uint32 m_last_session = 0;
		main_service_gateway(const char* service_name) :
			gnet::service(service_name),
			m_my_gateway(9527,10)
		{
			
		}
		~main_service_gateway()
		{
		}
		void init()
		{
			std::function<void(gnet::net_packet*, uint32)> handler = std::bind(&main_service_gateway::net_msg_handler, this, std::placeholders::_1, std::placeholders::_2);
			m_my_gateway.set_server_session_msg_handler(handler);
			m_my_gateway.start();
		}
		void net_msg_handler(gnet::net_packet* net_msg, uint32 sid)
		{//网络线程
			m_last_session = sid;
			do
			{
				std::unique_lock<std::mutex> lock(m_net_packet_queue_lock);
				m_net_packet_queue.push(std::make_pair(sid,net_msg));
			} while (0);
		}
		void msg_execute()
		{
			std::queue<std::pair<int, gnet::net_packet*> >	net_packet_queue_tmp;
			do
			{
				std::unique_lock<std::mutex> lock(m_net_packet_queue_lock);
				std::swap(net_packet_queue_tmp, m_net_packet_queue);
			} while (0);

			do
			{
				if (net_packet_queue_tmp.empty())
				{
					break;
				}
				else
				{
					auto msg = net_packet_queue_tmp.front();
					msg.second->skip_head_for_read();
					uint32 opcode = msg.second->get_uint32();//协议号
					uint32 val = msg.second->get_int32();
					printf("client_%d num is: %d ...\n", msg.first, val);
					
					net_packet_queue_tmp.pop();
					delete msg.second;
				}
			} while (true);
		}
		virtual void update()
		{
			msg_execute();
			static uint32 index = 0;
			index++;
			if (index % 5000 == 0)
			{
				gnet::net_packet* packet = new gnet::net_packet();	//构造心跳包
				packet->put_uint32(0);
				packet->put_uint32(100);
				packet->put_uint32(index);
				packet->flip();

				m_my_gateway.send_msg_to_client(m_last_session,packet);
			}
			if (index == 5000000)
			{
				mgr.remove_service(get_service_id());
			}
		};
		virtual void message_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};
	
	mgr.add_service(std::make_shared<main_service_gateway>("main_service_gateway"));
	mgr.start();

	getchar();

	gnet::io_service_holder::deleteInstance();
}

