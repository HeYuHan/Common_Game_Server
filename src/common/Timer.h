#pragma once
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#ifndef __TIMER_H__
#define __TIMER_H__
typedef void(*TimerCallBack)(float time,void *);
class Timer
{
public:
	Timer();
	~Timer();
	void Init(struct event_base *base, float time, TimerCallBack call_back, void* arg, bool loop = false);
	void Begin();
	void Stop();
private:
	static void timeout_cb(evutil_socket_t fd, short event, void *arg);
public:
	struct timeval m_LastTime;
	bool m_Loop;
	bool m_Stop;
	float m_Time;
	void* m_Arg;
	TimerCallBack m_CallBack;
	struct event *m_TimerEvent;
	struct event_base *m_Base;
	struct timeval m_Tv;
};


#endif // !__TIMER_H__

