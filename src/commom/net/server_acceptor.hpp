#pragma once

#include "server_session_mgr.hpp"
#include "io_service_holder.h"
namespace gnet {
class server_acceptor
{
public:
	server_acceptor(uint32 port, uint32 timeout):
		m_timeout(timeout),
		m_acceptor(g_io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{
	
	}

	void start_accept(std::shared_ptr<server_session_mgr>& server_session_mgr_sp)
	{
		m_server_session_mgr_sp = server_session_mgr_sp;
		std::shared_ptr<server_session> new_session(new server_session(m_timeout));
		m_acceptor.async_accept(new_session->socket(),
			std::bind(&server_acceptor::handle_accept,this, new_session,
			std::placeholders::_1));
	}

	void handle_accept(std::shared_ptr<server_session> new_session,
		const std::error_code& error)	
	{
		if (!error)
		{
			new_session->set_net_msg_handler(m_net_msg_handler);
			new_session->init_session_id();
			new_session->set_server_session_mgr(m_server_session_mgr_sp);
			new_session->start(); 

			new_session.reset(new server_session(m_timeout));
			m_acceptor.async_accept(new_session->socket(),
				std::bind(&server_acceptor::handle_accept, this, new_session,
					std::placeholders::_1));
		}
		else
		{
			//LOG(ERROR) <<  "handle_accept error :" << error.message();
		}
	}
	void stop()
	{
		asio::error_code ec;
		m_acceptor.close(ec);
		if (ec)
		{
			   // An error occurred.
		}
	}
	void set_net_msg_handler(std::function<void(data_packet*, uint32)>& handler)
	{
		m_net_msg_handler = handler;
	}
private:
	asio::ip::tcp::acceptor	m_acceptor;
	std::shared_ptr<server_session_mgr> m_server_session_mgr_sp;
	std::function<void(data_packet*, uint32)> m_net_msg_handler;
	uint32 m_timeout;
};
};