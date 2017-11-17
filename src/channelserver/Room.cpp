#include "Room.h"
#include "ChannelServer.h"
#include <common.h>
#include "MessageType.h"
#include "Client.h"
#include <log.h>
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
	m_LoadingTime = 0;
	m_RoomState = ROOM_STATE_WAIT;
	m_GameTime = 0;
	m_LastGameTime = 0;
	m_ClientList.clear();
	m_DropItemList.clear();
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.02f, RoomUpdate, this,true);
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
		if (m_LoadingTime >= LOADING_WAIT_TIME)
		{
			
			StartGame();
		}
	}
	if (m_RoomState == ROOM_STATE_PLAYING)
	{
		PlayingUpdate(time);
	}
	if (m_RoomState == ROOM_STATE_BALANCE)
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
	for (int i = 0; i < DROP_ITEM_COUNT-1; i++)
	{
		DropItemRefreshInfo &refresh = gChannelServer.gDropRefreshItems[i];
		DropItemTimer &timer = m_DropItemTimers[i];
		if (timer.m_StartTime < refresh.m_StartTime)timer.m_StartTime += time;
		if (timer.m_StartTime >= refresh.m_StartTime)
		{
			if (timer.m_RefreshTime < refresh.m_RefreshTime)timer.m_RefreshTime += time;
			if (timer.m_RefreshTime >= refresh.m_RefreshTime)
			{
				timer.m_RefreshTime = 0;
				for (int j = 0; j < refresh.m_RefreshCount; j++)
				{
					DropItemInfo* info = CreateDropItem();
					if (info)
					{
						info->m_Type = refresh.m_Type;
						info->m_Duration = refresh.m_Duration;
						//gChannelServer.GetDropItemInfo(*info, refresh.m_Type);
						BroadCastCreateDropItem(info);
					}
				}
			}
		}
	}
	UpdateDropItem(time);

}

bool ChannelRoom::IsFull()
{
	return m_MaxClient == m_ClientList.size();
}

void ChannelRoom::ClientEnter(ChannelClient * c)
{
	m_ClientList.push_back(c);
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (m_CharacterInfoArray[i].uid == 0)
		{
			c->m_InGameInfo = &m_CharacterInfoArray[i];
			c->m_RoomID = uid;
			c->m_InGameInfo->uid = c->uid;
			c->m_InGameInfo->m_WeaponCount = WeaponCount - 1;
			for (int j = WeaponType::MachineGun; j < WeaponType::WeaponCount; j++)
			{
				gChannelServer.GetWeaponInfo(c->m_InGameInfo->m_WeaponList[j - 1], (WeaponType)j);
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
	m_BalanceWaitTime = MAX_BLANCE_TIME;
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

void ChannelRoom::BroadCastSkillChange(uint from_uid,SkillInfo * skill)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_SKILL_CHANGE);
			client->WriteInt(from_uid);
			client->WriteByte(skill->m_Type);
			client->WriteBool(skill->m_Enabled);
			client->EndWrite();
		}
	}
}

void ChannelRoom::BroadCastGetSkill(uint from_uid, byte skill_type)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_GET_SKILL);
			client->WriteInt(from_uid);
			client->WriteByte(skill_type);
			client->EndWrite();
		}
	}
}

void ChannelRoom::BroadCastSkillEffect(uint from_uid, uint to_uid, byte skill_type)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(CM_INGAME_SKILL_EFFECT);
			client->WriteInt(from_uid);
			client->WriteInt(to_uid);
			client->WriteByte(skill_type);
			client->EndWrite();
		}
	}
}

void ChannelRoom::UpdateDropItem(float time)
{
	for (std::vector<DropItemInfo*>::iterator it = m_DropItemList.begin(); it != m_DropItemList.end();)
	{
		DropItemInfo *info = *it;
		info->m_Duration -= time;
		if (info->m_Duration <= 0)
		{
			BroadCastRemoveDropItem(info);
			it = m_DropItemList.erase(it);
			gChannelServer.m_DropItemPool.Free(info->uid);
		}
		else
		{
			it++;
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
	if (info)m_DropItemList.push_back(info);
	return info;
}

void ChannelRoom::RemoveDropItem(DropItemInfo * info)
{
	BroadCastRemoveDropItem(info);
	std::vector<DropItemInfo*>::iterator it = std::find(m_DropItemList.begin(), m_DropItemList.end(), info);
	if (it != m_DropItemList.end())
	{
		m_DropItemList.erase(it);
	}
	gChannelServer.m_DropItemPool.Free(info->uid);
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
				log_debug("remove client in room uid:%d", c->uid);
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
	}
}

void ChannelRoom::StartGame()
{
	m_RoomState = ROOM_STATE_PLAYING;
	m_GameTime = MAX_GAME_TIME;
	m_LastGameTime = MAX_GAME_TIME;
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->Brith();
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

