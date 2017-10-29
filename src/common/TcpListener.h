#pragma once
#ifndef __TCPLISTENER_H__
#define __TCPLISTENER_H__
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <string>
class TcpListener
{
public:
	int m_Socket;
	//struct event m_ListenEvent;
	struct event_base &m_BaseEvent;
	struct sockaddr_in m_ListenAddr;
	evconnlistener *m_Listener;
public:
	TcpListener(struct event_base &base);
	~TcpListener();
public:
	void CreateTcpServer(const char *ip, int port, int max_client);
	static void ListenEvent(evconnlistener *listener, evutil_socket_t fd, sockaddr *sock, int socklen, void *arg);
	virtual void OnTcpAccept(int socket, sockaddr*)=0;
};

#endif // !__TCPLISTENER_H__
