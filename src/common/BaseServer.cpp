#include "BaseServer.h"
#include <event2/event_struct.h>
#include <event2/util.h>
#include "Timer.h"
BaseServer::BaseServer()
{
	
}

BaseServer::~BaseServer()
{
}

bool BaseServer::Init()
{
#ifdef WIN32  
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
	return true;
}

int BaseServer::Run()
{
	return Timer::Loop();
	
}
