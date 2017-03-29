#include "../commom/pre_header.h"
#include "../leaf_node/leaf_node.h"
#include "../commom/net/io_service_holder.h"
#include "../commom/service/service_mgr.h"

gnet::service_mgr mgr;
int main()
{
	mgr.init(8);
	gnet::io_service_holder::newInstance();
	gnet::io_service_holder::instance()->start_net_work_threads(8);

	class main_service_client :public gnet::service
	{
	public:
		gnet::leaf_node  my_leaf_node;
		main_service_client(const char* service_name) :
			gnet::service(service_name),
			//my_leaf_node("127.0.0.1", 9527, 10)
			my_leaf_node("127.0.0.1", 7788, 5)//192.168.7.164
		{
			std::function<void(gnet::net_packet*)> handler = std::bind(&main_service_client::net_msg_handler, this, std::placeholders::_1);
			my_leaf_node.set_client_session_msg_handler(handler);
			my_leaf_node.start();
		}
		void net_msg_handler(gnet::net_packet* net_msg)
		{
			//main_service_client*
			net_msg->skip_head_for_read();
			int opcode = net_msg->get_uint32();//协议号
			printf("get a full packet from server num is: %d ...\n", net_msg->get_uint32());
			delete net_msg;
		}
		virtual void update()
		{
			static uint32 index = 0;
			index++;
			if (index % 10000 == 0)
			{
				gnet::net_packet* packet = new gnet::net_packet();	//构造心跳包
				packet->put_uint32(0);
				packet->put_uint32(100);
				packet->put_uint32(index);
				packet->flip();

				my_leaf_node.get_session()->do_write(packet);
			}
		};
		virtual void message_handler(std::shared_ptr<gnet::service_msg>& msg_sp)
		{
		};
	};

	mgr.add_service(std::make_shared<main_service_client>("main_service_client"));
	mgr.start();

	getchar();
	gnet::io_service_holder::deleteInstance();
	
}
