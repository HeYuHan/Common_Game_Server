#pragma once
#ifndef __COMMON__H_
#define __COMMON__H_
#define GAME_MSG 254
#define KEEP_ALIVE_MSG 253
#define KEEP_ALIVE_TIME 5.0f

//lib
#if _WIN32
//pthread
#pragma comment(lib,"./../3rd/pthread/lib/x86/pthreadVC2.lib")
//libevent
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"./../3rd/libevent/libevent.lib")
#pragma comment(lib,"./../3rd/libevent/libevent_core.lib")
#pragma comment(lib,"./../3rd/libevent/libevent_extras.lib")
//RakNet_VS2008_LibStatic_Release_Win32.lib
//raknet
#ifdef _DEBUG
#pragma comment(lib,"./../3rd/RakNet/Lib/RakNet_VS2008_LibStatic_Debug_Win32.lib")
#else
#pragma comment(lib,"./../3rd/RakNet/Lib/RakNet_VS2008_LibStatic_Release_Win32.lib")

#endif

#endif


typedef unsigned char byte;
typedef byte uint8;
typedef long int64;
typedef unsigned long uint64;

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef long long llong;
typedef unsigned long long ullong;

#ifdef LINUX
#if __GNUC__>2
#define USING_VECOTR using namespace __gnu_cxx;
#else
#define USING_VECOTR using namespace stdext;
#endif

#else
#define USING_VECOTR using namespace std;
#endif // _WIN32

#define MAX(a,b) (a)>(b)?(a):(b)
#define MIN(a,b) (a)<(b)?(a):(b)

#define FOR_EACH_LIST(__TYPE__,__LIST__,__ITER__) \
	std::vector<__TYPE__*>::iterator iter##__ITER__; \
	for( iter##__ITER__ =  __LIST__.begin(); iter##__ITER__ != __LIST__.end(); iter##__ITER__++)


void ThreadSleep(unsigned int s);

#ifdef _WIN32
int gettimeofday(struct timeval *tp, void *tzp);
#endif // _WIN32


#endif // !__COMMON__H_
//#include "BaseServer.h"
//#include "NetworkConnection.h"
//#include "objectpool.h"
//#include "TcpConnection.h"
//#include "ThreadPool.h"
//#include "Timer.h"
//#include "UdpConnection.h"
//#include "UdpListener.h"


