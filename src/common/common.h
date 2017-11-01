#pragma once
#ifndef __COMMON__H_
#define __COMMON__H_


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
