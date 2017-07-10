#pragma once

#include "../pre_header.h"
#include "server_session.h"
namespace gnet
{
class server_session_mgr
{
public:
	server_session_mgr()
	{
		sessionList.clear();
	};
	~server_session_mgr(){};

	void stop()
	{
		std::unique_lock<std::mutex> lock(sessionListLock);
		for (auto ins : sessionList)
		{
			ins.second->close_socket();
		}
		sessionList.clear();
	}
	void add_new_session(std::shared_ptr<server_session>& session)
	{
		std::unique_lock<std::mutex> lock(sessionListLock);
		sessionList.insert(std::make_pair(session->get_session_id(), session));
	}
	void socket_close(std::shared_ptr<server_session>& session)
	{
		m_session_close_handler(session->get_session_id());
		std::unique_lock<std::mutex> lock(sessionListLock);
		auto it = sessionList.find(session->get_session_id());
		if (it != sessionList.end())
		{
			sessionList.erase(it);
		}
	}
	std::shared_ptr<server_session> find_session_by_id(uint32 sid)
	{
		std::unique_lock<std::mutex> lock(sessionListLock);
		auto it = sessionList.find(sid);
		if (it != sessionList.end())
			return it->second;
		return std::shared_ptr<server_session>();
	}
	void set_session_close_handler(std::function<void(uint32)>& handler)
	{
		m_session_close_handler = handler;
	}
	void kick_session(uint32 sid)
	{
		std::unordered_map<uint32, std::shared_ptr<server_session> >::iterator iter;
		bool find = false;
		do 
		{
			std::unique_lock<std::mutex> lock(sessionListLock);
			iter = sessionList.find(sid);
			if (iter != sessionList.end())
			{
				find = true;
			}
		} while (0);
		if (find)
		{
			auto sess = iter->second;
			g_io_service.post(
			[sess]()
			{
				sess->close_socket();
			}
			);
		}
	}
private:
	std::function<void(uint32)> m_session_close_handler;
	std::unordered_map<uint32, std::shared_ptr<server_session> >	sessionList;
	std::mutex			sessionListLock;
};
};