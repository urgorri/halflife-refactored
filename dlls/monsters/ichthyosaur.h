/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   This source code contains proprietary and confidential information of
 *   Valve LLC and its suppliers.  Access to this code is restricted to
 *   persons who have executed a written SDK license with Valve.  Any access,
 *   use or distribution of this code by or to any unlicensed person is illegal.
 *
 ****/
#ifndef MONSTERS_ICHTHYOSAUR_H
#define MONSTERS_ICHTHYOSAUR_H

#include "ai/flyingmonster.h"
#include "systems/effects.h"

#define SEARCH_RETRY 16
#define ICHTHYOSAUR_SPEED 150

#define EYE_MAD 0
#define EYE_BASE 1
#define EYE_CLOSED 2
#define EYE_BACK 3
#define EYE_LOOK 4

class CIchthyosaur : public CFlyingMonster
{
  public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	CUSTOM_SCHEDULES;

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );

	void Killed( entvars_t *pevAttacker, int iGib );
	void BecomeDead( void );

	void EXPORT CombatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT BiteTouch( CBaseEntity *pOther );

	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );

	BOOL CheckMeleeAttack1( float flDot, float flDist );
	BOOL CheckRangeAttack1( float flDot, float flDist );

	float ChangeYaw( int yawSpeed );
	Activity GetStoppedActivity( void );

	void Move( float flInterval );
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	void MonsterThink( void );
	void Stop( void );
	void Swim( void );
	Vector DoProbe( const Vector &Probe );

	float VectorToPitch( const Vector &vec );
	float FlPitchDiff( void );
	float ChangePitch( int pitchSpeed );

	Vector m_SaveVelocity;
	float m_idealDist;

	float m_flBlink;

	float m_flEnemyTouched;
	BOOL m_bOnAttack;

	float m_flMaxSpeed;
	float m_flMinSpeed;
	float m_flMaxDist;

	CBeam *m_pBeam;

	float m_flNextAlert;

	float m_flLastPitchTime; // Last frame time pitch was changed
	float m_flLastZYawTime;  // Last frame time Z was changed when yaw was changed

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pAttackSounds[];
	static const char *pBiteSounds[];
	static const char *pDieSounds[];
	static const char *pPainSounds[];

	void IdleSound( void );
	void AlertSound( void );
	void AttackSound( void );
	void BiteSound( void );
	void DeathSound( void );
	void PainSound( void );
};

#endif // MONSTERS_ICHTHYOSAUR_H
