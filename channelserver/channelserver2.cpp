// channelserver.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>

#include <assert.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <common.h>
#include <Timer.h>
#include <ThreadPool.h>

void time_cb(float t, void *arg)
{
	printf("%s   %f\n", arg, t);
}

int main()
{
#ifdef WIN32  
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
	struct event_base *base = event_base_new();
	Timer t;
	t.Init(base, 0.01f, time_cb, "asdfsdf", true);
	t.Begin();
	event_base_dispatch(base);
	getchar();
	return 0;
    return 0;
}

