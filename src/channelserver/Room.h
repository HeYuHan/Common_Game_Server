#pragma once
#ifndef __CHANNEL_ROOM_H__
#include <vector>
#include<Timer.h>
#include "ClientInfo.h"
#include "pch.h"
class ChannelClient;
class NetworkStream;
typedef enum
{

}RoomType;
typedef enum
{
	ROOM_STATE_IDLE = 1 << 1,
	ROOM_STATE_WAIT = 1 << 2,
	ROOM_STATE_LOADING = 1 << 3,
	ROOM_STATE_PLAYING = 1 << 4,
	ROOM_STATE_BALANCE = 1 << 5,
	ROOM_STATE_WAIT_OR_PLAYING = ROOM_STATE_WAIT | ROOM_STATE_PLAYING,

}RoomState;
struct DropItemTimer
{
	float m_RefreshTime;
	float m_StartTime;
};
class ChannelRoom
{
public:
	ChannelRoom();
	~ChannelRoom();
	void Init();
	void Clean();
	void Update(float time);
	void PlayingUpdate(float time);
	bool IsFull();
	void ClientEnter(ChannelClient* c);
	void ClientLeave(ChannelClient* c);
	void LoadingGame();
	void StartGame();
	void WriteAllClientInfo(NetworkStream* stream);
	void WriteAllClientState(NetworkStream* stream,byte state);
	void ClientLoading(ChannelClient* c);
	void ClientJoinInGame(ChannelClient* c);
	
	void BroadCastGameTime();
	void BroadCastCreateDropItem(DropItemInfo* info);
	void BroadCastRemoveDropItem(DropItemInfo* info);
	void BroadCastSkillUse(uint from_uid, byte skill_type);
	void BraodCastBuffState(uint from_uid,uint to_uid,BufferInfo* buff);
	void UpdateDropItem(float time);
	void GameToBalance();
	void LeaveBalance();
	DropItemInfo* CreateDropItem();
	void RemoveDropItem(DropItemInfo* info);
public:
	void WriteBlanceData(NetworkStream * stream);
public:
	uint uid;
	RoomState m_RoomState;
	std::vector<ChannelClient*> m_ClientList;
	byte m_MaxClient;
	float m_LoadingTime;
	float m_GameTime;
	float m_LastGameTime;
	float m_BalanceWaitTime;
	Timer m_UpdateTimer;
	DropItemTimer m_DropItemTimers[DROP_ITEM_COUNT];
	char m_DropItemIndex[MAX_DROP_POINT_COUNT];
	std::vector<DropItemInfo*> m_DropItemList;
	CharacterInGameInfo m_CharacterInfoArray[ROOM_MAX_CLIENT];
};


#define __CHANNEL_ROOM_H__
#endif