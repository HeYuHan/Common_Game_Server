// test_thread.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <ThreadPool.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <common.h>
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




int main()
{
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

