#pragma once
#ifndef __CLIENTINFO_H__
#define __CLIENTINFO_H__
#include <common.h>
#include <Vector3.h>
#include "pch.h"

typedef enum
{
	WeaponNone=0,
	MachineGun,
	Missile,
	JetFlame,
	WeaponCount
}WeaponType;
typedef enum
{
	DROP_ITEM_NONE = 0,
	DROP_ITEM_DIAMOND=1,
	DROP_ITEM_SHIELD,
	DROP_ITEM_PLASMA,
	DROP_ITEM_COUNT,
	DROP_ITEM_START = 1,
}DropItemType,SkillType;
struct DropItemRefreshInfo
{
	DropItemType m_Type;
	char m_Desc[64];
	int m_RefreshCount;
	float m_StartTime;


};
struct DropItemInfo
{
	uint uid;
	DropItemType m_Type;
	float m_Duration;
	int m_PositionIndex;
	Vector3 m_Position;

};
struct SkillInfo
{
	SkillType m_Type;
	float m_CoolDown;
	float m_Duration;
	float m_UserData[4];
	bool m_Enabled;
	float m_UsingTime;
};
typedef enum
{
	BUFF_TYPE_NONE = 0,
	BUFF_TYPE_SHIELD,
	BUFF_TYPE_PLASMA,
	BUFF_TYPE_COUNT,
	BUFF_TYPE_START=1,
}BuffType;
typedef enum
{
	BUFF_STATE_START = 1 << 1,
	BUFF_STATE_UPDATE = 1 << 2,
	BUFF_STATE_END = 1 << 3
}BuffState;
struct BufferInfo
{
	BuffType m_Type;
	float m_Duration;
	byte m_State;
	float m_UserData[4];
};
//struct DiamondInfo:public DropItemInfo
//{
//	int m_Score;
//};
//struct ShieldInfo :public DropItemInfo
//{
//	float m_Duration;
//};
//typedef DropItemInfo PlasmaInfo;
struct WeaponInfo
{
	WeaponType Type;
	int Damage;
	float AttackTime;
	int Ammunition;
	float ReloadTime;
	bool Tracker;
	float Range;
	float Speed;
};
class CharacterInfo
{
public:
	char Name[64];
	int MaxHP;

};
struct CharacterInGameInfo
{
	WeaponInfo m_WeaponList[WeaponCount - 1];
	SkillInfo m_SkillList[DROP_ITEM_COUNT];
	BufferInfo m_BuffList[BUFF_TYPE_COUNT];
	int m_Experience;
	int m_KillCount;
	int m_HP;
	int m_DiamondCount;
	int m_Level;
	bool m_Dead;
	float m_BrithTime;
	float m_PlayTime;
	uint uid;
};
struct LevelRewardInfo
{
	int m_RewardHP;
	int m_RewardAttack;
	int m_RequireEnergy;

};
struct GameConfig
{
	//level
	LevelRewardInfo LevelReward[MAX_LEVEL_REWARD];
	//weapon
	WeaponInfo WeaponList[WeaponType::WeaponCount - 1];
	//brith
	Vector3 BrithPoints[10];
	int BrithPointsCount;
	int BrithPointIndex;
	//dropitem
	DropItemRefreshInfo DropRefreshItems[DROP_ITEM_COUNT];
	Vector3 DropRefreshPoints[MAX_DROP_POINT_COUNT];
	int DropRefreshPointsCount;
	//skill
	SkillInfo SkillInfos[DROP_ITEM_COUNT];
};
extern GameConfig gGameConfig;
#endif // !__CLIENTINFO_H__
