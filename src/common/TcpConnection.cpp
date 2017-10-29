#include "TcpConnection.h"
TcpConnection::TcpConnection():
	m_Socket(-1),
	m_BufferEvent(NULL)
{
}

TcpConnection::~TcpConnection()
{
	m_BufferEvent = NULL;
	if (m_Socket > 0)evutil_closesocket(m_Socket);
	m_Socket = -1;
}

void TcpConnection::Update()
{
}

int TcpConnection::Read(void * data, int size)
{
	if (m_BufferEvent)
	{
		return bufferevent_read(m_BufferEvent, data, size);
	}
	return 0;
}

int TcpConnection::Send(void * data, int size)
{
	if (m_BufferEvent)
	{
		bufferevent_write(m_BufferEvent, data, size);
		return size;
	}
	return 0;
}

void TcpConnection::InitSocket(int socket, sockaddr * addr, event_base * base)
{
	m_Socket = socket;
	m_BufferEvent = bufferevent_socket_new(base, socket, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(m_BufferEvent, ReadEvent, WriteEvent, SocketEvent, this);
	bufferevent_enable(m_BufferEvent, EV_READ | EV_PERSIST);
	OnConnected();
}

bool TcpConnection::Connect(const char * ip, int port, event_base * base)
{
	m_BufferEvent = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	if (0 == bufferevent_socket_connect(m_BufferEvent, (sockaddr*)&addr, sizeof(addr)))
	{
		m_Socket = bufferevent_getfd(m_BufferEvent);
		bufferevent_setcb(m_BufferEvent, ReadEvent, WriteEvent, SocketEvent, this);
		bufferevent_enable(m_BufferEvent, EV_READ | EV_PERSIST);
		OnConnected();
		return true;
	}
	else
	{
		bufferevent_free(m_BufferEvent);
		m_BufferEvent = NULL;
		return false;
	}

	
}

void TcpConnection::DisConnect()
{
	evutil_closesocket(m_Socket);
	bufferevent_free(m_BufferEvent);
	m_Socket = -1;
	m_BufferEvent = NULL;
	OnDisconnected();
}

void TcpConnection::ReadEvent(bufferevent * bev, void * arg)
{
	TcpConnection* con = static_cast<TcpConnection*>(arg);
	if (con&&con->stream)con->stream->OnRevcMessage();
}

void TcpConnection::WriteEvent(bufferevent * bev, void * arg)
{
	
}

void TcpConnection::SocketEvent(bufferevent * bev, short events, void * arg)
{
	if (events & BEV_EVENT_EOF) {
		printf("connection closed\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		printf("some other error\n");
	}
	TcpConnection* c = (TcpConnection*)arg;
	evutil_closesocket(c->m_Socket);
	bufferevent_free(bev);
	c->m_Socket = -1;
	c->m_BufferEvent = NULL;
	c->OnDisconnected();
}
