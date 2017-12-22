
#include "ChannelServer.h"
#include "Client.h"
#include "Room.h"
#include <FileReader.h>
#include <json/json.h>
#include <log.h>
#include <tools.h>


ChannelServer gChannelServer;
GameConfig gGameConfig;

static void ParseJsonValue(Json::Value json, const char* key,int &value)
{
	if (!json[key].isNull())value = json[key].asInt();
	else
	{

		log_warn("%s config miss use default:%d", key,value);
	}
}
static void ParseJsonValue(Json::Value json, const char* key, float &value)
{
	if (!json[key].isNull())value = json[key].asDouble();
	else
	{

		log_warn("%s config miss use default:%f", key,value);
	}
}
static void ParseJsonValue(Json::Value json, const char* key, bool &value)
{
	if (!json[key].isNull())value = json[key].asBool();
	else
	{

		log_warn("%s config miss use default:%s", key,value?"true":"false");
	}
}
static void ParseJsonValue(Json::Value json, const char* key, char* str,int len)
{
	if (!json[key].isNull())
	{
		memset(str, 0, len);
		int str_len = json[key].asString().size();
		if (str_len < len)
		{
			strcpy(str, json[key].asString().c_str());
		}
	}
	else
	{
		
		log_warn("%s config miss use default:%s", key,str);
	}
}
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
		ChannelClient *client = it->second;
		if (client)
		{
			client->m_MessagePacket = p;
			client->m_KeepAliveTime = 0;
			client->OnRevcMessage();
		}

	}
}

void ChannelServer::OnUdpClientDisconnected(Packet* p)
{
	UdpClientIterator it = m_UdpClientMap.find(p->guid.g);
	if (it != m_UdpClientMap.end())
	{
		if (it->second)
		{
			log_error("client disconnect uid %d", it->second->uid);
			it->second->Disconnect();
		}
		else
		{
			m_UdpClientMap.erase(it);
			m_Socket->CloseConnection(p->systemAddress, true, HIGH_PRIORITY);
		}
	}
}

void ChannelServer::OnUdpAccept(Packet* p)
{
	ChannelClient *c = m_ClientPool.Allocate();
	if (NULL == c)
	{
		CloseClient(p->systemAddress);
		log_error("%s","allocate error client pool is full");
		return;
	}
	c->InitServerSocket(m_Socket, p->systemAddress);
	c->m_ConnectionID = p->guid.g;
	m_UdpClientMap.insert(UdpClientMapPair(p->guid.g, c));
	log_debug("new client connect uid: %d ip:%s", c->uid,p->systemAddress.ToString());
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
	memset(&gGameConfig, 0, sizeof(GameConfig));
	//parse data config
	Json::Value root;
	bool ret = ReadJson(root, m_Config.data_config_path);
	Json::Value weapon_list;
	//weapon
	ret = ret && !(weapon_list = root["m_InfoList"]).isNull();
	ret = ret && weapon_list.isArray() && weapon_list.size() == WeaponType::WeaponCount - 1;
	if (ret)
	{
		int index = 0;
		for (Json::ValueIterator it = weapon_list.begin(); it != weapon_list.end(); it++, index++)
		{
			
			Json::Value config = *it;
			int type = (config["Type"].asInt());
			WeaponInfo* info = &gGameConfig.WeaponList[type-1];
			info->Type = (WeaponType)type;
			ParseJsonValue(config, "Damage", info->Damage);
			ParseJsonValue(config, "AttackTime", info->AttackTime);
			ParseJsonValue(config, "Ammunition", info->Ammunition);
			ParseJsonValue(config, "ReloadTime", info->ReloadTime);
			ParseJsonValue(config, "Tracker", info->Tracker);
			ParseJsonValue(config, "Range", info->Range);
			ParseJsonValue(config, "Speed", info->Speed);
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
		
		gGameConfig.BrithPointsCount = 0;
		for (Json::ValueIterator it = brith_points.begin(); it != brith_points.end(); it++, gGameConfig.BrithPointsCount++)
		{
			ParseJosnVector3(*it, gGameConfig.BrithPoints[gGameConfig.BrithPointsCount]);
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
		gGameConfig.DropRefreshPointsCount = drop_points.size();
		gGameConfig.DropRefreshPointsCount= gGameConfig.DropRefreshPointsCount< MAX_DROP_POINT_COUNT? gGameConfig.DropRefreshPointsCount: MAX_DROP_POINT_COUNT;
		int index = 0;
		for (Json::ValueIterator it = drop_points.begin(); it != drop_points.end() && index<gGameConfig.DropRefreshPointsCount; it++, index++)
		{
			ParseJosnVector3(*it, gGameConfig.DropRefreshPoints[index]);
		}
		Json::Value refresh_items = drop_itmes["Refresh"];
		for (Json::ValueIterator it = refresh_items.begin(); it != refresh_items.end(); it++, index++)
		{
			int type = (*it)["m_Type"].asInt();
			index = type;
			gGameConfig.DropRefreshItems[index].m_Type = (DropItemType)type;
			//gDropRefreshItems[index].m_RefreshTime = (*it)["m_RefreshTime"].asDouble();
			gGameConfig.DropRefreshItems[index].m_RefreshCount = (*it)["m_RefreshCount"].asInt();
			//gDropRefreshItems[index].m_Duration = (*it)["m_Duration"].asDouble();
			gGameConfig.DropRefreshItems[index].m_StartTime = (*it)["m_StartTime"].asDouble();
		}
		Json::Value items = drop_itmes["Skills"];
		for (Json::ValueIterator it = items.begin(); it != items.end(); it++)
		{
			int type = (*it)["m_Type"].asInt();
			index = type;
			gGameConfig.SkillInfos[index].m_Type = (DropItemType)type;
			gGameConfig.SkillInfos[index].m_Duration = (*it)["m_Duration"].asDouble();
			gGameConfig.SkillInfos[index].m_CoolDown = (*it)["m_CoolDown"].asDouble();
			Json::Value user_data = (*it)["m_UserData"];
			if (!user_data.isNull())
			{
				for (Json::UInt u = 0; u < user_data.size(); u++)
				{
					gGameConfig.SkillInfos[index].m_UserData[u] = user_data[u].asDouble();
				}
			}
		}
	}
	else
	{
		log_error("parse skills config error path:%s", m_Config.data_config_path);
		return false;
	}
	Json::Value game_config = root["GameConfig"];
	if (!game_config.isNull())
	{
		ParseJsonValue(game_config, "m_MaxHp", m_Config.max_health);
		ParseJsonValue(game_config, "m_MaxRoom", m_Config.max_room);
		ParseJsonValue(game_config, "m_MaxClient", m_Config.max_client);
		ParseJsonValue(game_config, "m_MaxGameTime", m_Config.max_game_time);
		ParseJsonValue(game_config, "m_MaxBlanceTime", m_Config.max_blance_time);
		ParseJsonValue(game_config, "m_LoadingTime", m_Config.max_loading_time);
		ParseJsonValue(game_config, "m_RebirthTime", m_Config.rebirth_time);
		ParseJsonValue(game_config, "m_ReadyTime", m_Config.max_ready_time);
		//log
		ParseJsonValue(game_config, "m_LogName", gLogger.name,64);
		ParseJsonValue(game_config, "m_LogPath", gLogger.fileName,128);
		ParseJsonValue(game_config, "m_LogToConsole", gLogger.m_LogToConsole);
	}
	Json::Value level_config = root["LevelReward"];
	if (!level_config.isNull())
	{
		gGameConfig.LevelRewardCount = MIN(level_config.size(), MAX_LEVEL_REWARD);
		for (Json::UInt i = 0; i < level_config.size(); i++)
		{
			int index = level_config[i][(uint)0].asInt();
			if (index > MAX_LEVEL_REWARD)continue;
			index -= 1;
			LevelRewardInfo &info = gGameConfig.LevelReward[index];
			info.m_RewardHP= level_config[i][1].asInt();
			info.m_RewardAttack = level_config[i][2].asInt();
			info.m_RequireEnergy = level_config[i][3].asInt();
		}
	}
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
	if (!m_DropItemPool.Initialize(gGameConfig.DropRefreshPointsCount))
	{
		return false;
	}
	
	if (!CreateUdpServer(m_Config.ip, m_Config.port, m_Config.pwd, 1000))
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
		m_RoomList.clear();
		m_UpdateTimer.Init(0, ChannnelUpdate, this, true);
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
			//log_debug("%s\n", "create new room in cache");
			room->Init();
			return room;
		}
	}
	ChannelRoom* room = m_RoomPool.Allocate();
	if (room)
	{
		//log_debug("%s\n", "create new room in memory");
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
	RemoveClient(c->m_ConnectionID);
}

void ChannelServer::RemoveClient(uint64_t connectionID)
{
	UdpClientIterator iter = m_UdpClientMap.find(connectionID);
	if (iter != m_UdpClientMap.end())
	{
		if (iter->second)
		{
			log_debug("remove client uid:%d", iter->second->uid);
			m_ClientPool.Free(iter->second->uid);
			
		}
		m_UdpClientMap.erase(iter);
		
	}
}

bool ChannelServer::GetWeaponInfo(WeaponInfo & info, WeaponType type)
{
	for (int i = 0; i < WeaponCount - 1; i++)
	{
		if (gGameConfig.WeaponList[i].Type == type)
		{
			info = gGameConfig.WeaponList[i];
			return true;
		}
	}
	return false;
}

bool ChannelServer::GetSkillInfo(SkillInfo &info, SkillType type)
{
	if (type > 0 && type < DROP_ITEM_COUNT)
	{
		info = gGameConfig.SkillInfos[type - 1];
	}
	return false;
}

bool ChannelServer::RandomBrithPos(Vector3 & v3)
{
	if (gGameConfig.BrithPointsCount == 0)return false;
	int index = gGameConfig.BrithPointIndex % gGameConfig.BrithPointsCount;
	gGameConfig.BrithPointIndex++;
	v3 = gGameConfig.BrithPoints[index];
	return true;
}

bool ChannelServer::RandomDropPos(Vector3 & v3)
{
	if (gGameConfig.DropRefreshPointsCount == 0)return false;

	int index = RandomRange(0, gGameConfig.DropRefreshPointsCount - 1);
	v3 = gGameConfig.DropRefreshPoints[index];
	return true;
}

bool ChannelServer::GetDropItemPos(Vector3 & v3, int index)
{
	if (index < 0 || index >= gGameConfig.DropRefreshPointsCount)return false;
	v3 = gGameConfig.DropRefreshPoints[index];
	return true;
}


ChannelConfig::ChannelConfig():
	port(9530),
	max_client(10),
	max_room(1),
	max_health(3000),
	max_game_time(60*5),
	max_blance_time(30),
	max_loading_time(5),
	max_ready_time(3),
	rebirth_time(5)
{
	strcpy(ip, "127.0.0.1");
	strcpy(pwd, "channel");
	strcpy(data_config_path, "./Config.json");
}
