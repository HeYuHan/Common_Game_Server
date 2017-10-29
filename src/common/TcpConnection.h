#pragma once
#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__
#include "NetworkConnection.h"
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
class TcpConnection:public NetworkConnection
{
public:
	TcpConnection();
	~TcpConnection();
	virtual void Update();
	virtual int Read(void* data, int size);
	virtual int Send(void* data, int size);
	void InitSocket(int socket, sockaddr* addr, struct event_base *base);
	bool Connect(const char* ip, int port, event_base * base);
	virtual void DisConnect();
private:
	static void ReadEvent(bufferevent *bev, void *arg);
	static void WriteEvent(bufferevent *bev, void *arg);
	static void SocketEvent(bufferevent *bev, short events, void *arg);
public:
	int m_Socket;
	bufferevent *m_BufferEvent;
};


#endif // !__TCPCONNECTION_H__

