#pragma once

#include "../pre_header.h"
#include "../net/data_packet_pool.hpp"

namespace gnet {
	enum service_msg_type
	{
		msg_type_binary,	//通用消息  二进制
		msg_type_lua,		//lua service 之间的消息
	};
	class service_msg
	{
	public:
		service_msg(data_packet* t_data_packet_p = nullptr)
		{
			if (t_data_packet_p)
				m_data_packet_p = t_data_packet_p;
			else
				m_data_packet_p = g_data_packet_pool.get_data_packet();
		};
		virtual ~service_msg() 
		{
			g_data_packet_pool.put_data_packet(m_data_packet_p);
		};
	
		uint32 get_target_service_id() { return m_target_service_id; };
		uint32 get_source_service_id() { return m_source_service_id; };
		service_msg_type get_msg_type() { return m_msg_type; };
		void set_target_service_id(uint32 id) { m_target_service_id = id; };
		void set_source_service_id(uint32 id) { m_source_service_id = id; };
		void set_msg_type(service_msg_type msg_type) { m_msg_type = msg_type; };

		data_packet* get_data_packet_p() { return m_data_packet_p; };

	private: 
		uint32 m_target_service_id;
		uint32 m_source_service_id;
		data_packet*	m_data_packet_p = nullptr;
		service_msg_type m_msg_type = msg_type_binary;
	};
}
