#pragma once
#ifndef __CLIENTINFO_H__
#define __CLIENTINFO_H__
#include <common.h>
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
	float m_RefreshTime;
	int m_RefreshCount;
	float m_Duration;
	float m_StartTime;


};
struct DropItemInfo
{
	uint uid;
	DropItemType m_Type;
	float m_Duration;

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
struct BufferInfo
{
	BuffType m_Type;
	float m_Duration;
	bool m_Enabled;
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
	bool m_Dead;
	float m_BrithTime;
	float m_PlayTime;
	uint uid;
};
#endif // !__CLIENTINFO_H__
