#include "mysql_base_server.h"
#include "mysql_gate_service.h"
using namespace gnet;
mysql_base_server::mysql_base_server(uint32 port, uint32 timeout, uint32 logic_count)
:m_port(port),
m_timeout(timeout),
m_logic_count(logic_count)
{
}
mysql_base_server::~mysql_base_server()
{
}

void mysql_base_server::init(uint8 work_thread_count)
{
	//step1
	mgr.init(work_thread_count);
	//step2
	init_logic();
	//step3
	mgr.add_service(std::make_shared<mysql_gate_service>("mysql_gate_service", this));
	mgr.start();
}
