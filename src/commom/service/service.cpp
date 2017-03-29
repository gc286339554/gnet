#include "service.h"
#include "service_mgr.h"

gnet::service::service(const char* service_name)
:m_service_name(service_name)
{
}
gnet::service::~service()
{
}
void gnet::service::init()
{
}
void gnet::service::deal_message()
{
	std::queue<std::shared_ptr<service_msg> >	service_msg_queue_tmp;
	do
	{
		std::lock_guard<std::mutex> lock(m_service_msg_queue_lock);
		std::swap(service_msg_queue_tmp, m_service_msg_queue);
	} while (0);
	do
	{
		if (service_msg_queue_tmp.empty())
		{
			break;
		}
		else
		{
			std::shared_ptr<service_msg> msg = service_msg_queue_tmp.front();
			service_msg_queue_tmp.pop();
			message_handler(msg);
		}
	} while (true);
}
void gnet::service::update()
{
}
void gnet::service::set_service_mgr(gnet::service_mgr* service_mgr_p)
{
	m_service_mgr_p = service_mgr_p;
}
uint32 gnet::service::get_service_id(const char* service_name)
{
	return m_service_mgr_p->get_service_info(service_name).m_service_id;
}
void gnet::service::receive_msg(std::shared_ptr<service_msg>& msg_sp)
{
	std::lock_guard<std::mutex> lock(m_service_msg_queue_lock);
	m_service_msg_queue.emplace(msg_sp);
}
void gnet::service::send_msg(std::shared_ptr<service_msg>& msg_sp,const char* target_service_name)
{
	msg_sp->set_source_service_id(get_service_id());
	msg_sp->set_target_service_id(get_service_id(target_service_name));

	service_info& info = m_service_mgr_p->get_service_info(target_service_name);
	if (info.m_service_id != INVALID_SERVICE_ID)
	{
		info.m_service_sp->receive_msg(msg_sp);
	}
}