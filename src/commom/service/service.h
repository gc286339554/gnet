#pragma once

#include "../pre_header.h"
#include "service_msg.hpp"

namespace gnet {
	class service
	{
	public:
		service(const char* service_name);
		virtual ~service();

		virtual void init();
		virtual void update(uint32 ms_delay);
		virtual void service_msg_handler(std::shared_ptr<service_msg>& msg_sp) = 0;
		void deal_message();
		void receive_msg(std::shared_ptr<service_msg>& msg_sp);
		void send_msg(std::shared_ptr<service_msg>& msg_sp, const char* target_service_name);
		void set_service_mgr(class service_mgr* service_mgr_p);
		uint32 get_service_id() { return m_service_id; };
		void set_service_id(uint32 service_id) { m_service_id = service_id; };
		uint32 get_service_id(const char* service_name);
		const std::string& get_service_name() { return m_service_name; };

		bool need_init() { return !m_inited;};
		void set_inited(bool tmp_inited) { m_inited = tmp_inited; };
	private:
		uint32 m_service_id;
		std::mutex	m_service_msg_queue_lock;
		std::queue<std::shared_ptr<service_msg> >	m_service_msg_queue;
		std::queue<std::shared_ptr<service_msg> >	m_service_msg_queue_tmp;//for update
		std::string m_service_name;
		service_mgr* m_service_mgr_p;
		bool m_inited = false;
	};
}

