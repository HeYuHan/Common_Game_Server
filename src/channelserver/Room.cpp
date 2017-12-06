#include "Room.h"
#include "ChannelServer.h"
#include <common.h>
#include "MessageType.h"
#include "Client.h"
#include <log.h>
#include <algorithm>
#include <tools.h>

static void RoomUpdate(float time, void* arg)
{
	ChannelRoom *room = (ChannelRoom*)arg;
	room->Update(time);
}
ChannelRoom::ChannelRoom():
	uid(0),
	m_RoomState(ROOM_STATE_IDLE),
	m_MaxClient(10)
{
	
}

ChannelRoom::~ChannelRoom()
{
}

void ChannelRoom::Init()
{
	memset(&m_CharacterInfoArray, 0, sizeof(m_CharacterInfoArray));
	memset(&m_DropItemTimers, 0, sizeof(m_DropItemTimers));
	memset(m_DropItemIndex, 0, gGameConfig.DropRefreshPointsCount);
	memset(&m_DropItemIndex[gGameConfig.DropRefreshPointsCount], 1, MAX_DROP_POINT_COUNT- gGameConfig.DropRefreshPointsCount);
	m_LoadingTime = 0;
	m_RoomState = ROOM_STATE_WAIT;
	m_GameTime = 0;
	m_LastGameTime = 0;
	m_ClientList.clear();
	m_UpdateTimer.Init(0.02f, RoomUpdate, this,true);
	m_UpdateTimer.Begin();
}

void ChannelRoom::Clean()
{
	for (std::vector<DropItemInfo*>::iterator it = m_DropItemList.begin(); it != m_DropItemList.end(); it++)
	{
		gChannelServer.m_DropItemPool.Free((*it)->uid);
	}
	m_DropItemList.clear();
	m_ClientList.clear();
	m_RoomState = ROOM_STATE_IDLE;
	m_UpdateTimer.Stop();
}

void ChannelRoom::Update(float time)
{
	if (m_RoomState == ROOM_STATE_LOADING)
	{
		m_LoadingTime += time;
		if (m_LoadingTime >= gChannelServer.m_Config.max_loading_time)
		{
			m_LoadingTime = 0;
			m_RoomState = ROOM_STATE_READY_INGMAE;
		}
	}
	else if (m_RoomState == ROOM_STATE_READY_INGMAE)
	{
		m_LoadingTime += time;
		if (m_LoadingTime >= gChannelServer.m_Config.max_ready_time)
		{
			StartGame();
		}
	}
	else if (m_RoomState == ROOM_STATE_PLAYING)
	{
		PlayingUpdate(time);
	}
	else if (m_RoomState == ROOM_STATE_BALANCE)
	{
		m_BalanceWaitTime -= time;
		if (m_BalanceWaitTime <= 0)
		{
			LeaveBalance();
		}
	}
	
}

void ChannelRoom::PlayingUpdate(float time)
{
	m_GameTime -= time;
	if (m_LastGameTime - m_GameTime >= 1)
	{
		BroadCastGameTime();
	}
	if (m_GameTime <= 0)
	{
		m_GameTime = 0;
		BroadCastGameTime();
		GameToBalance();
	}
	//drop item
	for (int i = DROP_ITEM_START; i < DROP_ITEM_COUNT; i++)
	{
		DropItemRefreshInfo &refresh = gGameConfig.DropRefreshItems[i];
		if (m_DropItemTimers[i] < refresh.m_StartTime)
		{
			m_DropItemTimers[i] += time;
			if (m_DropItemTimers[i] >= refresh.m_StartTime)
			{
				for (int j = 0; j < refresh.m_RefreshCount; j++)
				{
					DropItemInfo* info = CreateDropItem();
					if (info)
					{
						info->m_Type = refresh.m_Type;
						BroadCastCreateDropItem(info);
					}

				}
			}
		}
		
	}

}

bool ChannelRoom::IsFull()
{
	return m_MaxClient == m_ClientList.size();
}

void ChannelRoom::ClientEnter(ChannelClient * c)
{
	m_ClientList.push_back(c);
	for (int i = 0; i < ROOM_MAX_CLIENT; i++)
	{
		if (m_CharacterInfoArray[i].uid == 0)
		{
			c->m_InGameInfo = &m_CharacterInfoArray[i];
			c->m_RoomID = uid;
			c->m_InGameInfo->uid = c->uid;
			for (int j = WeaponType::MachineGun; j < WeaponType::WeaponCount; j++)
			{
				gChannelServer.GetWeaponInfo(c->m_InGameInfo->m_WeaponList[j - 1], (WeaponType)j);
			}
			memcpy(c->m_InGameInfo->m_SkillList, gGameConfig.SkillInfos, sizeof(gGameConfig.SkillInfos));
			for (int i = BUFF_TYPE_START; i < BUFF_TYPE_COUNT; i++)
			{
				c->m_InGameInfo->m_BuffList[i].m_Type = (BuffType)i;
			}
			break;
		}
	}
}



void ChannelRoom::ClientLoading(ChannelClient * c)
{
	
	c->m_GameState = GAME_STATE_LOADING_GAME;
	c->BeginWrite();
	c->WriteByte(SM_GAME_LOGADING);
	c->EndWrite();
}


void ChannelRoom::WriteBlanceData(NetworkStream * stream)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		stream->WriteInt(client->uid);
		stream->WriteInt(client->m_InGameInfo->m_KillCount);
		stream->WriteInt(client->m_InGameInfo->m_DiamondCount);
		stream->WriteFloat(client->m_InGameInfo->m_PlayTime);
	}
}

void ChannelRoom::GameToBalance()
{
	m_RoomState = ROOM_STATE_BALANCE;
	m_BalanceWaitTime = gChannelServer.m_Config.max_blance_time;
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		client->m_GameState = GAME_STATE_IN_BALANCE;
		client->BeginWrite();
		client->WriteByte(SM_INGAME_TO_BALANCE);
		client->WriteByte(m_ClientList.size());
		WriteBlanceData(client);
		client->EndWrite();
	}
}


void ChannelRoom::BroadCastCreateDropItem(DropItemInfo * info)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_CREATE_DROPITEM);
			client->WriteInt(info->uid);
			client->WriteByte(info->m_Type);
			client->WriteVector3(info->m_Position);
			client->EndWrite();
		}
	}
}

void ChannelRoom::BroadCastRemoveDropItem(DropItemInfo * info)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_REMOVE_DROPITEM);
			client->WriteInt(info->uid);
			client->EndWrite();
		}
	}
}

void ChannelRoom::BroadCastSkillUse(uint from_uid, byte skill_type)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_USE_SKILL);
			client->WriteInt(from_uid);
			client->WriteByte(skill_type);
			client->EndWrite();
		}
	}
}



void ChannelRoom::BroadCastBuffState(uint from_uid, uint to_uid, BufferInfo* buff)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_BUFF_STATE);
			client->WriteInt(from_uid);
			client->WriteInt(to_uid);
			client->WriteByte(buff->m_Type);
			client->WriteByte(buff->m_State);
			if (buff->m_State != BUFF_STATE_END)
			{
				client->WriteFloat(buff->m_Duration);
				for (int i = 0; i < 4; i++)
					client->WriteFloat(buff->m_UserData[i]);
			}
			client->EndWrite();
		}
	}
}
void ChannelRoom::LeaveBalance()
{
	m_BalanceWaitTime = 0;
	ClientIterator iter = m_ClientList.begin();
	while (iter != m_ClientList.end())
	{
		(*iter)->Disconnect();
		iter = m_ClientList.begin();
	}
}

DropItemInfo * ChannelRoom::CreateDropItem()
{
	DropItemInfo *info = gChannelServer.m_DropItemPool.Allocate();
	if (info)
	{
		m_DropItemList.push_back(info);
		UpdateDropItemPosition(info);
	}
	return info;
}

void ChannelRoom::UpdateDropItemPosition(DropItemInfo * info)
{
	info->m_PositionIndex = -1;
	int empty_pos[MAX_DROP_POINT_COUNT] = { 0 };
	int count = 0;
	for (int i = 0; i < gGameConfig.DropRefreshPointsCount; i++)
	{
		if (m_DropItemIndex[i] == 0)
		{
			empty_pos[count] = i;
			count++;
		}
	}
	if (count > 0)
	{
		int index = RandomRange(0, count - 1);
		m_DropItemIndex[empty_pos[index]] = 1;
		info->m_PositionIndex = empty_pos[index];
		gChannelServer.GetDropItemPos(info->m_Position, info->m_PositionIndex);
	}
	if (info->m_PositionIndex == -1)
	{
		gChannelServer.RandomDropPos(info->m_Position);
	}
}

void ChannelRoom::RemoveDropItem(DropItemInfo * info)
{
	BroadCastRemoveDropItem(info);
	if (info->m_PositionIndex > 0&&info->m_PositionIndex<gGameConfig.DropRefreshPointsCount)
	{
		m_DropItemIndex[info->m_PositionIndex] = 0;
	}
	UpdateDropItemPosition(info);
	BroadCastCreateDropItem(info);
	//std::vector<DropItemInfo*>::iterator it = find(m_DropItemList.begin(), m_DropItemList.end(), info);
	//if (it != m_DropItemList.end())
	//{
	//	m_DropItemList.erase(it);
	//}
	//gChannelServer.m_DropItemPool.Free(info->uid);
}

bool ChannelRoom::CheckAllClientReadyInGame()
{
	if (m_RoomState != ROOM_STATE_LOADING)return false;
	{
		FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			if (client->m_GameState != GAME_STATE_IN_GAME)
			{
				return false;
			}
		}
	}
	{
		FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			client->BeginWrite();
			client->WriteByte(SM_LOADING_END);
			client->WriteInt(gChannelServer.m_Config.max_ready_time);
			client->EndWrite();
		}
	}
	m_RoomState = ROOM_STATE_READY_INGMAE;
	return true;
}

void ChannelRoom::BroadCastGameTime()
{
	m_LastGameTime = m_GameTime;
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_GAME_TIME);
			client->WriteFloat(m_GameTime);
			client->EndWrite();
		}
	}
}

void ChannelRoom::ClientJoinInGame(ChannelClient * c)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (c->uid != client->uid)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_JOIN);
			ChannelClient::WriteCharacterInfo(client,c);
			client->EndWrite();
		}
	}
	c->Birth();
	//drop item
	{
		FOR_EACH_LIST(DropItemInfo, m_DropItemList, Info)
		{
			DropItemInfo *info = *iterInfo;
			c->BeginWrite();
			c->WriteByte(SM_INGAME_CREATE_DROPITEM);
			c->WriteInt(info->uid);
			c->WriteByte(info->m_Type);
			c->WriteVector3(info->m_Position);
			c->EndWrite();
		}
	}
	
}

void ChannelRoom::WriteAllClientInfo(NetworkStream * stream)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		ChannelClient::WriteCharacterInfo(stream, client);
		ChannelClient::WriteIngameState(stream, client, INGAME_STATE_CHANGE_ALL);
	}
}
void ChannelRoom::WriteAllClientState(NetworkStream * stream, byte state)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		ChannelClient::WriteIngameState(stream, client, state);
	}
}
void ChannelRoom::ClientLeave(ChannelClient * c)
{
	if (c->m_RoomID == uid)
	{
		c->m_RoomID = 0;
		if (c->m_InGameInfo)
		{
			//CleanCharacterIngameInfo(*(c->m_InGameInfo));
			memset(c->m_InGameInfo, 0, sizeof(CharacterInGameInfo));
			c->m_InGameInfo = NULL;
			c->m_OwnerRoom = NULL;
		}
		ClientIterator iter;
		for (iter = m_ClientList.begin(); iter != m_ClientList.end();)
		{
			ChannelClient *client = *iter;
			if (client->uid == c->uid)
			{
				iter = m_ClientList.erase(iter);
				
			}
			else
			{
				if (client->m_NetState == NET_STATE_CONNECTED)
				{
					client->BeginWrite();
					client->WriteByte(SM_ROOM_LEAVE);
					client->WriteInt(c->uid);
					client->EndWrite();
				}
				iter++;
			}
		}
		if (m_ClientList.size() == 0)
		{
			gChannelServer.FreeRoom(this);
		}
		//ÇÐ»»·¿Ö÷
		else if (m_RoomState == ROOM_STATE_WAIT && c->m_IsRoomHost)
		{
			ChannelClient *new_host = m_ClientList.front();
			if (new_host)
			{
				new_host->m_IsRoomHost = true;
				FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
				{
					ChannelClient *client = *iterClient;
					client->BeginWrite();
					client->WriteByte(SM_ROOM_HOST_CHANGE);
					client->WriteInt(new_host->uid);
					client->EndWrite();
				}
			}
		}
	}
}

void ChannelRoom::StartGame()
{
	m_RoomState = ROOM_STATE_PLAYING;
	m_GameTime = gChannelServer.m_Config.max_game_time;
	m_LastGameTime = m_GameTime;
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->Birth();
		}
	}
}

void ChannelRoom::LoadingGame()
{
	m_RoomState = ROOM_STATE_LOADING;
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ClientLoading(*iterClient);
	}
}

