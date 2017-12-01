#pragma once
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <vector>
#include <queue>
#include <pthread.h>
class ThreadPool;
class MutexLock;
class ThreadCondition;
class ThreadTask;
class ThreadNoncopyable
{
protected:
	ThreadNoncopyable() {}
	~ThreadNoncopyable() {}
private:
	ThreadNoncopyable(const ThreadNoncopyable & rhs);
	ThreadNoncopyable & operator=(const ThreadNoncopyable &rhs);
};

class Thread :private ThreadNoncopyable
{
public:
	Thread(ThreadPool& pool);
	~Thread();
	void Start();
	void Join();
	void Run();
private:
	static void* RunInThread(void *arg);
private:
	ThreadPool &m_Pool;
	pthread_t m_ThreadID;
	bool m_IsRunning;
};
class MutexLock :private ThreadNoncopyable
{
public:
	MutexLock();
	~MutexLock();
	void Lock();
	void Unlock();
	pthread_mutex_t * GetPtr();
private:
	pthread_mutex_t mutex;
};
class EasyMutexLock
{
public:
	EasyMutexLock(MutexLock &lock)
		:mutex(lock)
	{
		mutex.Lock();
	}

	~EasyMutexLock()
	{
		mutex.Unlock();
	}
private:
	MutexLock & mutex;
};
class ThreadCondition :private ThreadNoncopyable
{
public:
	ThreadCondition(MutexLock &lock);
	~ThreadCondition();
	void Wait();
	void Notify();
	void NotifyAll();
private:
	pthread_cond_t cond;
	MutexLock &mutex;
};
class ThreadBuffer
{
public:
	ThreadBuffer(int size);
	~ThreadBuffer();
	void Push(ThreadTask* task);
	ThreadTask* Pop();
	bool Empty();
	bool Full();
private:
	MutexLock mutex;
	ThreadCondition notfull;
	ThreadCondition notempty;
	int m_Size;
	std::queue<ThreadTask*> m_TaskQueue;
};


class ThreadTask
{
public:
	ThreadTask();
	~ThreadTask();
	virtual void Process() = 0;

private:

};


class ThreadPool
{
public:
	ThreadPool(int buff_size,int thread_count);
	~ThreadPool();
	void Start();
	void Stop();
	void AddTask(ThreadTask* task);
	ThreadTask* GetTask();
	void Process();
private:
	ThreadBuffer m_Buffer;
	int m_ThreadCount;
	bool m_Running;
	std::vector<Thread*> m_ThreadList;
};
#endif // !__THREADPOOL_H__
