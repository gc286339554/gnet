#include "client_session.h"

using namespace gnet;
client_session::client_session(const char* ip, uint32 port, uint32 timeout) 
:base_session(timeout),
m_endpoint(asio::ip::address_v4::from_string(ip), port)
{
}
void client_session::stop()
{
	close_socket();
}
void client_session::start()
{
	if (connect()){
		base_session::start();
	}
}
bool client_session::connect()
{
	close_socket();//关掉有可能存在的连接
	asio::error_code error;
	m_socket.connect(m_endpoint, error);
	if (error){
		//error.message()
		return false;//连接失败
	}
	return true;
}
void client_session::post_net_packet_handler()
{
	m_client_session_msg_handler(m_net_packet_p);
}

void client_session::timeout_check()
{
	auto This = shared_from_this();
	m_timeout_monitor.expires_from_now(boost::posix_time::seconds(m_timeout / 2));
	m_timeout_monitor.async_wait(std::bind([This]()
	{
		if (This->is_timeout()) {
			if (This->connect()) {
				This->base_session::start();
			}else{
				This->timeout_check();//继续添加回调。
			}
		}
		else {
			This->send_heat_beat();
			This->timeout_check();//继续添加回调。
		}
	}));
}

void client_session::set_client_session_msg_handler(std::function<void(net_packet*)>& handler)
{
	m_client_session_msg_handler = handler;
}
