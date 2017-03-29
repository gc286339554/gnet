#pragma once
#include "../pre_header.h"
#include "asio/asio.hpp"
#include "../singleton.h"

namespace gnet {
/*
此io_service一般用于网络io，可以复用为其他timer。
如果独立的功能模块，可以另外定义 io_service
*/
class io_service_holder : public singleton<io_service_holder>{
public:
    asio::io_service& get_io_service(); 
    void start_net_work_threads(uint32 thread_count = 1); //启动网络线程
	void stop();
private:
	void do_timer_async_wait();
	std::shared_ptr<asio::deadline_timer>	m_timer_sp;//默认给一个异步任务，防止io_server退出
    asio::io_service						m_io_service;	
};

};

#define g_io_service ((*(gnet::io_service_holder::instance())).get_io_service())