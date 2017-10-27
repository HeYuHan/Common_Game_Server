#include "common.h"
#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

//lib
#if _WIN32
#pragma comment(lib,"./../3rd/pthread/lib/x86/pthreadVC2.lib")
#endif

void ThreadSleep(unsigned int s)
{
#if _WIN32
	Sleep(s * 1000);
#else
	sleep(s * 1000);
#endif
}