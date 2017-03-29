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
void leaf_node::set_client_session_msg_handler(std::function<void(net_packet*)>& handler)
{
	m_client_session_sp->set_client_session_msg_handler(handler);
}
void leaf_node::start()
{
	m_client_session_sp->start();
}
void leaf_node::stop()
{
	m_client_session_sp->stop();
}
std::shared_ptr<client_session>& leaf_node::get_session()
{
	return m_client_session_sp;
}
};
