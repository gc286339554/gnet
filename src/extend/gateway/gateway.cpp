#include "gateway.h"

using namespace gnet;
gateway::gateway(uint32 port, uint32 timeout)
:m_port(port),
m_server_acceptor(port,timeout)
{
	m_server_session_mgr_sp = std::make_shared<server_session_mgr>();
}
gateway::~gateway()
{
    m_server_acceptor.stop(); 
	m_server_session_mgr_sp->stop();
}

void gateway::start()
{
	m_server_acceptor.start_accept(m_server_session_mgr_sp);
}

void gateway::set_net_msg_handler(std::function<void(data_packet*, uint32)>& handler)
{
	m_server_acceptor.set_net_msg_handler(handler);
}
void gateway::set_session_close_handler(std::function<void(uint32)>& handler)
{
	m_server_session_mgr_sp->set_session_close_handler(handler);
}
bool gateway::send_msg_to_client(uint32 sid, data_packet* packet)
{
	std::shared_ptr<server_session> ssp = m_server_session_mgr_sp->find_session_by_id(sid);
	if (ssp)
	{
		ssp->do_write(packet);
		return true;
	}
	else
	{
		g_data_packet_pool.put_data_packet(packet);
	}
	return false;
}
void gateway::kick_session(uint32 sid)
{
	m_server_session_mgr_sp->kick_session(sid);
}
