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
	void Init();
	void ReadCharacterInfo();
	void ReadyGameEnter();
	void ParseJoinGame();
	void ParseGameReady();
	void ParseStartGame();
	void ParseMoveData();
	void ParseShoot();
public:
	void Brith();
public:
	void WriteCharacterInfo(ChannelClient* c);

};


#endif // !__CHANNELCLIENT_H__

