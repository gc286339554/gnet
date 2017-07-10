#pragma once
#include "../pre_header.h"
#include "asio.hpp"
#include "data_packet.hpp"
#include "io_service_holder.h"
#include "base_session.hpp"

namespace gnet {
class client_session : public base_session<client_session>
{
private:
    asio::ip::tcp::endpoint			                        m_endpoint;
	std::function<void(data_packet*)>						m_net_msg_handler;
	bool													m_auto_reconnect;
	bool													m_last_state_is_connected = false;
	std::function<void(bool)>								m_session_state_change_handler;
	std::atomic_bool										m_do_heart_beat = true;
public:
	client_session(const char* ip, uint32 port, uint32 timeout, bool auto_reconnect = true);
	void stop();
	void start();
	bool connect();
	void connect_state_change(bool state_is_connected);
	void post_data_packet_handler();
	void timeout_check();
	void set_net_msg_handler(std::function<void(data_packet*)>& handler);
	bool is_auto_reconnect();
	void set_session_state_change_handler(std::function<void(bool)>& handler);
	void set_do_heart_beat(bool is_do);

	virtual void close_socket();
};
};