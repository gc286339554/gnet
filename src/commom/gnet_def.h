#pragma once

//type def begin
#ifdef __GNUC__
	typedef long long			int64;
	typedef int					int32;
	typedef short				int16;
	typedef signed char			int8;
	typedef unsigned long long	uint64;
	typedef unsigned int		uint32;
	typedef unsigned short		uint16;
	typedef unsigned char		uint8;
#else
	typedef signed __int64		int64;
	typedef signed __int32		int32;
	typedef signed __int16		int16;
	typedef signed __int8		int8;
	typedef unsigned __int64	uint64;
	typedef unsigned __int32	uint32;
	typedef unsigned __int16	uint16;
	typedef unsigned __int8		uint8;
#endif

typedef float					f32;
typedef double					f64;
//type def end

#define GNET_OP_HEART_BEAT_PING_PONG	(0xffffffff)
#define INVALID_SERVICE_ID				(0xffffffff)

