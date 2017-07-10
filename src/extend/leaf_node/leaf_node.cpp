#include "leaf_node.h"

namespace gnet {

leaf_node::leaf_node()
{
}
leaf_node::leaf_node(const char* ip, uint32 port, uint32 timeout)
:m_gateway_ip(ip),
m_gateway_port(port)
{
	m_client_session_sp = std::make_shared<client_session>(ip, port, timeout);
}
leaf_node::~leaf_node() 
{
}

void leaf_node::set_gateway_addr(const char* ip, uint32 port, uint32 timeout)
{
	m_gateway_ip = ip;
	m_gateway_port = port;
	m_client_session_sp = std::make_shared<client_session>(ip,port, timeout);
}
void leaf_node::set_net_msg_handler(std::function<void(data_packet*)>& handler)
{
	m_client_session_sp->set_net_msg_handler(handler);
}
void leaf_node::set_session_state_change_handler(std::function<void(bool)>& handler)
{
	m_client_session_sp->set_session_state_change_handler(handler);
}
void leaf_node::start()
{
	g_io_service.post([this]()
	{
		m_client_session_sp->start();
	});
}
void leaf_node::stop()
{
	m_client_session_sp->stop();
}
void leaf_node::set_do_heart_beat(bool is_do)
{
	m_client_session_sp->set_do_heart_beat(is_do);
}
std::shared_ptr<client_session>& leaf_node::get_session()
{
	return m_client_session_sp;
}
};
