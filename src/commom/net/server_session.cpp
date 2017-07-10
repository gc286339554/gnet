#include "../../src/3rd/log/glog.h"
#include "server_session.h"
#include "server_session_mgr.hpp"
#include "opcode.h"
using namespace gnet;
server_session::server_session(uint32 timeout) 
:base_session(timeout)
{
}
void server_session::set_server_session_mgr(std::shared_ptr<server_session_mgr>& server_session_mgr_sp)
{
	m_server_session_mgr_sp = server_session_mgr_sp;
	m_server_session_mgr_sp->add_new_session(shared_from_this());
}
void server_session::post_data_packet_handler()
{
	m_data_packet_p->start_read();
	uint32 opcode = m_data_packet_p->get_op();//协议号
	if (opcode == OP_HEART_BEAT_PING_PONG)
	{//每个消息都检测，效率低下
		g_data_packet_pool.put_data_packet(m_data_packet_p);
		send_heart_beat();
		return;
	}
	m_data_packet_p->reset();
	m_net_msg_handler(m_data_packet_p, m_session_id);
}
void server_session::timeout_check()
{
	if (m_timeout_monitor_finish)
	{
		m_timeout_monitor_finish = false;
		auto This = shared_from_this();
		m_timeout_monitor.expires_from_now(boost::posix_time::seconds(m_timeout / 2));
		m_timeout_monitor.async_wait(std::bind([This]()
		{
			This->m_timeout_monitor_finish = true;
			if (This->is_timeout())
			{
				LOG(WARNING) << "server timeout\n";
				This->stop();
			}
			else
			{
				This->timeout_check();//继续添加回调。
			}
		}));
	}
}

void server_session::set_net_msg_handler(std::function<void(data_packet*, uint32)>& handler)
{
	m_net_msg_handler = handler;
}

void server_session::stop()
{
	close_socket();
	m_server_session_mgr_sp->socket_close(shared_from_this());
}

