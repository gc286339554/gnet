#pragma once
#include "../commom/pre_header.h"
#include "../commom/net/server_session_mgr.hpp"
#include "../commom/net/server_acceptor.hpp"
namespace gnet {

class gateway
{
public:
	gateway(uint32 port, uint32 timeout);
	~gateway();

	void start();
	void set_server_session_msg_handler(std::function<void(net_packet*, uint32)>& handler);
	bool send_msg_to_client(uint32 sid, net_packet* packet);
private:
	uint32 m_port;
	std::shared_ptr<server_session_mgr> m_server_session_mgr_sp;
    server_acceptor m_server_acceptor;
};

};