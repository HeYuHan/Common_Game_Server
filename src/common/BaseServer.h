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
	virtual bool Init();
	virtual int Run();
};


#endif // !__BASESERVER_H__

