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
	m_GameState(GAME_STATE_NONE)
{
	connection = this;
	stream = this;
}

ChannelClient::~ChannelClient()
{
}

void ChannelClient::OnConnected()
{
	m_RoomID = 0;
	m_GameState = GAME_STATE_SYNC_INFO;
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.05f,ClientUpdate, this, true);
	m_UpdateTimer.Begin();
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
	}
}
void ChannelClient::Update(float time)
{
	UdpConnection::Update(time);
}
void ChannelClient::ReadCharacterInfo()
{
	ReadString(m_CharacterInfo.Name, sizeof(m_CharacterInfo.Name));
	ReadInt(m_CharacterInfo.MaxHP);
	ReadShort(m_CharacterInfo.WeaponCount);
	m_HP = m_CharacterInfo.MaxHP;

	for (int i = 0; i < m_CharacterInfo.WeaponCount; i++)
	{
		WeaponInfo &info = m_WeaponList[i];
		byte type;
		ReadByte(type);
		info.Type = (WeaponType)type;
		ReadFloat(info.AttackRange);
		ReadFloat(info.Damage);
		ReadFloat(info.FireTime);
		ReadFloat(info.ReloadTime);
	}
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
	if (!room)room = gChannelServer.CreateNewRoom();
	BeginWrite();
	WriteByte(SM_ROOM_INFO);
	WriteByte((room == NULL ? ERROR_CREATE_ROOM : ERROR_NONE));
	if (room)
	{
		m_GameState = GAME_STATE_IN_ROOM;
		m_RoomID = room->uid;
		WriteInt(room->uid);
		WriteByte(room->m_MaxClient);
		room->m_ClientList.push_back(this);
		byte room_client_count = room->m_ClientList.size();
		WriteByte(room_client_count);
		FOR_EACH_LIST(ChannelClient,room->m_ClientList, Client)
		{
			ChannelClient *client = *iterClient;
			WriteInt(client->uid);
			WriteString(client->m_CharacterInfo.Name);
			if (client->uid != uid)
			{
				client->BeginWrite();
				client->WriteByte(SM_ROOM_ENTER);
				client->WriteInt(uid);
				client->WriteString(m_CharacterInfo.Name);
				client->EndWrite();
			}

		}
	}
	EndWrite();
	if (room)
	{
		if (room->m_State == ROOM_STATE_PLAYING)
		{
			BeginWrite();
			WriteByte(SM_GAME_LOGADING);
			EndWrite();
		}
		else if (room->IsFull())
		{
			room->StartGame();
		}
	}
}

void ChannelClient::WriteCharacterInfo(ChannelClient* c)
{
	WriteInt(c->uid);
	WriteString(c->m_CharacterInfo.Name);
	//WriteInt(c->m_CharacterInfo.MaxHP);
	//WriteShort(c->m_CharacterInfo.WeaponCount);
	//for (int i = 0; i < c->m_CharacterInfo.WeaponCount; i++)
	//{
	//	WeaponInfo &weapon = c->m_WeaponList[i];
	//	WriteByte((byte)weapon.Type);
	//	WriteShort(SORT_TO_CLIENT(i));
	//}
}
