#pragma once

//S  game server
//G  gate server
//C	 client
enum opcode
{
	//commom def
	OP_INVALID = 0,
	OP_HEART_BEAT_PING_PONG,			//心跳
	
	OP_SG_GS_BEGIN	= 0x40000000,
	OP_SG_GS_END	= 0x4000FFFF,
	OP_CS_SC_BEGIN	= 0x40010000,		//客户单 服务器
	OP_CS_SC_END	= 0x4001FFFF,		//客户单 服务器
	OP_SS_BEGIN		= 0x40020000,		//服务器 服务器
	OP_SS_END		= 0x4002FFFF,		//服务器 服务器
	
	OP_SG_AUTH = OP_SG_GS_BEGIN + 1,	//注册监听的消息
	OP_SG_REG_LISTEN_OP,				//服务器端链接，校验
	OP_SG_CLIENT_AUTH,					//客户端登录后，通知gate
	OP_SG_KICK_CLIENT,					//踢掉客户端链接
	OP_GS_CLIENT_SOCKET_CLOSE,			//客户端掉线

	//commom def

	//game def
	//预留200个协议，登录前使用，拉去一些公告，配置等信息
	OP_CS_LOGIN = OP_CS_SC_BEGIN + 1,
	OP_SC_LOGIN,

	OP_CS_SC_NEED_AUTH,

	OP_CS_GET_BASE_INFO,
	OP_SC_GET_BASE_INFO,


	OP_SS_HELLO = OP_SS_BEGIN+1,
	//game def

	OP_COUNT,
};