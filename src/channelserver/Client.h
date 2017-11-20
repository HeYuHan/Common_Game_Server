#pragma once
#ifndef __CHANNELCLIENT_H__
#define __CHANNELCLIENT_H__
#include <UdpConnection.h>
#include <Timer.h>
#include "ClientInfo.h"

#define SORT_TO_CLIENT(__sort__) (short)(__sort__ + 1)
#define SORT_TO_SERVER(__sort__) (short)(__sort__ - 1)
class ChannelRoom;
typedef enum
{
	GAME_STATE_NONE,
	GAME_STATE_SYNC_INFO,
	GAME_STATE_IN_ROOM,
	GAME_STATE_LOADING_GAME,
	GAME_STATE_IN_GAME,
	GAME_STATE_IN_BALANCE,
}GameState;
typedef enum
{
	NET_STATE_NONE,
	NET_STATE_CONNECTING,
	NET_STATE_VERIFY_PROTOCOL,
	NET_STATE_CONNECTED,
	NET_STATE_DISCONNECTED
}NetState;
typedef enum
{
	PROTOCOL_NONE=0,
	PROTOCOL_KEEPALIVE=1<<1,
	PROTOCOL_DEBUGCLIENT=1<<2,
}ConnectionProtocol;

typedef enum
{
	INGAME_STATE_CHANGE_HEALTH = 1 << 1,
	INGAME_STATE_CHANGE_EXP = 1 << 2,
	INGMAE_STATE_CHANGE_KILLCOUNT = 1 << 3,
	INGAME_STATE_CHANGE_DIAMONDCOUNT= 1<<4,
	INGAME_STATE_CHANGE_ALL = INGAME_STATE_CHANGE_HEALTH | INGAME_STATE_CHANGE_EXP | INGMAE_STATE_CHANGE_KILLCOUNT | INGAME_STATE_CHANGE_DIAMONDCOUNT
}ClinetInGameState;

typedef enum
{
	SYNC_TRANSFROM_POSITION = 1 << 1,
	SYNC_TRANSFROM_ROTATION = 1 << 2,
	SYNC_TRANSFROM_VELOCITY = 1 << 3
}SyncData;

class ChannelClient:public NetworkStream,public UdpConnection
{
public:
	ChannelClient();
	~ChannelClient();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual void OnMessage();
	virtual void Update(float time);
	void OnKeepAlive();
public:
	uint uid;
	uint64_t m_ConnectionID;
	uint m_RoomID;
	ChannelRoom* m_OwnerRoom;
	GameState m_GameState;
	NetState m_NetState;
	CharacterInfo m_CharacterInfo;
	CharacterInGameInfo *m_InGameInfo;
	Vector3 m_Position;
	Vector3 m_Velocity;
	Quaternion m_Rotation;
	bool m_IsRoomHost;
	byte m_Proto;
	float m_KeepAliveTime;
	Timer m_UpdateTimer;
private:
	WeaponInfo* GetWeapon(byte sort);
	void IngameUpdate(float time);
private:
	void Init();
	void ReadCharacterInfo();
	void ReadyGameEnter();
	void ParseJoinGame();
	void ParseGameReady();
	void ParseStartGame();
	void ParseMoveData();
	void ParseShoot();
	void ParseHitCharacter();
	void ParseGetDropItem();
	void ParseUseSkill();
	void ParseKillEffect();
public:
	int GetSocore();
	void Brith();
	void Dead();
	void InGameStateChange(byte state);
	//void WriteCharacterInfo(ChannelClient* c);
	static void WriteCharacterInfo(NetworkStream* stream, ChannelClient* c);
	static void WriteIngameState(NetworkStream* stream,ChannelClient* c, byte state);

};


#endif // !__CHANNELCLIENT_H__

