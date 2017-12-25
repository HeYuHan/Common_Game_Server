#include "ThreadPool.h"
Thread::Thread(ThreadPool& pool):
	m_Pool(pool)
{
}

Thread::~Thread()
{
	if (m_IsRunning)
	{
		pthread_detach(m_ThreadID);
		m_IsRunning = false;
	}
}

void Thread::Start()
{
	pthread_create(&m_ThreadID, NULL, Thread::RunInThread, this);
	m_IsRunning = true;
}

void Thread::Join()
{
	pthread_join(m_ThreadID, NULL);
	m_IsRunning = false;
}

void Thread::Run()
{
	m_Pool.Process();
}

void* Thread::RunInThread(void * arg)
{
	Thread *t = static_cast<Thread*>(arg);
	if (NULL != t)t->Run();
	return NULL;
}


ThreadBuffer::ThreadBuffer():
	mutex(),
	notfull(mutex),
	notempty(mutex),
	m_Size(0)
{
}

ThreadBuffer::~ThreadBuffer()
{

}

void ThreadBuffer::Push(ThreadTask * task)
{
	EasyMutexLock lock(mutex);
	while (Full())
	{
		notfull.Wait();
	}
	m_TaskQueue.push(task);
	notempty.Notify();
}

ThreadTask * ThreadBuffer::Pop()
{
	EasyMutexLock lock(mutex);
	while (Empty())
	{
		notempty.Wait();
	}
	ThreadTask * task = m_TaskQueue.front();
	m_TaskQueue.pop();
	notfull.Notify();
	return task;
}

bool ThreadBuffer::Init(int size)
{
	m_Size = size;
	return true;
}

bool ThreadBuffer::Empty()
{
	return m_TaskQueue.empty();
}

bool ThreadBuffer::Full()
{
	return ((int)m_TaskQueue.size())==m_Size;
}


ThreadTask::ThreadTask()
{
}

ThreadTask::~ThreadTask()
{
}



ThreadPool::ThreadPool():
	m_ThreadCount(0),
	m_Running(true)

{
}

ThreadPool::~ThreadPool()
{
	Stop();
}

bool ThreadPool::Start(int buff_size, int thread_count)
{
	m_ThreadCount = thread_count;
	m_ThreadList.reserve(m_ThreadCount);
	if (!m_Buffer.Init(buff_size))
	{
		return false;
	}

	for (int i = 0; i != m_ThreadCount; i++)
	{
		Thread *thread = new Thread(*this);
		m_ThreadList.push_back(thread);
		thread->Start();
	}
	return true;
}

void ThreadPool::Stop()
{
	if (m_Running)
	{
		m_Running = false;
		std::vector<Thread*>::iterator it;
		for (it = m_ThreadList.begin(); it != m_ThreadList.end(); it++)
		{
			(*it)->Join();
			delete *it;
		}
		m_ThreadList.clear();
	}
}

void ThreadPool::AddTask(ThreadTask * task)
{
	m_Buffer.Push(task);
}

ThreadTask * ThreadPool::GetTask()
{
	return m_Buffer.Pop();
}

void ThreadPool::Process()
{
	while (m_Running)
	{
		ThreadTask *task = GetTask();
		if (NULL != task)
		{
			task->Process();
		}
	}
}

MutexLock::MutexLock()
{
	pthread_mutex_init(&mutex, NULL);
}

MutexLock::~MutexLock()
{
	pthread_mutex_destroy(&mutex);
}

void MutexLock::Lock()
{
	pthread_mutex_lock(&mutex);
}

void MutexLock::Unlock()
{
	pthread_mutex_unlock(&mutex);
}

pthread_mutex_t * MutexLock::GetPtr()
{
	return &mutex;
}

ThreadCondition::ThreadCondition(MutexLock & lock) :
	mutex(lock)
{
	pthread_cond_init(&cond, NULL);
}

ThreadCondition::~ThreadCondition()
{
	pthread_cond_destroy(&cond);
}

void ThreadCondition::Wait()
{
	pthread_cond_wait(&cond, mutex.GetPtr());
}

void ThreadCondition::Notify()
{
	pthread_cond_signal(&cond);
}

void ThreadCondition::NotifyAll()
{
	pthread_cond_broadcast(&cond);
}


