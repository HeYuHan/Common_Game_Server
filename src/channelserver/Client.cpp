#include "Client.h"
#include "MessageType.h"
#include "ChannelServer.h"
#include "Room.h"
#include <common.h>
#define CHECK_ROOM() \
if(NULL == m_OwnerRoom) { \
	log_error("m_OwnerRoom is null uid:%d", uid); \
	Disconnect(); \
	return; \
}

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
	m_GameState = GAME_STATE_NONE;
	m_UpdateTimer.Stop();
	if (m_OwnerRoom)
	{
		m_OwnerRoom->ClientLeave(this);
	}
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
	case CM_INGAME_HIT_CHARACTER:
		ParseHitCharacter();
		break;
	case CM_INGAME_GET_DROP_ITEM:
		ParseGetDropItem();
		break;
	case CM_INGAME_USE_SKILL:
		ParseUseSkill();
		break;
	case CM_INGAME_KILL_SELF:
		ParseKillSelf();
		break;
	case CM_INGMAE_EXPLODE_CHARACTER:
		ParseExplodeCharacter();
		break;
	}
	OnKeepAlive();
}
void ChannelClient::Update(float time)
{
	UdpConnection::Update(time);

	switch (m_GameState)
	{
	case GAME_STATE_IN_GAME:
		IngameUpdate(time);
		break;
	default:
		break;
	}
	if ((m_Proto & PROTOCOL_KEEPALIVE) > 0 && m_NetState == NET_STATE_CONNECTED)
	{
		m_KeepAliveTime += time;
		if (m_KeepAliveTime > KEEP_ALIVE_TIME)
		{
			log_error("keep alive time out disconnect %d", uid);
			Disconnect();
		}
	}
	
}
void ChannelClient::IngameUpdate(float time)
{
	if (m_InGameInfo->m_Dead)
	{
		if (m_InGameInfo->m_BrithTime > 0)
		{
			m_InGameInfo->m_BrithTime -= time;
			if (m_InGameInfo->m_BrithTime <= 0)
			{
				Birth();
			}
		}
	}
	//skill
	for (int i = DROP_ITEM_START; i < DROP_ITEM_COUNT; i++)
	{
		SkillInfo *info = &m_InGameInfo->m_SkillList[i];
		if (info->m_Enabled)
		{
			info->m_UsingTime -= time;
			if (info->m_UsingTime <= 0)
			{
				info->m_Enabled = false;
			}
		}
	}
	//buff
	for (int i = 0; i < BUFF_TYPE_COUNT; i++)
	{
		BufferInfo *buff = &m_InGameInfo->m_BuffList[i];
		if (buff->m_State != BUFF_STATE_END)
		{
			buff->m_Duration -= time;
			switch (buff->m_Type)
			{
				case BUFF_TYPE_SHIELD:
				{
				
					if (buff->m_Duration <= 0 || buff->m_UserData[0]<=0)
					{
						buff->m_State = BUFF_STATE_END;
					}
					break;
				}
				default:
				{
					if (buff->m_Duration <= 0)
					{
						buff->m_State = BUFF_STATE_END;
					}
					break;
				}
			}
			if (buff->m_State == BUFF_STATE_END)
			{
				m_OwnerRoom->BroadCastBuffState(0, uid, buff);
			}
		}
	}
}
void ChannelClient::OnKeepAlive()
{
	m_KeepAliveTime = 0;
}
WeaponInfo * ChannelClient::GetWeapon(byte sort)
{
	sort = SORT_TO_SERVER(sort);
	if (m_InGameInfo&&sort >= 0 && sort < WeaponCount - 1)
	{
		return &m_InGameInfo->m_WeaponList[sort];

	}
	return NULL;
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
	m_OwnerRoom = NULL;
	gChannelServer.RandomBrithPos(m_Position);
	m_Velocity = Vector3(0, 0, 0);
	m_Rotation = Quaternion(0, 0, 0, 1);
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.05f, ClientUpdate, this, true);
	m_UpdateTimer.Begin();
}
void ChannelClient::ReadCharacterInfo()
{
	ReadString(m_CharacterInfo.Name, sizeof(m_CharacterInfo.Name));
	ReadInt(m_CharacterInfo.MaxHP);
	m_CharacterInfo.MaxHP = gChannelServer.m_Config.max_health;
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
		m_OwnerRoom = room;
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
	CHECK_ROOM();
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

void ChannelClient::ParseGameReady()
{
	CHECK_ROOM();
	if (m_GameState != GAME_STATE_LOADING_GAME)return;
	m_GameState = GAME_STATE_IN_GAME;
	BeginWrite();
	WriteByte(SM_INGAME_ROOM_INFO);
	WriteByte(m_OwnerRoom->m_ClientList.size());
	m_OwnerRoom->WriteAllClientInfo(this);
	EndWrite();
	if (m_OwnerRoom->m_RoomState == ROOM_STATE_PLAYING)
	{
		m_OwnerRoom->ClientJoinInGame(this);
	}
	else
	{
		m_OwnerRoom->CheckAllClientReadyInGame();
	}
	
}

void ChannelClient::ParseStartGame()
{
	if (m_IsRoomHost)
	{
		if (m_OwnerRoom&&m_OwnerRoom->m_RoomState == ROOM_STATE_WAIT)
		{
			m_OwnerRoom->LoadingGame();
		}
	}
}

void ChannelClient::ParseMoveData()
{
	CHECK_ROOM();
	char* read_start = read_position;
	byte delta = 0;
	byte flag = 0;
	ReadByte(delta);
	ReadByte(flag);

	if ((flag & SYNC_TRANSFROM_POSITION) > 0)
	{
		ReadVector3(m_Position);
	}
	if ((flag & SYNC_TRANSFROM_ROTATION) > 0)
	{
		ReadShortQuaternion(m_Rotation);
	}
	if ((flag & SYNC_TRANSFROM_VELOCITY) > 0)
	{
		ReadVector3(m_Velocity);
	}
	FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_MOVE_DATA);
			client->WriteInt(uid);
			client->WriteData(read_start, read_end - read_start);
			client->EndWrite();
		}

	}
}


void ChannelClient::ParseShoot()
{
	CHECK_ROOM();
	FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
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

void ChannelClient::ParseHitCharacter()
{
	uint hit_uid = 0;
	byte sort = 0;
	ReadUInt(hit_uid);
	ReadByte(sort);
	ChannelClient* c = gChannelServer.m_ClientPool.Get(hit_uid);
	WeaponInfo* weapon = GetWeapon(sort);
	if (c && weapon && hit_uid != uid)
	{
		if (c->m_InGameInfo->m_Dead)return;
		//打到盾上了,抵挡攻击
		if ((c->m_InGameInfo->m_BuffList[BUFF_TYPE_SHIELD].m_UserData[0]--) > 0)
		{
			c->BuffStateChange(uid, BUFF_TYPE_SHIELD);
			return;
		}
		c->m_InGameInfo->m_HP -= weapon->Damage;
		if (c->m_InGameInfo->m_HP <= 0)
		{
			c->Dead();
			m_InGameInfo->m_KillCount += 1;
			InGameStateChange(INGMAE_STATE_CHANGE_KILLCOUNT);
		}
		else
		{
			c->InGameStateChange(INGAME_STATE_CHANGE_HEALTH);
		}
	}
}

void ChannelClient::ParseExplodeCharacter()
{
	uint hit_uid = 0;
	byte sort = 0;
	short dis = 0;
	ReadUInt(hit_uid);
	ReadByte(sort);
	ReadShort(dis);
	ChannelClient* c = gChannelServer.m_ClientPool.Get(hit_uid);
	WeaponInfo* weapon = GetWeapon(sort);
	if (c && weapon && hit_uid != uid)
	{
		if (weapon->Range == 0)
		{
			log_error("%d weapon range is zero", sort);
			return;
		}
		if (c->m_InGameInfo->m_Dead)return;
		//打到盾上了,抵挡攻击
		if ((c->m_InGameInfo->m_BuffList[BUFF_TYPE_SHIELD].m_UserData[0]--) > 0)
		{
			c->BuffStateChange(uid, BUFF_TYPE_SHIELD);
			return;
		}
		int damage = weapon->Damage;
		float d_dis = weapon->Range / 3.0f;
		if (dis > d_dis)
		{
			damage = (1 - (dis - d_dis) / (weapon->Range - d_dis))*weapon->Damage;
			if (damage < 0)damage = 0;
		}
		c->m_InGameInfo->m_HP -= damage;
		if (c->m_InGameInfo->m_HP <= 0)
		{
			c->Dead();
			m_InGameInfo->m_KillCount += 1;
			InGameStateChange(INGMAE_STATE_CHANGE_KILLCOUNT);
		}
		else
		{
			c->InGameStateChange(INGAME_STATE_CHANGE_HEALTH);
		}
	}
}

void ChannelClient::ParseGetDropItem()
{
	int item_id = 0;
	ReadInt(item_id);
	DropItemInfo* info = gChannelServer.m_DropItemPool.Get(item_id);
	if (info)
	{
		//能量水晶
		if (info->m_Type == DROP_ITEM_DIAMOND)
		{
			m_InGameInfo->m_DiamondCount += 1;
			InGameStateChange(INGAME_STATE_CHANGE_DIAMONDCOUNT);
		}
		else
		{
			BeginWrite();
			WriteByte(SM_INGAME_GET_SKILL);
			WriteByte(info->m_Type);
			EndWrite();
		}
		m_OwnerRoom->RemoveDropItem(info);
	}
}

void ChannelClient::ParseUseSkill()
{
	byte type;
	ReadByte(type);
	if (type > DROP_ITEM_NONE && type < DROP_ITEM_COUNT)
	{
		SkillInfo *skill = &m_InGameInfo->m_SkillList[type];
		if (skill->m_Enabled)return;
		skill->m_Enabled = true;
		skill->m_UsingTime = skill->m_CoolDown;
		m_OwnerRoom->BroadCastSkillUse(uid,skill->m_Type);
		switch (type)
		{
		case DROP_ITEM_SHIELD:
		{
			BufferInfo *buff = &m_InGameInfo->m_BuffList[BUFF_TYPE_SHIELD];
			buff->m_State = BUFF_STATE_START;
			buff->m_Duration = skill->m_Duration;
			memcpy(buff->m_UserData, skill->m_UserData, sizeof(float) * 4);
			m_OwnerRoom->BroadCastBuffState(uid,uid,buff);
			break;
		}
		case DROP_ITEM_PLASMA:
		{
			FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
			{
				ChannelClient *client = *iterClient;
				if (uid != client->uid && client->m_GameState == GAME_STATE_IN_GAME)
				{
					float dis = Length(client->m_Position - m_Position);
					if (dis <= skill->m_UserData[0])
					{
						BufferInfo *buff = &client->m_InGameInfo->m_BuffList[BUFF_TYPE_PLASMA];
						buff->m_State = BUFF_STATE_START;
						buff->m_Duration = skill->m_Duration;
						memcpy(buff->m_UserData, skill->m_UserData, sizeof(float) * 4);
						m_OwnerRoom->BroadCastBuffState(uid, client->uid, buff);
					}
				}
			}

			break;
		}
		default:
			break;
		}
	}
}

void ChannelClient::ParseKillSelf()
{
	if (!m_InGameInfo->m_Dead)
	{
		Dead();
	}
}

int ChannelClient::GetSocore()
{
	return m_InGameInfo->m_KillCount + m_InGameInfo->m_DiamondCount;
}

void ChannelClient::Birth()
{
	if (m_InGameInfo->m_PlayTime == 0)m_InGameInfo->m_PlayTime = m_OwnerRoom->m_GameTime;
	m_InGameInfo->m_HP = m_CharacterInfo.MaxHP;
	m_InGameInfo->m_Dead = false;
	gChannelServer.RandomBrithPos(m_Position);
	m_Rotation = Quaternion(0, 0, 0, 1);
	memcpy(&m_InGameInfo->m_SkillList, &gGameConfig.SkillInfos, sizeof(m_InGameInfo->m_SkillList));
	CHECK_ROOM();
	FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_BIRTH);
			client->WriteInt(uid);
			client->WriteInt(m_InGameInfo->m_HP);
			client->WriteVector3(m_Position);
			client->WriteShortQuaternion(m_Rotation);
			client->EndWrite();
		}
	}

}

void ChannelClient::Dead()
{
	CHECK_ROOM();
	m_InGameInfo->m_HP = 0;
	m_InGameInfo->m_Dead = true;
	m_InGameInfo->m_BrithTime = gChannelServer.m_Config.rebirth_time;
	FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_CHARACTER_DEAD);
			client->WriteInt(uid);
			client->WriteFloat(m_InGameInfo->m_BrithTime);
			client->EndWrite();
		}
	}
}

void ChannelClient::InGameStateChange(byte state)
{
	CHECK_ROOM();
	FOR_EACH_LIST(ChannelClient, m_OwnerRoom->m_ClientList, Client)
	{
		ChannelClient *client = *iterClient;
		if (client->m_GameState == GAME_STATE_IN_GAME)
		{
			client->BeginWrite();
			client->WriteByte(SM_INGAME_STATE_CHANGE);
			client->WriteInt(uid);
			WriteIngameState(client, this, state);
			client->EndWrite();
		}
	}
}

void ChannelClient::BuffStateChange(uint from_uid, int type)
{
	CHECK_ROOM();
	m_InGameInfo->m_BuffList[type].m_State = BUFF_STATE_UPDATE;
	m_OwnerRoom->BroadCastBuffState(from_uid, uid, &m_InGameInfo->m_BuffList[type]);
}

void ChannelClient::UpdateLevel()
{
	int k = 4;
	if (m_InGameInfo->m_Level == 0)
		m_InGameInfo->m_Level = 1;
	if (m_InGameInfo->m_Level == 1 && m_InGameInfo->m_DiamondCount >= k)
	{

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
	stream->WriteVector3(c->m_Position);
	stream->WriteShortQuaternion(c->m_Rotation);
	stream->WriteBool(c->m_InGameInfo->m_Dead);
	stream->WriteShort(WeaponCount - 1);
	for (int i = 0; i < WeaponCount-1; i++)
	{
		WeaponInfo &weapon = c->m_InGameInfo->m_WeaponList[i];
		stream->WriteFloat(weapon.AttackTime);
		stream->WriteFloat(weapon.ReloadTime);
		stream->WriteFloat(weapon.Damage);
		stream->WriteInt(weapon.Ammunition);
		stream->WriteBool(weapon.Tracker);
		stream->WriteFloat(weapon.Range);
		stream->WriteFloat(weapon.Speed);
	}
	stream->WriteShort(DROP_ITEM_COUNT);
	for (int i = DROP_ITEM_START; i < DROP_ITEM_COUNT; i++)
	{
		SkillInfo &skill = c->m_InGameInfo->m_SkillList[i];
		stream->WriteFloat(skill.m_CoolDown);
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
	if ((state & INGMAE_STATE_CHANGE_KILLCOUNT) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_KillCount);
	}
	if ((state & INGAME_STATE_CHANGE_DIAMONDCOUNT) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_DiamondCount);
	}
	if ((state & INGAME_STATE_CHANGE_LEVELUP) > 0)
	{
		stream->WriteInt(c->m_InGameInfo->m_Level);
	}
}
