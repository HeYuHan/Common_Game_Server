#include "Timer.h"
#include "common.h"
static float diff_time(timeval &v1, timeval &v2)
{
	float diff_sec = v1.tv_sec - v2.tv_sec;
	float diff_usec = (v1.tv_usec - v2.tv_usec) / 1000000.0f;
	return diff_sec + diff_usec;
}
Timer::Timer():
	m_Base(NULL)
	
{
}

Timer::~Timer()
{
}

void Timer::Init( event_base *base, float time, TimerCallBack call_back, void * arg, bool loop)
{
	m_Loop = loop;
	m_Base = base;
	m_Time = time;
	m_CallBack = call_back;
	m_Arg = arg;
}

void Timer::Begin()
{
	if (NULL == m_Base)return;
	evtimer_assign(&m_TimeOut, m_Base, timeout_cb, this);
	evutil_timerclear(&m_Tv);
	long sec = (long)m_Time;
	long usec = (long)((m_Time - sec) * 1000 * 1000);
	m_Tv.tv_sec = sec;
	m_Tv.tv_usec = usec;
	evutil_gettimeofday(&m_LastTime, NULL);
	event_add(&m_TimeOut, &m_Tv);
}

void Timer::timeout_cb(evutil_socket_t fd, short event, void * arg)
{
	Timer *t = static_cast<Timer*>(arg);
	if (t)
	{
		timeval current;
		evutil_gettimeofday(&current, NULL);
		t->m_CallBack(diff_time(current,t->m_LastTime),t->m_Arg);
		if (t->m_Loop)t->Begin();
	}
	
}
