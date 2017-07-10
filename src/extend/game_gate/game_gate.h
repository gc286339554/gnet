#pragma once
#include "../commom/pre_header.h"
#include "../gateway/gateway.h"

namespace gnet {
	/*
	template <typename T>
	class thread_safe_list : public std::list<T> {
		std::recursive_mutex m_lock;
	public:
		void push_back(T& value)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			std::list<T>::push_back(value);
		}
		void remove(T& value)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			std::list<T>::remove(value);
		}
	};

	template <typename KT, typename VT>
	class thread_safe_map : public std::unordered_map<KT,VT> {
		std::recursive_mutex m_lock;
	public:
		uint32 erase(KT& key)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			return std::unordered_map<KT, VT>::erase(key);
		};
		VT& operator[](const KT& key)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			return std::unordered_map<KT, VT>::operator[](key);
		}
	};
	*/

	template <typename T>
	class thread_safe_list {
		std::recursive_mutex m_lock;
		std::list<T> m_list;
	public:
		void push_back(T& value)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			m_list.push_back(value);
		}
		void remove(const T& value)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			m_list.remove(value);
		}
		void for_each(std::function<void(T)> func)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			for (auto item : m_list)
			{
				func(item);
			}
		}
	};

	template <typename KT, typename VT>
	class thread_safe_map {
		std::recursive_mutex m_lock;
		std::unordered_map<KT, VT> m_map;
	public:
		uint32 erase(KT& key)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			return m_map.erase(key);
		};
		VT& operator[](const KT& key)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			return m_map[key];
		}
		void for_each(std::function<void(KT,VT)> func)
		{
			std::lock_guard<std::recursive_mutex> lock(m_lock);
			for (auto item : m_map)
			{
				func(item.first, item.second);
			}
		}
	};

	struct server_session_info
	{
		uint32	sid			= 0;
		uint32	server_id	= 0;
		bool	auth		= false;
		thread_safe_list<uint32> listen_op;
	};
	struct client_session_info
	{
		uint32	sid = 0;
		bool	auth = false;
	};
	
	class game_gate {
	public:
		game_gate(uint32 server_port, uint32 server_timeout, uint32 client_port, uint32 client_timeout);
		~game_gate();

		void init();
		void start();

		void server_net_msg_handler(data_packet* net_msg, uint32 sid);
		void server_session_close_handler(uint32 sid);
		void client_net_msg_handler(data_packet* net_msg, uint32 sid);
		void client_session_close_handler(uint32 sid);

		void set_gate_id(uint32 gid);
		void set_auth(std::string p_auth);

	private:
		uint32 m_client_port;//开放给客户端的端口
		uint32 m_server_port;//开放给服务端的端口
		uint32 m_client_timeout;
		uint32 m_server_timeout;
	
		thread_safe_map<uint32, thread_safe_list<uint32> > m_server_op_route;//key: op  val: sid list

		gateway m_client_gate;
		thread_safe_map<uint32, client_session_info> m_client_session_info_map;

		gateway m_server_gate;
		thread_safe_map<uint32, server_session_info> m_server_session_info_map;
		
		uint32 m_gate_id;
		std::string m_auth;
	};
};