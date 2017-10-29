#pragma once
#ifndef __BASESERVER_H__
#define __BASESERVER_H__
#include <event2/event.h>
class BaseServer
{
protected:
	BaseServer();
	~BaseServer();
public:
	virtual void Init();
	virtual void Run();
	struct event_base* GetEventBase();
private:
	struct event_base* m_EvetBase;
};


#endif // !__BASESERVER_H__

