// test_thread.cpp : 定义控制台应用程序的入口点。
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


class MyTask:public ThreadTask
{
public:
	virtual void Process()
	{
		srand(time(NULL));
		int num = rand() % 100;
		char msg[128] = { 0 };
		sprintf_s(msg, "    id:%d", pthread_self());
		std::cout << "product a number:" << num << msg <<std::endl;
		ThreadSleep(2);
	}


};



void time_cb(float t,void *arg)
{
	printf("%s   %f\n", arg,t);
}

int main()
{
#ifdef WIN32  
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
	struct event_base *base= event_base_new();
	Timer t;
	t.Init(base, 0.01f, time_cb, "asdfsdf", true);
	t.Begin();
	event_base_dispatch(base);
	getchar();
	return 0;
	ThreadPool pool(5, 4);
	pool.Start();
	ThreadTask *task = new MyTask;
	while (true)
	{
		pool.AddTask(task);
		ThreadSleep(1);
	}
	printf("======================================== ");
	getchar();
	return 0;
}

