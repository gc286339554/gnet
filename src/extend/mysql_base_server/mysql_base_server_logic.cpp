#include "mysql_base_server_logic.h"
using namespace gnet;

mysql_base_server_logic::mysql_base_server_logic(const char* service_name, mysql_connect_param param) :
	service(service_name),
	m_mysql_connect_param(param)
{

}
mysql_base_server_logic::~mysql_base_server_logic()
{
}

void mysql_base_server_logic::init()
{
	m_is_valid = m_mysql_connection.connect(m_mysql_connect_param.hostname.c_str(), m_mysql_connect_param.port,
		m_mysql_connect_param.username.c_str(), m_mysql_connect_param.password.c_str(), m_mysql_connect_param.dbname.c_str());
	if (!m_is_valid)
	{
		printf("mysql connect failed !\n");
	}
	else
	{
		printf("mysql connect succeed !\n");
	}
}
void mysql_base_server_logic::update(uint32 ms_delay)
{
	
};
void mysql_base_server_logic::service_msg_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
{
	//step1 取数据
	data_packet* p_data = msg_sp->get_data_packet_p();
	p_data->start_read();
	uint32 opcode = p_data->get_op();
	uint32 index = p_data->get_uint32();
	//step2 执行sql
	auto iter = m_handlers_map.find(opcode);
	if (iter != m_handlers_map.end())
	{
		std::shared_ptr<gnet::service_msg> msg_send = std::make_shared<gnet::service_msg>();
		msg_send->get_data_packet_p()->start_write().set_op(opcode).put_extend_data("sid", p_data->get_extend_data("sid").c_str()).put_uint32(index);
		iter->second(p_data,msg_send->get_data_packet_p());
		msg_send->get_data_packet_p()->end_write();
		//step3 返回执行结果
		send_msg(msg_send, "mysql_gate_service");
	}
	else
	{
		assert(false && "not find opcode");
	}
};