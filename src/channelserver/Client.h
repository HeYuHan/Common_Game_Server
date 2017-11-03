#pragma once
#ifndef __CHANNELCLIENT_H__
#define __CHANNELCLIENT_H__
#include <UdpConnection.h>
#include <Timer.h>
#include <common.h>
#define SORT_TO_CLIENT(__sort__) (short)(__sort__ + 1)
#define SORT_TO_SERVER(__sort__) (short)(__sort__ - 1)
typedef enum 
{
	MachineGun = 1,
	PlasmaGun,
	JetFlame,
	LightShield,
	FireWhirlwind,
	Missile,
	OrdinaryGun,
	WeaponCount
}WeaponType;
typedef enum
{
	GAME_STATE_NONE,
	GAME_STATE_SYNC_INFO,
	GAME_STATE_READY_INGAME
}GameState;
class WeaponInfo
{
public:
	WeaponType Type;
	float AttackRange;
	float Damage;
	float FireTime;
	float ReloadTime;
};
class CharacterInfo
{
public:
	char Name[64];
	int MaxHP;
	short WeaponCount;
};
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
	uint m_RoomID;
	GameState m_GameState;
	CharacterInfo m_CharacterInfo;
	int m_HP;
	unsigned int m_RoomID;
	WeaponInfo m_WeaponList[WeaponCount-1];
	Timer m_UpdateTimer;

private:
	void ReadCharacterInfo();
	void ReadyGameEnter();
	void ParseJoinGame();
	void WriteCharacterInfo(ChannelClient* c);

};


#endif // !__CHANNELCLIENT_H__

