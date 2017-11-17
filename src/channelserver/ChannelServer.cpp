
#include "ChannelServer.h"
#include "Client.h"
#include "Room.h"
#include <FileReader.h>
#include <json/json.h>
#include <log.h>
#include <tools.h>

ChannelServer gChannelServer;
//weapon
WeaponInfo gWeaponList[WeaponType::WeaponCount - 1];
//brith
Vector3 gBrithPoints[10];
int gBrithPointsCount = 0;
int gBrithPointIndex = 0;
//dropitem


static void ChannnelUpdate(float time, void *channel)
{
	gChannelServer.Update(time);
}
static void ParseJosnVector3(Json::Value json,Vector3 &v3)
{
	v3.x = json[((Json::UInt)0)].asDouble();
	v3.y = json[((Json::UInt)1)].asDouble();
	v3.z = json[((Json::UInt)2)].asDouble();
}
ChannelServer::ChannelServer():
	gDropRefreshPointsCount(0)
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
	if (!m_DropItemPool.Initialize(m_Config.max_drop_item))
	{
		return false;
	}
	m_RoomList.clear();
	if (!CreateUdpServer(m_Config.ip, m_Config.port, m_Config.pwd,m_Config.max_client))
	{
		return false;
	}
	//parse data config
	Json::Value root;
	bool ret = ReadJson(root, m_Config.data_config_path);
	Json::Value weapon_list;
	//weapon
	memset(&gWeaponList, 0, sizeof(gWeaponList));
	ret = ret && !(weapon_list = root["m_InfoList"]).isNull();
	ret = ret && weapon_list.isArray() && weapon_list.size() == WeaponType::WeaponCount - 1;
	if (ret)
	{
		int index = 0;
		for (Json::ValueIterator it = weapon_list.begin(); it != weapon_list.end(); it++, index++)
		{
			Json::Value config = *it;
			WeaponInfo* info = &gWeaponList[index];
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
		log_error("parse weapon config error path:%s", m_Config.data_config_path);
		return false;
	}
	Json::Value brith_points;
	ret = !(brith_points = root["BrithPonits"]).isNull();
	ret = ret && brith_points.isArray();
	if (ret)
	{
		
		gBrithPointsCount = 0;
		for (Json::ValueIterator it = brith_points.begin(); it != brith_points.end(); it++, gBrithPointsCount++)
		{
			ParseJosnVector3(*it,gBrithPoints[gBrithPointsCount]);
		}
	}
	else
	{
		log_error("parse brith point config error path:%s", m_Config.data_config_path);
		return false;
	}

	Json::Value drop_itmes = root["DropItem"];
	if (!drop_itmes.isNull())
	{
		Json::Value drop_points = drop_itmes["Points"];
		gDropRefreshPointsCount = drop_points.size();
		gDropRefreshPointsCount= gDropRefreshPointsCount< MAX_DROP_POINT_COUNT? gDropRefreshPointsCount: MAX_DROP_POINT_COUNT;
		int index = 0;
		for (Json::ValueIterator it = drop_points.begin(); it != drop_points.end() && index<gDropRefreshPointsCount; it++, index++)
		{
			ParseJosnVector3(*it, gDropRefreshPoints[index]);
		}
		Json::Value refresh_items = drop_itmes["Refresh"];
		for (Json::ValueIterator it = refresh_items.begin(); it != refresh_items.end(); it++, index++)
		{
			int type = (*it)["m_Type"].asInt();
			index = type - 1;
			gDropRefreshItems[index].m_Type = (DropItemType)(*it)["Type"].asInt();
			gDropRefreshItems[index].m_RefreshTime = (*it)["m_RefreshTime"].asDouble();
			gDropRefreshItems[index].m_RefreshCount = (*it)["m_RefreshCount"].asInt();
			gDropRefreshItems[index].m_Duration = (*it)["m_Duration"].asDouble();
			gDropRefreshItems[index].m_StartTime = (*it)["m_StartTime"].asDouble();
		}
		memset(&gSkillInfos, 0, sizeof(gSkillInfos));
		Json::Value items = drop_itmes["Skills"];
		for (Json::ValueIterator it = items.begin(); it != items.end(); it++)
		{
			int type = (*it)["m_Type"].asInt();
			index = type - 1;
			gSkillInfos[index].m_Type = (DropItemType)type;
			gSkillInfos[index].m_Duration = (*it)["m_Duration"].asDouble();
			gSkillInfos[index].m_CoolDown = (*it)["m_CoolDown"].asDouble();
			gSkillInfos[index].m_UseCount = (*it)["m_UseCount"].asDouble();
			gSkillInfos[index].m_SpeedLimit = (*it)["m_SpeedLimit"].asDouble();
			gSkillInfos[index].m_Range = (*it)["m_Range"].asDouble();
		}
	}
	else
	{
		log_error("parse skills config error path:%s", m_Config.data_config_path);
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
		int ret =BaseServer::Run();
		log_debug("server end result %d", ret);
		return ret;
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
}

bool ChannelServer::GetWeaponInfo(WeaponInfo & info, WeaponType type)
{
	for (int i = 0; i < WeaponCount - 1; i++)
	{
		if (gWeaponList[i].Type == type)
		{
			info = gWeaponList[i];
			return true;
		}
	}
	return false;
}

bool ChannelServer::GetSkillInfo(SkillInfo &info, SkillType type)
{
	if (type > 0 && type < DROP_ITEM_COUNT)
	{
		info = gSkillInfos[type - 1];
	}
	return false;
}

bool ChannelServer::RandomBrithPos(Vector3 & v3)
{
	if (gBrithPointsCount == 0)return false;
	int index = gBrithPointIndex % gBrithPointsCount;
	gBrithPointIndex++;
	v3 = gBrithPoints[index];
	return true;
}

bool ChannelServer::RandomDropPos(Vector3 & v3)
{
	if (gDropRefreshPointsCount == 0 || gDropRefreshPoints == NULL)return false;

	int index = RandomRange(0, gDropRefreshPointsCount - 1);
	v3 = gDropRefreshPoints[index];
	return true;
}


ChannelConfig::ChannelConfig():
	port(9530),
	max_client(20),
	max_room(10),
	max_drop_item(100)
{
	strcpy(ip, "127.0.0.1");
	strcpy(pwd, "channel");
	strcpy(data_config_path, "./Config.json");
}
