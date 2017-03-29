#pragma once
#include "../pre_header.h"
#include "asio/asio.hpp"
#include "net_packet.hpp"
#include "io_service_holder.h"

namespace gnet {
#define BUF_READ_SIZE (8*1024)
template<class _Ty>
class base_session : public std::enable_shared_from_this<_Ty>
{
protected:
	std::chrono::steady_clock::time_point	m_last_packet_time;//上次收到数据的时间点
	asio::ip::tcp::socket					m_socket;
	std::mutex								m_send_list_lock;
	bool									m_is_sending = false;	
	uint8									m_buf_read[BUF_READ_SIZE];
	uint32									m_temp_size = 0;
	uint32									m_session_id = 0;
	uint32									m_timeout = 10;
	net_packet*								m_net_packet_p = nullptr;
	std::list<net_packet*>					m_send_list;
	asio::deadline_timer					m_timeout_monitor;
public:
	virtual void post_net_packet_handler() = 0;
	virtual void timeout_check() = 0;

	base_session(uint32 timeout)
		: m_socket(g_io_service),
		m_timeout_monitor(g_io_service),
		m_timeout(timeout),
		m_last_packet_time(std::chrono::steady_clock::now())
	{
	}
	
	virtual ~base_session()
	{
		if (m_net_packet_p)
		{
			delete m_net_packet_p;
		}

		for (auto it = m_send_list.begin(); it != m_send_list.end(); it++)
		{
			delete *it;
		}
	}
	virtual void start()
	{
		m_last_packet_time = std::chrono::steady_clock::now();
		m_session_id = m_socket.native();
		//m_socket.non_blocking(true);
		//m_socket.set_option(asio::ip::tcp::no_delay(true));
		m_last_packet_time = std::chrono::steady_clock::now();//重新计时
		timeout_check();
		do
		{
			std::unique_lock<std::mutex> lock(m_send_list_lock);
			m_is_sending = false;
			m_send_list.clear();
		} while (0);
		do_read();
	}
	virtual void stop()
	{
		m_timeout_monitor.cancel();
		close_socket();
	}
	uint32 get_session_id()
	{
		return m_session_id;
	}

	uint16 get_local_ip_port(std::string& ip)
	{
		asio::error_code error;
		asio::ip::tcp::endpoint ep = m_socket.local_endpoint(error);
		if (error)
		{
			ip.clear();
			return 0;
		}

		ip = ep.address().to_v4().to_string(error);
		if (error)
		{
			ip.clear();
			return 0;
		}
		return ep.port();
	}
	uint16 get_remote_ip_port(std::string& ip)
	{
		asio::error_code error;
		asio::ip::tcp::endpoint ep = m_socket.remote_endpoint(error);
		if (error)
		{
			return 0;
		}

		ip = ep.address().to_v4().to_string(error);
		if (error)
		{
			return 0;
		}

		return ep.port();
	}
	void do_read()
	{
		m_socket.async_read_some(asio::buffer(m_buf_read + m_temp_size, BUF_READ_SIZE - m_temp_size),
			std::bind(&base_session::handle_read,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	}
	void format_net_packet()
	{
		if (m_temp_size > 0)
		{
			if (!m_net_packet_p)
			{
				if (m_temp_size >= DATA_HEAD_LEN)
				{
					uint32 len = *((uint32*)m_buf_read);
					if (len > DATA_MAX_LEN)
					{
						stop();
						return;
					}
					m_net_packet_p = new gnet::net_packet(len);
					m_net_packet_p->put_buff(m_buf_read, std::min(len, m_temp_size));

					if (len < m_temp_size)
					{
						memcpy(m_buf_read, m_buf_read + len, m_temp_size - len);
						m_temp_size = m_temp_size - len;
					}
					else
					{
						m_temp_size = 0;
					}
				}
			}
			else
			{
				uint32 leftData = m_net_packet_p->get_buff_len() - m_net_packet_p->get_data_pos();
				m_net_packet_p->put_buff(m_buf_read, std::min(leftData, m_temp_size));

				if (leftData < m_temp_size)
				{
					memcpy(m_buf_read, m_buf_read + leftData, m_temp_size - leftData);
					m_temp_size = m_temp_size - leftData;
				}
				else
				{
					m_temp_size = 0;
				}
			}
		}
	}
	void send_heat_beat()
	{
		static int index = 0;
		gnet::net_packet* packet = new gnet::net_packet();	//构造心跳包
		packet->put_uint32(0);
		packet->put_uint32(GNET_OP_HEART_BEAT_PING_PONG);
		packet->put_uint32(index++);
		packet->flip();
		do_write(packet);
	}
	bool post_net_packet()
	{
		if (m_net_packet_p && (m_net_packet_p->get_buff_len() == m_net_packet_p->get_data_pos()))
		{// full data
			m_net_packet_p->flip();
			//post data to right place
			post_net_packet_handler();
			//delete m_net_packet_p;  由消息处理着负责销毁
			m_net_packet_p = NULL;
			return true;
		}
		return false;
	}
	void handle_read(const std::error_code& error, size_t bytes_read)
	{
		if (!error)
		{
			//只要收到一个包，就重置客户端的超时时间
			m_last_packet_time = std::chrono::steady_clock::now();
			try
			{
				m_temp_size = bytes_read + m_temp_size;
				do
				{
					format_net_packet();
				} while (post_net_packet());

				do_read();
			}
			catch (...)
			{
			}
		}
		else
		{
			stop();
		}
	}
	void do_write(net_packet* packet)
	{
		if (!m_socket.is_open())
		{
			delete packet;
			return;
		}
		std::unique_lock<std::mutex> lock(m_send_list_lock);
		if (m_send_list.empty() && !m_is_sending)
		{
			m_is_sending = true;
			asio::async_write(m_socket,
				asio::buffer((*packet).get_buff(), (*packet).get_data_len()),
				std::bind(&base_session::handle_write,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2,
					packet));
		}
		else
		{
			m_send_list.push_back(packet);
		}
	}
	void handle_write(const std::error_code& error, size_t size_, gnet::net_packet* lastPacket)
	{
		delete lastPacket;

		if (!error)
		{
			std::unique_lock<std::mutex> lock(m_send_list_lock);
			m_is_sending = false;
			if (!m_send_list.empty())
			{
				m_is_sending = true;
				gnet::net_packet* packet = *(m_send_list.begin()); m_send_list.pop_front();
				asio::async_write(m_socket,
					asio::buffer((*packet).get_buff(), (*packet).get_data_len()),
					std::bind(&base_session::handle_write,
						shared_from_this(),
						std::placeholders::_1,
						std::placeholders::_2,
						packet));
			}
		}
		else
		{
			stop();
		}
	}
	asio::ip::tcp::socket& socket()
	{
		return m_socket;
	}
	void close_socket()
	{
		if (m_socket.is_open())
		{
			try {
				m_socket.close();
			}
			catch (...) {

			}
		}
	}
	bool is_timeout()
	{
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(end - m_last_packet_time).count() > m_timeout)//和timer.restart的间隔秒数
		{
			return true;
		}
		return false;
	}

};

};

