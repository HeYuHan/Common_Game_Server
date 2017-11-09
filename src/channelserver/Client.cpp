#include "Client.h"
#include "MessageType.h"
#include "ChannelServer.h"
#include "Room.h"
#include <common.h>

static void ClientUpdate(float time, void* arg)
{
	((ChannelClient*)arg)->Update(time);
}
ChannelClient::ChannelClient():
	uid(0),
	m_ConnectionID(0),
	m_RoomID(0),
	m_GameState(GAME_STATE_NONE),
	m_InGameInfo(NULL),
	m_IsRoomHost(false)
{
	connection = this;
	stream = this;
}

ChannelClient::~ChannelClient()
{
}

void ChannelClient::OnConnected()
{
	Init();
	BeginWrite();
	WriteInt(uid);
	EndWrite();
}

void ChannelClient::OnDisconnected()
{
	m_UpdateTimer.Stop();
	gChannelServer.RemoveClient(this);
}


void ChannelClient::OnMessage()
{
	byte type;
	ReadByte(type);
	switch (type)
	{
	case CM_PLAYER_INFO:
		ReadCharacterInfo();
		break;
	case CM_REQUEST_JOIN_GAME:
		ParseJoinGame();
		break;
	case CM_ROOM_START_GAME:
		ParseStartGame();
		break;
	case CM_READY_IN_GAME:
		ParseGameReady();
		break;
	case CM_INGAME_MOVE_DATA:
		ParseMoveData();
		break;
	}
}
void ChannelClient::Update(float time)
{
	UdpConnection::Update(time);
}
void ChannelClient::Init()
{
	m_RoomID = 0;
	m_GameState = GAME_STATE_SYNC_INFO;
	m_IsRoomHost = false;
	m_InGameInfo = NULL;
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.05f, ClientUpdate, this, true);
	m_UpdateTimer.Begin();
}
void ChannelClient::ReadCharacterInfo()
{
	ReadString(m_CharacterInfo.Name, sizeof(m_CharacterInfo.Name));
	ReadInt(m_CharacterInfo.MaxHP);
	ReadyGameEnter();

}

void ChannelClient::ReadyGameEnter()
{
	BeginWrite();
	WriteByte(SM_CAN_ENTER_GAME);
	EndWrite();
}

void ChannelClient::ParseJoinGame()
{
	ChannelRoom* room = gChannelServer.GetRoom(ROOM_STATE_WAIT_OR_PLAYING);
	if (!room) {
		room = gChannelServer.CreateNewRoom();
		m_IsRoomHost = room != NULL;
	}

	BeginWrite();
	WriteByte(SM_ROOM_INFO);
	WriteByte((room == NULL ? ERROR_CREATE_ROOM : ERROR_NONE));
	if (room)
	{
		m_GameState = GAME_STATE_IN_ROOM;
		room->ClientEnter(this);
		WriteInt(m_RoomID);
		WriteByte(room->m_MaxClient);
		byte room_client_count = room->m_ClientList.size();
		WriteByte(room_client_count);
		FOR_EACH_LIST(ChannelClient,room->m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			WriteInt(client->uid);
			WriteString(client->m_CharacterInfo.Name);
			WriteBool(client->m_IsRoomHost);
			if (client->uid != uid)
			{
				client->BeginWrite();
				client->WriteByte(SM_ROOM_ENTER);
				client->WriteInt(uid);
				client->WriteString(m_CharacterInfo.Name);
				client->WriteBool(m_IsRoomHost);
				client->EndWrite();
			}

		}
	}
	EndWrite();
	if (room)
	{
		if (room->m_RoomState == ROOM_STATE_PLAYING)
		{
			room->ClientLoading(this);
			room->ClientJoinInGame(this);
		}
		else if (room->IsFull())
		{
			room->StartGame();
		}
	}
}

void ChannelClient::ParseGameReady()
{
	m_GameState = GAME_STATE_IN_GAME;
	ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
	if (room)
	{
		FOR_EACH_LIST(ChannelClient, room->m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			client->BeginWrite();
			client->WriteByte(SM_INGAME_ROOM_INFO);
			client->WriteByte(room->m_ClientList.size());
			room->WriteAllClientInfo(client);
			client->EndWrite();
		}
	}
	else
	{
		log_error("get room error uid:%d", uid);
		DisConnect();
	}
	
}

void ChannelClient::ParseStartGame()
{
	if (m_IsRoomHost)
	{
		ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
		if (room&&room->m_RoomState == ROOM_STATE_WAIT)
		{
			room->StartGame();
		}
	}
}

void ChannelClient::ParseMoveData()
{
	ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
	if (room)
	{
		FOR_EACH_LIST(ChannelClient, room->m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			if (client->m_GameState == GAME_STATE_IN_GAME)
			{
				client->BeginWrite();
				client->WriteByte(SM_INGAME_MOVE_DATA);
				client->WriteInt(uid);
				client->WriteData(read_position, read_end - read_position);
				client->EndWrite();
			}

		}
	}
	else
	{
		log_error("get room error uid:%d", uid);
		DisConnect();
	}
}

void ChannelClient::WriteCharacterInfo(ChannelClient* c)
{
	WriteInt(c->uid);
	WriteString(c->m_CharacterInfo.Name);
	WriteInt(c->m_CharacterInfo.MaxHP);
	WriteShort(c->m_InGameInfo->WeaponCount);
	for (int i = 0; i < c->m_InGameInfo->WeaponCount; i++)
	{
		WeaponInfo &weapon = c->m_InGameInfo->m_WeaponList[i];
		WriteByte((byte)weapon.Type);
		WriteShort(SORT_TO_CLIENT(i));
	}
}
