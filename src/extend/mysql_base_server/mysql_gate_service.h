#pragma once
#include "service/service.h"
#include "../gateway/gateway.h"


namespace gnet {
	class mysql_gate_service : public service
	{
	private:
		gateway  m_my_gateway;
		std::mutex m_data_packet_queue_lock;
		std::queue<std::pair<int, gnet::data_packet*> >	m_data_packet_queue;
		class mysql_base_server* m_base_server;
		uint32 m_cur_sql_request_count = 0;
	public:
		mysql_gate_service(const char* service_name, mysql_base_server* base_server);
		~mysql_gate_service();
		void net_msg_handler(gnet::data_packet* net_msg, uint32 sid);
		void session_close_handler(uint32 sid);
		void msg_execute();
		virtual void init();
		virtual void update(uint32 ms_delay);
		virtual void service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp);
	};
};