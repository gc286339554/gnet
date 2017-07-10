#include "../../src/3rd/log/glog.h"
#include "client_session.h"
#include "opcode.h"

using namespace gnet;
client_session::client_session(const char* ip, uint32 port, uint32 timeout, bool auto_reconnect)
:base_session(timeout),
m_auto_reconnect(auto_reconnect),
m_endpoint(asio::ip::address_v4::from_string(ip), port)
{
}
void client_session::close_socket()
{
	base_session::close_socket();
	if (m_last_state_is_connected)
	{
		connect_state_change(false);
	}
}
void client_session::set_session_state_change_handler(std::function<void(bool)>& handler)
{
	m_session_state_change_handler = handler;
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
void client_session::connect_state_change(bool state_is_connected)
{
	m_last_state_is_connected = state_is_connected;
	if (m_session_state_change_handler)
		m_session_state_change_handler(m_last_state_is_connected);
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
	connect_state_change(true);
	return true;
}
void client_session::post_data_packet_handler()
{
	m_data_packet_p->start_read();
	uint32 opcode = m_data_packet_p->get_op();//协议号
	if (opcode == OP_HEART_BEAT_PING_PONG)
	{//每个消息都检测，效率低下
		g_data_packet_pool.put_data_packet(m_data_packet_p);
		return;
	}
	m_data_packet_p->reset();
	m_net_msg_handler(m_data_packet_p);
}
bool client_session::is_auto_reconnect()
{
	return m_auto_reconnect;
}

void client_session::timeout_check()
{
	if (m_timeout_monitor_finish)
	{
		m_timeout_monitor_finish = false;
		auto This = shared_from_this();
		m_timeout_monitor.expires_from_now(boost::posix_time::seconds(m_timeout / 2));
		m_timeout_monitor.async_wait(std::bind([This]()
		{
			This->m_timeout_monitor_finish = true;
			if (This->is_timeout()) {
				if (This->m_last_state_is_connected)
				{
					This->connect_state_change(false);
				}
				This->clear_data_packet();
				if (This->is_auto_reconnect() && This->connect()) {
					This->base_session::start();
				}
				else {
					This->timeout_check();//继续添加回调。
				}
				LOG(WARNING) << "client timeout\n";
			}
			else {
				if (This->m_do_heart_beat)
				{
					This->send_heart_beat();
				}
				This->timeout_check();//继续添加回调。
			}
		}));
	}
}
void client_session::set_do_heart_beat(bool is_do)
{
	m_do_heart_beat = is_do;
}
void client_session::set_net_msg_handler(std::function<void(data_packet*)>& handler)
{
	m_net_msg_handler = handler;
}
