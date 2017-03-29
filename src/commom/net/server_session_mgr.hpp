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
			ins.second->stop();
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
private:
	std::unordered_map<uint32, std::shared_ptr<server_session> >	sessionList;
	std::mutex			sessionListLock;
};
};