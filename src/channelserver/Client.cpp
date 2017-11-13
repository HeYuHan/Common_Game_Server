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
	m_NetState(NET_STATE_NONE),
	m_InGameInfo(NULL),
	m_IsRoomHost(false),
	m_Proto(PROTOCOL_NONE),
	m_KeepAliveTime(0)
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
	m_NetState = NET_STATE_DISCONNECTED;
	m_UpdateTimer.Stop();
	gChannelServer.RemoveClient(this);
}


void ChannelClient::OnMessage()
{
	if (m_NetState == NET_STATE_VERIFY_PROTOCOL)
	{
		m_NetState = NET_STATE_CONNECTED;
		ReadByte(m_Proto);
		return;
	}
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
	case CM_INGAME_SHOOT:
		ParseShoot();
		break;
	}
	OnKeepAlive();
}
void ChannelClient::Update(float time)
{
	UdpConnection::Update(time);
	if ((m_Proto & PROTOCOL_KEEPALIVE) > 0)
	{
		m_KeepAliveTime += time;
		if (m_KeepAliveTime > KEEP_ALIVE_TIME)
		{
			Disconnect();
		}
	}
}
void ChannelClient::OnKeepAlive()
{
	m_KeepAliveTime = 0;
}
void ChannelClient::Init()
{
	m_RoomID = 0;
	m_GameState = GAME_STATE_SYNC_INFO;
	m_NetState = NET_STATE_VERIFY_PROTOCOL;
	m_Proto = PROTOCOL_NONE;
	m_KeepAliveTime = 0;
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
			//room->ClientJoinInGame(this);
		}
		else if (room->IsFull())
		{
			room->LoadingGame();
		}
	}
}

void ChannelClient::ParseGameReady()
{
	m_GameState = GAME_STATE_IN_GAME;
	ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
	if (room)
	{
		
		BeginWrite();
		WriteByte(SM_INGAME_ROOM_INFO);
		WriteByte(room->m_ClientList.size());
		room->WriteAllClientInfo(this);
		EndWrite();
		if (room->m_RoomState == ROOM_STATE_PLAYING)
		{
			room->ClientJoinInGame(this);
			Brith();
		}
	}
	else
	{
		log_error("get room error uid:%d", uid);
		Disconnect();
	}
	
}

void ChannelClient::ParseStartGame()
{
	if (m_IsRoomHost)
	{
		ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
		if (room&&room->m_RoomState == ROOM_STATE_WAIT)
		{
			room->LoadingGame();
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
		Disconnect();
	}
}

void ChannelClient::ParseShoot()
{
	ChannelRoom* room = gChannelServer.m_RoomPool.Get(m_RoomID);
	if (room)
	{
		FOR_EACH_LIST(ChannelClient, room->m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			if (uid != client->uid)
			{
				client->BeginWrite();
				client->WriteByte(SM_INGAME_SHOOT);
				client->WriteInt(uid);
				client->WriteData(read_position, read_end - read_position);
				client->EndWrite();
			}
		}
	}
	
}

void ChannelClient::Brith()
{
	BeginWrite();
	WriteByte(SM_INGAME_BRITH);
	WriteInt(uid);
	EndWrite();
}

void ChannelClient::InGameStateChange(byte state)
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
				client->WriteByte(SM_INGAME_STATE_CHANGE);
				client->WriteInt(uid);
				WriteIngameState(client, this, INGAME_STATE_CHANGE_ALL);
				client->EndWrite();
			}
		}
	}
}

//void ChannelClient::WriteCharacterInfo(ChannelClient* c)
//{
//	WriteInt(c->uid);
//	WriteString(c->m_CharacterInfo.Name);
//	WriteInt(c->m_CharacterInfo.MaxHP);
//	WriteShort(c->m_InGameInfo->WeaponCount);
//	for (int i = 0; i < c->m_InGameInfo->WeaponCount; i++)
//	{
//		WeaponInfo &weapon = c->m_InGameInfo->m_WeaponList[i];
//		WriteByte((byte)weapon.Type);
//		WriteByte(SORT_TO_CLIENT(i));
//	}
//}

void ChannelClient::WriteCharacterInfo(NetworkStream * stream, ChannelClient * c)
{
	stream->WriteInt(c->uid);
	stream->WriteString(c->m_CharacterInfo.Name);
	stream->WriteInt(c->m_CharacterInfo.MaxHP);
	stream->WriteShort(c->m_InGameInfo->m_WeaponCount);
	for (int i = 0; i < c->m_InGameInfo->m_WeaponCount; i++)
	{
		WeaponInfo &weapon = c->m_InGameInfo->m_WeaponList[i];
		stream->WriteByte((byte)weapon.Type);
		stream->WriteByte(SORT_TO_CLIENT(i));
	}
}

void ChannelClient::WriteIngameState(NetworkStream* stream,ChannelClient * c, byte state)
{
	stream->WriteByte(state);
	if ((state & INGAME_STATE_CHANGE_HEALTH) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_HP);
	}
	if ((state & INGAME_STATE_CHANGE_EXP) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_Experience);
	}
	if ((state & INGMAE_STATE_CHANGE_SCORE) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_Score);
	}
	if ((state & INGMAE_STATE_CHANGE_KILLCOUNT) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_KillCount);
	}
}
