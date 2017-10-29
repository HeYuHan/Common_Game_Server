
#include "pch.h"
#include <Timer.h>
ChannelServer gChannelServer;
Timer m_UpdateTimer;
void ChannnelUpdate(float time, void *channel)
{
	gChannelServer.Update(time);
}
ChannelServer::ChannelServer()
{
	Timer m_UpdateTimer;
}

ChannelServer::~ChannelServer()
{
}

void ChannelServer::OnUdpClientMessage(Packet * p)
{
	UdpClientIterator it = m_UdpClientMap.find(p->guid.g);
	if (it != m_UdpClientMap.end())
	{
		it->second->m_MessagePacket = p;
		it->second->OnRevcMessage();
	}
}

void ChannelServer::OnUdpClientDisconnected(Packet* p)
{
	UdpClientIterator it = m_UdpClientMap.find(p->guid.g);
	if (it != m_UdpClientMap.end())
	{
		it->second->OnDisconnected();
		m_ClientPool.Free(it->second->uid);
	}
}

void ChannelServer::OnUdpAccept(Packet* p)
{
	ChannelClient *c = m_ClientPool.Allocate();
	c->InitServerSocket(m_Socket, p->systemAddress);
	m_UdpClientMap.insert(UdpClientMapPair(p->guid.g, c));
}

void ChannelServer::Init()
{
	BaseServer::Init();
	m_ClientPool.Initialize(256);
	m_UpdateTimer.Init(GetEventBase(), 0.01f, ChannnelUpdate, this, true);
	m_UpdateTimer.Begin();
}

void ChannelServer::Update(float time)
{
	UdpListener::Update();
}
