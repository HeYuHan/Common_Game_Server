#include <Timer.h>
#include "ChannelServer.h"
#include "Client.h"
#include "Room.h"
#include <log.h>
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
	log_debug("new client connect %s", p->systemAddress.ToString());
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
	m_RoomList.clear();
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
		log_debug("server run in %s:%d pwd:%s", m_Config.ip, m_Config.port,m_Config.pwd);
		
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

ChannelRoom * ChannelServer::GetRoom(int state, bool check_full)
{
	RoomIterator it;
	for (it = m_RoomList.begin(); it != m_RoomList.end(); it++) 
	{
		ChannelRoom* room = *it;
		if ((!check_full ||(check_full && room->IsFull())) && (room->m_State & state)>0)
		{
			return room;
		}
	}
	return CreateNewRoom();
}

ChannelRoom * ChannelServer::CreateNewRoom()
{
	ChannelRoom* room = m_RoomPool.Allocate();
	if (room)
	{
		room->Init();
		m_RoomList.push_back(room);
	}
	return room;
}

void ChannelServer::FreeRoom(ChannelRoom * room)
{
	if (room)
	{
		room->Clean();
	}
}

ChannelConfig::ChannelConfig():
	port(9530),
	max_client(512),
	max_room(100)
{
	strcpy(ip, "127.0.0.1");
	strcpy(pwd, "channel");
}
