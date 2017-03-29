#include "io_service_holder.h"
#include <thread>

namespace gnet{

asio::io_service& io_service_holder::get_io_service()
{
    return m_io_service;
}
void io_service_holder::do_timer_async_wait()
{
	m_timer_sp->expires_from_now(boost::posix_time::seconds(INT32_MAX));
	m_timer_sp->async_wait([this](const std::error_code&) {
		do_timer_async_wait();
	});
}
void io_service_holder::start_net_work_threads(uint32 thread_count)
{
	m_timer_sp = std::make_shared<asio::deadline_timer>(m_io_service);
	do_timer_async_wait();	
    if(thread_count < 1)thread_count = std::thread::hardware_concurrency();//cup 核心数
    for (uint32 i=0;i<thread_count;i++)
    {
        std::thread th(
            [this]()
            {
               m_io_service.run();
            }
        );
        th.detach();
    }
}

void io_service_holder::stop()
{
	m_timer_sp->cancel();
	m_timer_sp.reset();
	m_io_service.stop();
}

};
