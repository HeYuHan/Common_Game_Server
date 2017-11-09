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


class ChannelClient:public NetworkStream,public UdpConnection
{
public:
	ChannelClient();
	~ChannelClient();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual void OnMessage();
	virtual void Update(float time);
public:
	uint uid;
	uint64_t m_ConnectionID;
	uint m_RoomID;
	GameState m_GameState;
	CharacterInfo m_CharacterInfo;
	CharacterInGameInfo *m_InGameInfo;
	bool m_IsRoomHost;
	Timer m_UpdateTimer;

private:
	void Init();
	void ReadCharacterInfo();
	void ReadyGameEnter();
	void ParseJoinGame();
	void ParseGameReady();
	void ParseStartGame();
	void ParseMoveData();
public:
	void WriteCharacterInfo(ChannelClient* c);

};


#endif // !__CHANNELCLIENT_H__

