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
	m_LoadingTime = 0;
	m_RoomState = ROOM_STATE_WAIT;
	m_GameTime = 0;
	m_LastGameTime = 0;
	m_ClientList.clear();
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.02f, RoomUpdate, this,true);
	m_UpdateTimer.Begin();
}

void ChannelRoom::Clean()
{
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
		m_GameTime += time;
		if (m_GameTime - m_LastGameTime >= 1)
		{
			
			BroadCastGameTime();
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
			c->m_InGameInfo = NULL;
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
				client->BeginWrite();
				client->WriteByte(SM_ROOM_LEAVE);
				client->WriteInt(c->uid);
				client->EndWrite();
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


