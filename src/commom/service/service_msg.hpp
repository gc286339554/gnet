#pragma once

#include "../pre_header.h"
#include "../net/net_packet.hpp"

namespace gnet {
	class service_msg
	{
	public:
		service_msg() {};
		virtual ~service_msg() {};
	
		uint32 get_target_service_id() { return m_target_service_id; };
		uint32 get_source_service_id() { return m_source_service_id; };
		void set_target_service_id(uint32 id) { m_target_service_id = id; };
		void set_source_service_id(uint32 id) { m_source_service_id = id; };

		std::shared_ptr<net_packet>& get_net_packet_sp() { return m_net_packet_sp; };
		void set_net_packet_sp(std::shared_ptr<net_packet>& net_packet_sp) { m_net_packet_sp = net_packet_sp; };

	private: 
		uint32 m_target_service_id;
		uint32 m_source_service_id;
		std::shared_ptr<net_packet> m_net_packet_sp;
	};
}
