#include "../../src/3rd/log/glog.h"
#include "game_gate.h"
using namespace gnet;
game_gate::game_gate(uint32 server_port, uint32 server_timeout, uint32 client_port, uint32 client_timeout)
:m_server_port(server_port),
m_server_timeout(server_timeout),
m_client_port(client_port),
m_client_timeout(client_timeout),
m_server_gate(m_server_port, m_server_timeout),
m_client_gate(m_client_port, m_client_timeout)
{
}
game_gate::~game_gate()
{
}

void game_gate::init()
{
	std::function<void(data_packet*, uint32)> handler = std::bind(&game_gate::server_net_msg_handler, this, std::placeholders::_1, std::placeholders::_2);
	m_server_gate.set_net_msg_handler(handler);
	std::function<void(uint32)> handler01 = std::bind(&game_gate::server_session_close_handler, this, std::placeholders::_1);
	m_server_gate.set_session_close_handler(handler01);

	std::function<void(data_packet*, uint32)> c_handler = std::bind(&game_gate::client_net_msg_handler, this, std::placeholders::_1, std::placeholders::_2);
	m_client_gate.set_net_msg_handler(c_handler);
	std::function<void(uint32)> c_handler01 = std::bind(&game_gate::client_session_close_handler, this, std::placeholders::_1);
	m_client_gate.set_session_close_handler(c_handler01);	
}
void game_gate::start()
{
	m_server_gate.start();
	m_client_gate.start();
}

void game_gate::server_net_msg_handler(data_packet* net_msg, uint32 sid)
{//网络线程
	net_msg->start_read();
	server_session_info* info = &m_server_session_info_map[sid];
	uint32 op = net_msg->get_op();
	if (info->auth)
	{
		if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_END)
		{
			//需要手动重新组包，比较繁琐，效率低下，后续优化
			uint32 client_sid = net_msg->get_uint32();
			data_packet* net_msg_to_c = g_data_packet_pool.get_data_packet();
			net_msg_to_c->set_op(op);
			net_msg_to_c->start_write();
			net_msg_to_c->append(reinterpret_cast<const uint8*>(net_msg->get_buff() + net_msg->get_data_pos()), 
				net_msg->get_data_len() - net_msg->get_data_pos());
			net_msg_to_c->flip();
			m_client_gate.send_msg_to_client(client_sid, net_msg_to_c);
		}
		else if (op > OP_SS_BEGIN && op < OP_SS_END)
		{
			thread_safe_list<uint32> * t_list = &m_server_op_route[op];
			t_list->for_each([this, net_msg, sid](uint32 ssid)
			{
				if (ssid != sid)
				{
					data_packet* t_net_msg_to_s = g_data_packet_pool.get_data_packet();
					t_net_msg_to_s->copy_from(net_msg);
					m_server_gate.send_msg_to_client(ssid, t_net_msg_to_s);
				}
			});
		}
		else if (op > OP_SG_GS_BEGIN && op < OP_SG_GS_END)
		{
			switch (op)
			{
			case OP_SG_AUTH:
				LOG(WARNING) << "auth agin,server sid " << info->server_id;
				break;
			case OP_SG_REG_LISTEN_OP:
				{
					uint32 count = net_msg->get_uint32();
					for (uint32 i = 0; i<count; i++)
					{
						uint32 listen_op = net_msg->get_uint32();
						thread_safe_list<uint32> * t_list = &m_server_op_route[listen_op];
						t_list->push_back(sid);
						info->listen_op.push_back(listen_op);
					}
				}
				break;
			case OP_SG_CLIENT_AUTH:
				{
					uint32 csid = net_msg->get_uint32();
					bool suc = net_msg->get_bool();
					if (suc)
					{
						m_client_session_info_map[csid].auth = true;
					}
				}
				break;
			case OP_SG_KICK_CLIENT:
				{
					uint32 csid = net_msg->get_uint32();
					m_client_gate.kick_session(csid);
				}
				break;

			default:
				break;
			}	
		}
		else
		{
			m_server_gate.kick_session(sid);
			LOG(ERROR) << "kick,server sid " << info->server_id << " op not in array";
		}
	}
	else
	{
		if (op == OP_SG_AUTH)
		{
			std::string auth = net_msg->get_string();
			if (m_auth == auth)
			{
				info->auth = true;
				info->sid = sid;
				info->server_id = net_msg->get_uint32();
			}
			else
			{
				m_server_gate.kick_session(sid);
				LOG(ERROR) << "kick,server sid " << sid << " auth fail with " << auth;
			}
		}
		else
		{
			m_server_gate.kick_session(sid);
			LOG(ERROR) << "kick,server " << sid << ", send op " << op << ",but not auth";
		}
	}
	g_data_packet_pool.put_data_packet(net_msg);
}
void game_gate::server_session_close_handler(uint32 sid)
{//网络线程
	server_session_info* info = &m_server_session_info_map[sid];
	info->listen_op.for_each([this,sid](uint32 op)
	{
		thread_safe_list<uint32> * t_list = &m_server_op_route[op];
		t_list->remove(sid);
	});
	m_server_session_info_map.erase(sid);
	LOG(ERROR) << "server " << sid << "is closed";
}
void game_gate::client_net_msg_handler(data_packet* net_msg, uint32 sid)
{//网络线程
	net_msg->start_read();
	uint32 op = net_msg->get_op();
	client_session_info* info = &m_client_session_info_map[sid];
	if (info->auth)
	{
		if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_END)
		{
			//需要手动重新组包，比较繁琐，效率低下，后续优化
			data_packet* net_msg_to_s = g_data_packet_pool.get_data_packet();
			net_msg_to_s->set_op(op);
			net_msg_to_s->start_write();
			net_msg_to_s->put_uint32(sid);
			net_msg_to_s->append(reinterpret_cast<const uint8*>(net_msg->get_buff() + net_msg->get_data_pos()),
				net_msg->get_data_len() - net_msg->get_data_pos());
			net_msg_to_s->flip();

			thread_safe_list<uint32> * t_list = &m_server_op_route[op];
			t_list->for_each([this, net_msg_to_s](uint32 ssid)
			{
				data_packet* t_net_msg_to_s = g_data_packet_pool.get_data_packet();
				t_net_msg_to_s->copy_from(net_msg_to_s);
				m_server_gate.send_msg_to_client(ssid, t_net_msg_to_s);
			});

			g_data_packet_pool.put_data_packet(net_msg_to_s);
		}
		else
		{
			m_client_gate.kick_session(sid);
			LOG(ERROR) << "kick,client sid " << info->sid << ", op not in array";
		}
	}
	else
	{
		if (op > OP_CS_SC_BEGIN && op < OP_CS_SC_NEED_AUTH)
		{
			data_packet* net_msg_to_s = g_data_packet_pool.get_data_packet();
			net_msg_to_s->set_op(op);
			net_msg_to_s->start_write();
			net_msg_to_s->put_uint32(sid);
			net_msg_to_s->append(reinterpret_cast<const uint8*>(net_msg->get_buff() + net_msg->get_data_pos()),
				net_msg->get_data_len() - net_msg->get_data_pos());
			net_msg_to_s->flip();

			thread_safe_list<uint32> * t_list = &m_server_op_route[op];
			t_list->for_each([this, net_msg_to_s](uint32 ssid)
			{
				data_packet* t_net_msg_to_s = g_data_packet_pool.get_data_packet();
				t_net_msg_to_s->copy_from(net_msg_to_s);
				m_server_gate.send_msg_to_client(ssid, t_net_msg_to_s);
			});

			g_data_packet_pool.put_data_packet(net_msg_to_s);
		}
		else
		{
			m_client_gate.kick_session(sid);
			LOG(ERROR) << "kick,client sid " << sid << ",not auth";
		}
	}
	g_data_packet_pool.put_data_packet(net_msg);
}
void game_gate::client_session_close_handler(uint32 sid)
{//网络线程
	data_packet* net_msg_to_s = g_data_packet_pool.get_data_packet();
	net_msg_to_s->set_op(OP_GS_CLIENT_SOCKET_CLOSE);
	net_msg_to_s->start_write();
	net_msg_to_s->put_uint32(sid);
	net_msg_to_s->flip();

	thread_safe_list<uint32> * t_list = &m_server_op_route[OP_GS_CLIENT_SOCKET_CLOSE];
	t_list->for_each([this, net_msg_to_s](uint32 ssid)
	{
		data_packet* t_net_msg_to_s = g_data_packet_pool.get_data_packet();
		t_net_msg_to_s->copy_from(net_msg_to_s);
		m_server_gate.send_msg_to_client(ssid, t_net_msg_to_s);
	});
	g_data_packet_pool.put_data_packet(net_msg_to_s);	
}
void game_gate::set_gate_id(uint32 gid)
{
	m_gate_id = gid;
}
void game_gate::set_auth(std::string p_auth)
{
	m_auth = p_auth;
}