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
static void CleanCharacterIngameInfo(CharacterInGameInfo &info)
{
	info.uid = 0;
	info.m_HP = 0;
	info.WeaponCount = 0;
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
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		CleanCharacterIngameInfo(m_CharacterInfoArray[i]);
	}
	m_RoomState = ROOM_STATE_WAIT;
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
			c->m_RoomID = uid;
			c->m_InGameInfo = &m_CharacterInfoArray[i];
			c->m_InGameInfo->uid = c->uid;
			break;
		}
	}
}

void ChannelRoom::ClientLoading(ChannelClient * c)
{
	m_RoomState = ROOM_STATE_PLAYING;
	c->m_GameState = GAME_STATE_LOADING_GAME;
	c->BeginWrite();
	c->WriteByte(SM_GAME_LOGADING);
	c->EndWrite();
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
			client->WriteCharacterInfo(c);
			client->EndWrite();
		}
	}
}

void ChannelRoom::WriteAllClientInfo(ChannelClient * stream)
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		stream->WriteCharacterInfo(client);
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
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ClientLoading(*iterClient);
	}
}


