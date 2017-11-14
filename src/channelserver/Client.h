#pragma once
#ifndef __CHANNELCLIENT_H__
#define __CHANNELCLIENT_H__
#include <UdpConnection.h>
#include <Timer.h>
#include "ClientInfo.h"
#define SORT_TO_CLIENT(__sort__) (short)(__sort__ + 1)
#define SORT_TO_SERVER(__sort__) (short)(__sort__ - 1)

typedef enum
{
	GAME_STATE_NONE,
	GAME_STATE_SYNC_INFO,
	GAME_STATE_IN_ROOM,
	GAME_STATE_LOADING_GAME,
	GAME_STATE_IN_GAME,
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
	INGMAE_STATE_CHANGE_SCORE = 1 << 3,
	INGMAE_STATE_CHANGE_KILLCOUNT = 1 << 4,
	INGAME_STATE_CHANGE_ALL = INGAME_STATE_CHANGE_HEALTH | INGAME_STATE_CHANGE_EXP | INGMAE_STATE_CHANGE_SCORE | INGMAE_STATE_CHANGE_KILLCOUNT
}ClinetInGameState;

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
	GameState m_GameState;
	NetState m_NetState;
	CharacterInfo m_CharacterInfo;
	CharacterInGameInfo *m_InGameInfo;
	bool m_IsRoomHost;
	byte m_Proto;
	float m_KeepAliveTime;
	Timer m_UpdateTimer;
private:
	WeaponInfo* GetWeapon(byte sort);
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
public:
	void Brith();
	void InGameStateChange(byte state);
	//void WriteCharacterInfo(ChannelClient* c);
	static void WriteCharacterInfo(NetworkStream* stream, ChannelClient* c);
	static void WriteIngameState(NetworkStream* stream,ChannelClient* c, byte state);

};


#endif // !__CHANNELCLIENT_H__

