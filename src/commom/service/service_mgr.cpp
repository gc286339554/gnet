#include "service_mgr.h"

gnet::service_mgr::service_mgr()
{
}
gnet::service_mgr::~service_mgr()
{
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
			m_service_name_map.insert(std::make_pair(service_sp->get_service_name(), tmp_service_info));
		}
	} while (0);

	do
	{
		std::lock_guard<std::mutex> lock(m_service_init_list_lock);
		m_service_init_list.push_back(tmp_service_info);//需要執行init的service
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
	
	while (m_is_run)
	{
		///////////////////////init begin////////////////////////////
		std::list<service_info> service_init_list_tmp;
		do
		{
			std::lock_guard<std::mutex> lock(m_service_init_list_lock);
			std::swap(service_init_list_tmp, m_service_init_list);
		} while (0);
		for (service_info& service_info_tmp : service_init_list_tmp)
		{
			service_info_tmp.m_service_sp->init();
		}
		/////////////////////////init end/////////////////////////////

		///////////////////////update begin////////////////////////////
		std::vector<std::shared_ptr<service> > service_list_tmp;
		do
		{
			std::lock_guard<std::mutex> lock(*(m_service_lock[work_thread_index]).get());
			service_list_tmp = m_service_list[work_thread_index];
		} while (0);

		for (std::shared_ptr<service>& service : service_list_tmp)
		{
			//消息处理
			service->deal_message();
			//时间片分发
			service->update();
		}
		/////////////////////////update end/////////////////////////////

		std::chrono::milliseconds dura(1);
		std::this_thread::sleep_for(dura);
	}
}

