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
struct WeaponInfo
{
	WeaponType Type;
	int Damage;
	float AttackTime;
	int Ammunition;
	float ReloadTime;
	bool Tracker;
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
	short m_WeaponCount;
	int m_Experience;
	int m_Score;
	int m_KillCount;
	int m_HP;
	uint uid;
};
#endif // !__CLIENTINFO_H__
