#pragma once
#ifndef __CHANNEL_ROOM_H__
#include <vector>
#include<Timer.h>
#include<common.h>
class ChannelClient;
typedef enum
{

}RoomType;
typedef enum
{
	ROOM_STATE_IDLE = 1 << 1,
	ROOM_STATE_WAIT = 1 << 2,
	ROOM_STATE_PLAYING = 1 << 3,
	ROOM_STATE_WAIT_OR_PLAYING = ROOM_STATE_WAIT | ROOM_STATE_PLAYING,

}RoomState;
class ChannelRoom
{
public:
	ChannelRoom();
	~ChannelRoom();
	void Init();
	void Clean();
	void Update(float time);
	bool IsFull();
	void ClientLeave(ChannelClient* c);
	void StartGame();
	void WriteAllClientInfo(ChannelClient* stream);
	void ClientLoading(ChannelClient* c);
	void ClientJoinInGame(ChannelClient* c);
public:
	uint uid;
	RoomState m_RoomState;
	std::vector<ChannelClient*> m_ClientList;
	byte m_MaxClient;
	Timer m_UpdateTimer;
};


#define __CHANNEL_ROOM_H__
#endif