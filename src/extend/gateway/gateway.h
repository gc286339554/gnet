#pragma once
#include "../commom/pre_header.h"
#include "../commom/net/server_session_mgr.hpp"
#include "../commom/net/server_acceptor.hpp"
namespace gnet {

class gateway
{
public:
	gateway(uint32 port, uint32 timeout);//超时时间  需比客户端长
	~gateway();

	void start();
	void set_net_msg_handler(std::function<void(data_packet*, uint32)>& handler);
	void set_session_close_handler(std::function<void(uint32)>& handler);
	bool send_msg_to_client(uint32 sid, data_packet* packet);
	void kick_session(uint32 sid);
private:
	uint32 m_port;
	std::shared_ptr<server_session_mgr> m_server_session_mgr_sp;
    server_acceptor m_server_acceptor;
};

};