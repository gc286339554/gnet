#include "service_mgr.h"
#include "../io_service_holder.h"
#include "../3rd/performance/execute_timer.hpp"

gnet::service_mgr::service_mgr()
{
	gnet::data_packet_pool::newInstance();
	gnet::io_service_holder::newInstance();
	gnet::io_service_holder::instance()->start_net_work_threads();
}
gnet::service_mgr::~service_mgr()
{
	gnet::io_service_holder::deleteInstance();
	gnet::data_packet_pool::deleteInstance();
}
void gnet::service_mgr::init(uint8 work_thread_count)
{
	if (work_thread_count == 0)
	{
		work_thread_count = std::thread::hardware_concurrency();//cup 核心数;
	}
	m_work_thread_count = work_thread_count;
	m_service_list.resize(work_thread_count);
	for (uint8 i = 0; i < work_thread_count; i++)
	{
		m_service_lock.emplace_back(std::make_shared<std::mutex>());
	}
}
void gnet::service_mgr::start()
{
	for (uint8 i = 0; i < m_work_thread_count; i++)
	{
		std::shared_ptr<std::thread> th_sp = std::make_shared<std::thread>(&gnet::service_mgr::work_thread_handler,this,i);
		m_work_thread_list.emplace_back(th_sp);
	}

	for (std::shared_ptr<std::thread>& th : m_work_thread_list)
	{
		th->join();
	}
}
void gnet::service_mgr::stop()
{
	m_is_run = false;
}
gnet::service_info& gnet::service_mgr::get_service_info(const char* service_name)
{
	do
	{
		std::lock_guard<std::mutex> lock(m_service_name_map_lock);
		auto iter = m_service_name_map.find(std::string(service_name));
		if (iter != m_service_name_map.end())
		{
			return iter->second;
		}
	} while (0);

	static gnet::service_info invalid_service;
	return invalid_service;
}
bool gnet::service_mgr::add_service(std::shared_ptr<service> service_sp)
{
	service_info tmp_service_info;
	do
	{
		std::lock_guard<std::mutex> lock(m_service_name_map_lock);
		if (m_service_name_map.find(service_sp->get_service_name()) != m_service_name_map.end())
		{
			return false;
		}
		else
		{
			static uint32 service_id_index = 0;
			tmp_service_info.m_service_id = service_id_index++;
			tmp_service_info.m_service_name = service_sp->get_service_name();
			tmp_service_info.m_service_sp = service_sp;
			service_sp->set_service_id(tmp_service_info.m_service_id);
			m_service_name_map.insert(std::make_pair(service_sp->get_service_name(), tmp_service_info));
		}
	} while (0);

	static uint32 target_work_thread = 0;//线程负载均衡，将service均分到各个work线程
	uint32 target_work_thread_tmp = 0;
	do
	{
		static std::mutex add_service_lock;
		std::lock_guard<std::mutex> lock(add_service_lock);
		target_work_thread_tmp = target_work_thread++;
	} while (0);

	//需要增加独享模式，主service独享一个工作线程
	do
	{
		std::lock_guard<std::mutex> lock(*(m_service_lock[target_work_thread_tmp % m_work_thread_count]).get());
		m_service_list[target_work_thread_tmp % m_work_thread_count].emplace_back(service_sp);
	} while (0);
	service_sp->set_service_mgr(this);
	return true;
}
bool gnet::service_mgr::remove_service(uint32 service_id)
{
	bool remove_suc = false;
	std::string service_name;
	for (uint32 i = 0; i < m_service_list.size(); i++)
	{
		
		std::lock_guard<std::mutex> lock(*(m_service_lock[i]).get());
		for (auto iter = m_service_list[i].begin(); iter != m_service_list[i].end(); iter++)
		{
			if ((*iter)->get_service_id() == service_id)
			{
				service_name = (*iter)->get_service_name();
				m_service_list[i].erase(iter);
				remove_suc = true;
				break;
			}
		}
		if (remove_suc)
		{
			break;
		}
	}

	do
	{
		std::lock_guard<std::mutex> lock(m_service_name_map_lock);
		auto iter = m_service_name_map.find(std::string(service_name));
		if (iter != m_service_name_map.end())
		{
			m_service_name_map.erase(iter);
		}
	} while (0);

	return remove_suc;
}
void gnet::service_mgr::work_thread_handler(uint8 work_thread_index_param)
{
	uint8 work_thread_index = work_thread_index_param;
	std::vector<std::shared_ptr<service> > service_list_tmp;
	std::chrono::steady_clock::time_point last_update_time_stamp = std::chrono::steady_clock::now();
	std::chrono::milliseconds dura(1);
	uint64 ms_delay;
	while (m_is_run)
	{
		///////////////////////update begin////////////////////////////
		
		service_list_tmp.clear();
		do
		{
			std::lock_guard<std::mutex> lock(*(m_service_lock[work_thread_index]).get());
			service_list_tmp = m_service_list[work_thread_index];
		} while (0);

		ms_delay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_update_time_stamp).count();
		last_update_time_stamp = std::chrono::steady_clock::now();
		for (std::shared_ptr<service>& service : service_list_tmp)
		{
			//init 
			if (service->need_init())
			{
				service->init();
				service->set_inited(true);
			}
			//消息处理
			service->deal_message();
			//时间片分发
			service->update(ms_delay);
		}
		/////////////////////////update end/////////////////////////////

		std::this_thread::sleep_for(dura);
	}
}

