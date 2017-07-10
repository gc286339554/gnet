#pragma once
#include "service/service.h"
#include "../3rd/mysql/mysql_connection.h"

namespace gnet {
	struct mysql_connect_param
	{
		std::string hostname;
		uint32 port;
		std::string username;
		std::string password;
		std::string dbname;
	};
	class mysql_base_server_logic : public service
	{
		public:
			mysql_base_server_logic(const char* service_name, mysql_connect_param param);
			~mysql_base_server_logic();

			virtual void init();
			virtual void update(uint32 ms_delay);
			virtual void service_msg_handler(std::shared_ptr<service_msg>& msg_sp);
		protected:
			std::map<uint32, std::function<void(data_packet*,data_packet*)> > m_handlers_map;
			mysql_connection m_mysql_connection;
			mysql_connect_param m_mysql_connect_param;
			bool m_is_valid = false;
	};
};