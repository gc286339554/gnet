#include "gateway.h"

using namespace gnet;
gateway::gateway(uint32 port, uint32 timeout)
:m_port(port),
m_server_acceptor(port,timeout)
{

}
gateway::~gateway()
{
    m_server_acceptor.stop(); 
	m_server_session_mgr_sp->stop();
}

void gateway::start()
{
    m_server_session_mgr_sp = std::make_shared<server_session_mgr>();
	m_server_acceptor.start_accept(m_server_session_mgr_sp);
}

void gateway::set_server_session_msg_handler(std::function<void(net_packet*, uint32)>& handler)
{
	m_server_acceptor.set_server_session_msg_handler(handler);
}
bool gateway::send_msg_to_client(uint32 sid, net_packet* packet)
{
	std::shared_ptr<server_session> ssp = m_server_session_mgr_sp->find_session_by_id(sid);
	if (ssp)
	{
		ssp->do_write(packet);
		return true;
	}
	return false;
}
