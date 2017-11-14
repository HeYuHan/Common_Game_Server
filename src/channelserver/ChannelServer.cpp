#include <Timer.h>
#include "ChannelServer.h"
#include "Client.h"
#include "Room.h"
#include <FileReader.h>
#include <json/json.h>
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
		ChannelClient* c = it->second;
		c->OnDisconnected();
		m_ClientPool.Free(c->uid);
		
		
	}
}

void ChannelServer::OnUdpAccept(Packet* p)
{
	ChannelClient *c = m_ClientPool.Allocate();
	c->InitServerSocket(m_Socket, p->systemAddress);
	c->m_ConnectionID = p->guid.g;
	m_UdpClientMap.insert(UdpClientMapPair(p->guid.g, c));
	log_debug("new client connect %s", p->systemAddress.ToString());
}

void ChannelServer::OnKeepAlive(Packet * p)
{
	UdpClientIterator it = m_UdpClientMap.find(p->guid.g);
	if (it != m_UdpClientMap.end())
	{
		it->second->m_MessagePacket = p;
		it->second->OnKeepAlive();
	}
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
	//parse weapon config
	Json::Value root;
	bool ret = ReadJson(root, m_Config.weapon_config_path);
	Json::Value weapon_list;
	ret = ret && !(weapon_list = root["m_InfoList"]).isNull();
	ret = ret && weapon_list.isArray() && weapon_list.size() == WeaponType::WeaponCount - 1;
	if (ret)
	{
		int index = 0;
		for (Json::ValueIterator it = weapon_list.begin(); it != weapon_list.end(); it++, index++)
		{
			Json::Value config = *it;
			WeaponInfo* info = &m_Config.m_WeaponList[index];
			info->Type = (WeaponType)(config["Type"].asInt());
			info->Damage = (config["Damage"].asInt());
			info->AttackTime = config["AttackTime"].asDouble();
			info->Ammunition = config["Ammunition"].asInt();
			info->ReloadTime = config["ReloadTime"].asDouble();
			info->Tracker = config["Tracker"].asBool();
		}
	}
	else
	{
		log_error("parse weapon config error path:%s", m_Config.weapon_config_path);
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
		if ((!check_full ||(check_full && !room->IsFull())) && (room->m_RoomState & state)>0)
		{
			return room;
		}
	}
	return NULL;
}

ChannelRoom * ChannelServer::CreateNewRoom()
{
	RoomIterator it;
	for (it = m_RoomList.begin(); it != m_RoomList.end(); it++)
	{
		ChannelRoom* room = *it;
		if (room->m_RoomState== ROOM_STATE_IDLE)
		{
			log_debug("%s\n", "create new room in cache");
			room->Init();
			return room;
		}
	}
	ChannelRoom* room = m_RoomPool.Allocate();
	if (room)
	{
		log_debug("%s\n", "create new room in memory");
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

void ChannelServer::RemoveClient(ChannelClient * c)
{
	UdpClientIterator iter = m_UdpClientMap.find(c->m_ConnectionID);
	if (iter != m_UdpClientMap.end())
	{
		m_UdpClientMap.erase(iter);
	}
	ChannelRoom* room = m_RoomPool.Get(c->m_RoomID);
	if (room)
	{
		room->ClientLeave(c);
	}
}

bool ChannelServer::GetWeaponInfo(WeaponInfo & info, WeaponType type)
{
	for (int i = 0; i < WeaponCount - 1; i++)
	{
		if (m_Config.m_WeaponList[i].Type == type)
		{
			info = m_Config.m_WeaponList[i];
			return true;
		}
	}
	return false;
}


ChannelConfig::ChannelConfig():
	port(9530),
	max_client(512),
	max_room(100)
{
	strcpy(ip, "127.0.0.1");
	strcpy(pwd, "channel");
	strcpy(weapon_config_path, "./Config.json");
	memset(&m_WeaponList, 0, sizeof(m_WeaponList));
}
