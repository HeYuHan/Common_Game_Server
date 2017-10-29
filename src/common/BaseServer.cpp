#include "BaseServer.h"
#include <event2/event_struct.h>
#include <event2/util.h>

BaseServer::BaseServer():
	m_EvetBase(NULL)
{
	
}

BaseServer::~BaseServer()
{
	if(m_EvetBase)event_base_free(m_EvetBase);
	m_EvetBase = NULL;
}

void BaseServer::Init()
{
#ifdef WIN32  
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
	m_EvetBase = event_base_new();
}

void BaseServer::Run()
{
	event_base_dispatch(m_EvetBase);
	
}

event_base * BaseServer::GetEventBase()
{
	if (m_EvetBase)return &(*m_EvetBase);
	return NULL;
}
