#pragma once
#include "pre_header.h"
#include "data_packet.hpp"
#include "../singleton.h"

namespace gnet {

//thread safe
class data_packet_pool : public singleton<data_packet_pool>
{
	public:
		data_packet_pool()
		{
		}
		~data_packet_pool()
		{
			std::lock_guard<std::mutex> lock(m_free_data_packet_list_lock);
			for (data_packet* p_data_packet : m_free_data_packet_list)
			{
				SAFE_DEL(p_data_packet);
			}
		}
		data_packet* get_data_packet() 
		{
			std::lock_guard<std::mutex> lock(m_free_data_packet_list_lock);
			data_packet* p_data_packet = nullptr;
			if (m_free_data_packet_list.empty())
			{
				p_data_packet = new data_packet();
				m_data_packet_count++;
			}
			else
			{
				p_data_packet = m_free_data_packet_list.front();
				m_free_data_packet_list.pop_front();
				p_data_packet->set_using(true);
			}
			m_get_count++;
			return p_data_packet;
		}
		void put_data_packet(data_packet* packet)
		{
			packet->reset();
			std::lock_guard<std::mutex> lock(m_free_data_packet_list_lock);
#ifdef _DEBUG
			/*for test*/
			/*for (auto obj : m_free_data_packet_list)
			{
				if (obj == packet) assert(false);
			}*/
			/*for test*/
#endif // _DEBUG
			if (m_free_data_packet_list.size() > 100)
			{
				SAFE_DEL(packet);
				m_put_count++;
			}
			else
			{
				m_free_data_packet_list.push_back(packet);
				packet->set_using(false);
				m_put_count++;
			}
		}

	private:
		std::mutex	m_free_data_packet_list_lock;
		std::list<data_packet*> m_free_data_packet_list;
		uint32 m_data_packet_count = 0;
		uint32 m_get_count = 0;
		uint32 m_put_count = 0;
};

};
#define g_data_packet_pool ((*(gnet::data_packet_pool::instance())))

