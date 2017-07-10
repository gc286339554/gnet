#pragma once
#include "../gateway/gateway.h"
#include "service/service_mgr.h"
#include "mysql_base_server_logic.h"

namespace gnet {
	class mysql_base_server
	{
		friend class mysql_base_server_logic;
		friend class mysql_gate_service;
	public:
		mysql_base_server(uint32 port, uint32 timeout, uint32 logic_conut);
		~mysql_base_server();

		void init(uint8 work_thread_count);
		virtual void init_logic()=0;
	protected:
		service_mgr mgr;
		uint32 m_port;
		uint32 m_timeout;
		uint32 m_logic_count;
	};
};