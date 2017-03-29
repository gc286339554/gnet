#pragma once
#include "../pre_header.h"
#include "asio.hpp"
#include "net_packet.hpp"
#include "io_service_holder.h"
#include "base_session.hpp"

namespace gnet {
class client_session : public base_session<client_session>
{
private:
    asio::ip::tcp::endpoint			                        m_endpoint;
	std::function<void(net_packet*)>						m_client_session_msg_handler;
public:
	client_session(const char* ip, uint32 port, uint32 timeout);
	void stop();
	void start();
	bool connect();
	void post_net_packet_handler();
	void timeout_check();
	void set_client_session_msg_handler(std::function<void(net_packet*)>& handler);
};

};