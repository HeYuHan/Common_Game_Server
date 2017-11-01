#include "common.h"
#if _WIN32
#include <time.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif



void ThreadSleep(unsigned int s)
{
#if _WIN32
	Sleep(s * 1000);
#else
	sleep(s * 1000);
#endif
}

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
#endif
