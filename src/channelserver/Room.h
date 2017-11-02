#pragma once
#ifndef __CHANNEL_ROOM_H__
typedef enum
{

}RoomType;
typedef enum
{
	ROOM_STATE_IDLE = 1 << 1,
	ROOM_STATE_WAIT = 1 << 2,
	ROOM_STATE_PLAYING = 1 << 3

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
public:
	unsigned int uid;
	RoomState m_State;
	int m_ClientCount;
};


#define __CHANNEL_ROOM_H__
#endif