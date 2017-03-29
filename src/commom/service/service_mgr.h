#pragma once

#include "../pre_header.h"
#include "service.h"

namespace gnet {
	struct service_info
	{
		uint32 m_service_id = INVALID_SERVICE_ID;
		std::string m_service_name;
		std::shared_ptr<service> m_service_sp;
	};
	class service_mgr
	{
	public:
		service_mgr();
		virtual ~service_mgr();

		void init(uint8 work_thread_count);
		void start();
		void stop();
		bool add_service(std::shared_ptr<service> service_sp);
		bool remove_service(uint32 service_id);
		void work_thread_handler(uint8 work_thread_index);
		service_info& get_service_info(const char* service_name);
	private:
		std::mutex	m_service_name_map_lock;
		std::map<std::string, service_info> m_service_name_map;
		std::mutex	m_service_init_list_lock;
		std::list<service_info> m_service_init_list;//需要執行init的service

		std::vector<std::shared_ptr<std::thread> >	m_work_thread_list;
		std::vector<std::vector<std::shared_ptr<service> > > m_service_list;
		std::vector<std::shared_ptr<std::mutex> > m_service_lock;
		uint8 m_work_thread_count;
		bool m_is_run = true;
	};
}

