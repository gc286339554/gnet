#include "../../src/3rd/log/glog.h"
#include "../../src/extend/mysql_base_server/mysql_base_server.h"
#include "../../src/commom/pre_header.h"
#include "../../src/commom/net/data_packet.hpp"
#include "../../src/extend/leaf_node/leaf_node.h"




enum {
	SD_QUERY_TIME = 0,
	SD_INSERT_DATA,
	SD_DELETE_DATA,
	SD_UPDATE_DATA,
	SD_QUERY_DATA,
}; 
class my_sql_server_logic : public gnet::mysql_base_server_logic
{
public:
	/*non_sql为false，当有1000个客户端快速随机对数据库增删改查时，数据库查询会成为瓶颈，查询业务会堆积，内存会持续上升
	  non_sql为true，5k链接未见业务堆积，内存异常情况	
	*/
	bool non_sql = true;

	my_sql_server_logic(const char* service_name, gnet::mysql_connect_param param)
	:mysql_base_server_logic(service_name, param) {};
	~my_sql_server_logic() {};
	
	virtual void init() 
	{
		m_handlers_map[SD_QUERY_TIME] = std::bind(&my_sql_server_logic::handler_query_time, this, std::placeholders::_1, std::placeholders::_2);
		m_handlers_map[SD_INSERT_DATA] = std::bind(&my_sql_server_logic::handler_insert_data, this, std::placeholders::_1, std::placeholders::_2);
		m_handlers_map[SD_DELETE_DATA] = std::bind(&my_sql_server_logic::handler_delete_data, this, std::placeholders::_1, std::placeholders::_2);
		m_handlers_map[SD_UPDATE_DATA] = std::bind(&my_sql_server_logic::handler_update_data, this, std::placeholders::_1, std::placeholders::_2);
		m_handlers_map[SD_QUERY_DATA] = std::bind(&my_sql_server_logic::handler_query_data, this, std::placeholders::_1, std::placeholders::_2);
		
		gnet::mysql_base_server_logic::init();
	}

	void handler_query_time(gnet::data_packet* p_data_from,gnet::data_packet* p_data)
	{
		if (non_sql)
		{
			std::time_t result = std::time(nullptr);
			p_data->put_string(std::asctime(std::localtime(&result)));
			return;
		}
		std::string sql = p_data_from->get_string();
		gnet::mysql_rst_ptr rst = m_mysql_connection.query(sql.c_str());
		if (!rst) {
			LOG(ERROR) << ("query_time failed.\n");
			return;
		}
		if (rst->get_row_count() <= 0) {
			LOG(ERROR) << ("query_time, but there is not data.\n");
			return ;
		}
		const size_t number = 1;
		if (rst->get_field_count() != number) {
			LOG(ERROR) << ("query_time, but field number is not match.\n");
		}

		int i = 0;
		const char* time_str = nullptr;
		do {
			i++;
			gnet::mysql_field *field = rst->fetch();
			for (size_t index = 0; index < number; ++index, ++field)
				time_str = field->get_string();
				char bb[128];
				sprintf(bb,"query_time: %d Row %d Colomn %s \n", i, number, time_str);
				LOG(INFO) << bb;
			//field->GetUInt8() field->GetUInt16() ...

		} while (rst->next_row());
		
		if (time_str)
		{
			p_data->put_string(time_str);
		}
		else
		{
			std::time_t result = std::time(nullptr);
			p_data->put_string(std::asctime(std::localtime(&result)));
		}	
	}
	void handler_insert_data(gnet::data_packet* p_data_from, gnet::data_packet* p_data)
	{
		if (non_sql)
		{
			p_data->put_bool(true);
			return;
		}
		std::string sql = p_data_from->get_string();
		int32 affected_rows = m_mysql_connection.execute(sql.c_str());
		if (affected_rows > 0) {
			LOG(INFO) << "insert_data succeed.\n";
			p_data->put_bool(true);
		}
		else{
			LOG(ERROR) << "insert_data failed.\n";
			p_data->put_bool(false);
		}
	}
	void handler_delete_data(gnet::data_packet* p_data_from, gnet::data_packet* p_data)
	{
		if (non_sql)
		{
			p_data->put_int32(0);
			return;
		}
		std::string sql = p_data_from->get_string();
		int32 affected_rows = m_mysql_connection.execute(sql.c_str());
		p_data->put_int32(affected_rows);
	}
	void handler_update_data(gnet::data_packet* p_data_from, gnet::data_packet* p_data)
	{
		if (non_sql)
		{
			p_data->put_int32(0);
			return;
		}
		std::string sql = p_data_from->get_string();
		int32 affected_rows = m_mysql_connection.execute(sql.c_str());
		p_data->put_int32(affected_rows);
	}
	void handler_query_data(gnet::data_packet* p_data_from, gnet::data_packet* p_data)
	{
		if (non_sql)
		{
			p_data->put_int32(0);
			return;
		}
		std::string sql = p_data_from->get_string();
		gnet::mysql_rst_ptr rst = m_mysql_connection.query(sql.c_str());
		if (!rst) {
			LOG(ERROR) << "query_data failed.\n";
			p_data->put_int32(0);
			return;
		}
		if (rst->get_row_count() <= 0) {
			LOG(ERROR) << "query_data, but there is not data.\n";
			p_data->put_int32(0);
			return;
		}
		const size_t number = 1;
		if (rst->get_field_count() != number) {
			LOG(ERROR) << "query_data, but field number is not match.\n";
			p_data->put_int32(0);
		}

		int i = 0;
		do {
			i++;
			gnet::mysql_field *field = rst->fetch();
			for (size_t index = 0; index < number; ++index, ++field)
			{
				p_data->put_int32(field->get_int32());
			}
			break;

		} while (rst->next_row());
	}
};

class my_sql_server : public gnet::mysql_base_server
{
public:
	my_sql_server(uint32 port, uint32 timeout, uint32 logic_count)
	:mysql_base_server(port,timeout,logic_count){};
	~my_sql_server(){}; 

	virtual void init_logic()
	{
		struct gnet::mysql_connect_param param;
		param.hostname = "127.0.0.1";
		param.port = 3306;
		param.username = "root";
		param.password = "123456";
		param.dbname = "test";
		
		char buff[256];
		for (int i=0; i<m_logic_count; i++)
		{
			sprintf(buff, "my_sql_server_logic_%d", i);
			mgr.add_service(std::make_shared<my_sql_server_logic>(buff, param));
		}
	};

};

int main()
{
	gnet::glog::newInstance("test_mysql_server","log");
	LOG(INFO) << "test_mysql_server start";
	uint32 port = 6954;
	uint32 timeout = 150; 
	uint32 logic_count = 8;
	my_sql_server sss(port, timeout, logic_count);
	sss.init(1);
	
	LOG(INFO) << "test_mysql_server finish";
	getchar();
	gnet::glog::deleteInstance();

    return 0;
}

