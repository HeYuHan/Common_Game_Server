#include "Client.h"
#include "MessageType.h"
#include "Room.h"
ChannelClient::ChannelClient():
	uid(0),
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
	m_GameState = GAME_STATE_SYNC_INFO;
	BeginWrite();
	WriteInt(uid);
	EndWrite();
}

void ChannelClient::OnDisconnected()
{
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
	case CM_SHOOT:
		//ParseShoot();
		//BroadCastMessage((byte)SM_SHOOT,false);
		break;
	case CM_SYNC_TRANSFROM:
		//ParseTransfromData();
		//BroadCastMessage(ServerMessage.SM_SYNC_TRANSFROM);
		break;
	case CM_REQUEST_JOIN_GAME:
		ParseJoinGame();
		break;
	case CM_HIT_CHARACTER:
		//ParseHitCharacter();
		break;
	case CM_USE_SKILL_WEAPON:
		//ParseUseSkillWeapon();
		break;
	case CM_JOYSTICK:
		//BroadCastMessage((byte)SM_JOYSTICK, false);
		break;
	case CM_RESURRECTION:
		//ParseResurrection();
		break;
	case CM_SUPPLY_CHANGE:
		//ParseGetSupplyObj();
		break;
	case CM_ADD_NEW_WEAPON:
		//ParseNewWeapon();
		break;
	default:
		//if (connection)connection->DisConnect();
	}
}
void ChannelClient::ReadCharacterInfo()
{
	ReadString(m_CharacterInfo.Name, sizeof(m_CharacterInfo.Name));
	ReadInt(m_CharacterInfo.MaxHP);
	ReadInt(m_CharacterInfo.KillNumber);
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

	BeginWrite();
	WriteByte((byte)SM_WEAPON_SORT);
	WriteInt(m_CharacterInfo.WeaponCount);
	for (int i = 0; i < m_CharacterInfo.WeaponCount; i++)
	{
		WriteByte((byte)m_WeaponList[i].Type);
		WriteShort((short)i);
	}
	EndWrite();

	ReadyGameEnter();

}

void ChannelClient::ReadyGameEnter()
{
	m_GameState = GAME_STATE_READY_INGAME;
	BeginWrite();
	WriteByte((byte)SM_CAN_ENTER_GAME);
	EndWrite();
}

void ChannelClient::ParseJoinGame()
{
}
