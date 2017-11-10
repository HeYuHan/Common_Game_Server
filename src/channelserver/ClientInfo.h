#pragma once
#ifndef __CLIENTINFO_H__
#define __CLIENTINFO_H__
#include <common.h>
typedef enum
{
	MachineGun=1,
	Missile,
	JetFlame,
	WeaponCount
}WeaponType;
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

};
struct CharacterInGameInfo
{
public:
	WeaponInfo m_WeaponList[WeaponCount - 1];
	short WeaponCount;
	float m_Experience;
	float m_Score;
	int m_KillCount;
	int m_HP;
	uint uid;
};
#endif // !__CLIENTINFO_H__
