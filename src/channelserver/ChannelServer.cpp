
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

bool ChannelServer::Init()
{
	if (!BaseServer::Init())
	{
		return false;
	}
	if (!m_ClientPool.Initialize(m_Config.max_client))
	{
		return false;
	}
	if (!m_RoomPool.Initialize(m_Config.max_room))
	{
		return false;
	}
	if (!CreateUdpServer(m_Config.ip, m_Config.port, m_Config.pwd,m_Config.max_client))
	{
		return false;
	}

	return true;
}

int ChannelServer::Run()
{
	if (Init())
	{
		m_UpdateTimer.Init(GetEventBase(), 0.01f, ChannnelUpdate, this, true);
		m_UpdateTimer.Begin();
		return BaseServer::Run();
	}
	return -1;
}

void ChannelServer::Update(float time)
{
	UdpListener::Update();
}
