#include "server_session.h"
#include "server_session_mgr.hpp"
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
void server_session::post_net_packet_handler()
{
	m_net_packet_p->skip_head_for_read();
	uint32 opcode = m_net_packet_p->get_uint32();//协议号
	if (opcode == GNET_OP_HEART_BEAT_PING_PONG)
	{
		send_heat_beat();
	}
	m_net_packet_p->reset();
	m_server_session_msg_handler(m_net_packet_p, m_session_id);
}
void server_session::timeout_check()
{
	auto This = shared_from_this();
	m_timeout_monitor.expires_from_now(boost::posix_time::seconds(m_timeout / 2));
	m_timeout_monitor.async_wait(std::bind([This]()
	{
		if (This->is_timeout())
		{
			This->stop();
		}
		else
		{
			This->timeout_check();//继续添加回调。
		}
	}));
}

void server_session::set_server_session_msg_handler(std::function<void(net_packet*, uint32)>& handler)
{
	m_server_session_msg_handler = handler;
}

void server_session::stop()
{
	close_socket();
	m_server_session_mgr_sp->socket_close(shared_from_this());
}

