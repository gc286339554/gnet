#include "mysql_gate_service.h"
#include "mysql_base_server.h"
using namespace gnet;
mysql_gate_service::mysql_gate_service(const char* service_name, mysql_base_server* base_server)
:service(service_name),
m_base_server(base_server),
m_my_gateway(base_server->m_port, base_server->m_timeout)
{

}
mysql_gate_service::~mysql_gate_service()
{
}
void mysql_gate_service::init()
{
	std::function<void(gnet::data_packet*, uint32)> msg_handler = std::bind(&mysql_gate_service::net_msg_handler, this, std::placeholders::_1, std::placeholders::_2);
	m_my_gateway.set_net_msg_handler(msg_handler);
	std::function<void(uint32)> close_handler = std::bind(&mysql_gate_service::session_close_handler, this, std::placeholders::_1);
	m_my_gateway.set_session_close_handler(close_handler);

	m_my_gateway.start();
}
void mysql_gate_service::net_msg_handler(gnet::data_packet* net_msg, uint32 sid)
{//网络线程
	do
	{
		std::unique_lock<std::mutex> lock(m_data_packet_queue_lock);
		m_data_packet_queue.push(std::make_pair(sid, net_msg));
		assert(net_msg->get_using());
	} while (0);
}
void mysql_gate_service::session_close_handler(uint32 sid)
{//网络线程

}
void mysql_gate_service::msg_execute()
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
			data_packet_queue_tmp.pop();
			char buff[128];
			assert(msg.second->get_using());
			msg.second->put_extend_data("sid", _itoa(msg.first, buff, 10));
			sprintf(buff, "my_sql_server_logic_%d", m_cur_sql_request_count%m_base_server->m_logic_count);
			std::shared_ptr<gnet::service_msg> msg_sp = std::make_shared<gnet::service_msg>(msg.second);
			send_msg(msg_sp, buff);
			m_cur_sql_request_count++;
		}
	} while (true);
}
void mysql_gate_service::update(uint32 ms_delay)
{
	msg_execute();
};
void mysql_gate_service::service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
{
	std::string& target_sid = msg_sp->get_data_packet_p()->get_extend_data("sid");
	if (!target_sid.empty())
	{
		data_packet* packet = g_data_packet_pool.get_data_packet();
		packet->copy_from(msg_sp->get_data_packet_p());
		uint32 sid = atoi(target_sid.c_str());
		m_my_gateway.send_msg_to_client(sid, packet);
	}
};
