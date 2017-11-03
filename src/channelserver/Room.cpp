#include "Room.h"
#include "ChannelServer.h"
#include <common.h>
#include "MessageType.h"
#include "Client.h"
static void RoomUpdate(float time, void* arg)
{
	ChannelRoom *room = (ChannelRoom*)arg;
	room->Update(time);
}
ChannelRoom::ChannelRoom():
	uid(0),
	m_State(ROOM_STATE_IDLE),
	m_MaxClient(10)
{
}

ChannelRoom::~ChannelRoom()
{
}

void ChannelRoom::Init()
{
	m_State = ROOM_STATE_WAIT;
	m_ClientList.clear();
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.02f, RoomUpdate, this,true);
	m_UpdateTimer.Begin();
}

void ChannelRoom::Clean()
{
	m_UpdateTimer.Stop();
}

void ChannelRoom::Update(float time)
{
}

bool ChannelRoom::IsFull()
{
	return m_MaxClient == m_ClientList.size();
}

void ChannelRoom::ClientLeave(ChannelClient * c)
{
	if (c->m_RoomID == uid)
	{
		FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			if (client->uid == c->uid)
			{
				iterClient = m_ClientList.erase(iterClient);
				if (iterClient == m_ClientList.end())return;
			}
			else
			{
				client->BeginWrite();
				client->WriteByte(SM_ROOM_LEAVE);
				client->WriteInt(c->uid);
				client->EndWrite();
			}
		}

	}
}

void ChannelRoom::StartGame()
{
	FOR_EACH_LIST(ChannelClient, m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		client->BeginWrite();
		client->WriteByte(SM_GAME_LOGADING);
		client->EndWrite();
	}
}


