#include "../../src/3rd/log/glog.h"
#include "../commom/pre_header.h"
#include "../commom/service/service_mgr.h"
#include "../extend/game_gate/game_gate.h"

gnet::service_mgr mgr;
int main()
{
	gnet::glog::newInstance("game_gate_test", "log");
	mgr.init(8);
	//gnet::game_gate g_game_gate(9527, 10000, 9528, 10005);
	gnet::game_gate g_game_gate(9527, 10, 9528, 10);
	g_game_gate.init();
	g_game_gate.set_auth("auth");
	g_game_gate.start();
	mgr.start();

	getchar();
	gnet::glog::deleteInstance();
}

