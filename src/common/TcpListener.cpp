#include "TcpListener.h"
#include<string.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif // !_WIN32
TcpListener::TcpListener(event_base & base):
	m_BaseEvent(base),
	m_Listener(NULL)
{
}
TcpListener::~TcpListener()
{
	if (m_Listener)
	{
		evconnlistener_free(m_Listener);
		m_Listener = NULL;
	}
}
void TcpListener::CreateTcpServer(const char *ip, int port, int max_client)
{
	memset(&m_ListenAddr, 0, sizeof(m_ListenAddr));
	m_ListenAddr.sin_family = AF_INET;
	m_ListenAddr.sin_addr.s_addr = inet_addr(ip);
	m_ListenAddr.sin_port = htons(port);

	m_Listener = evconnlistener_new_bind(&m_BaseEvent,
		ListenEvent, this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		max_client, (sockaddr*)&m_ListenAddr, sizeof(sockaddr_in));
}


void TcpListener::ListenEvent(evconnlistener * listener, evutil_socket_t fd, sockaddr * sock, int socklen, void * arg)
{
	printf("accept a client %d\n", fd);
	TcpListener *l = (TcpListener*)arg;
	l->OnTcpAccept(fd, sock);
}