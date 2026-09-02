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
#ifndef MONSTERS_BULLSQUID_H
#define MONSTERS_BULLSQUID_H

#include "ai/monsters.h"

#define SQUID_SPRINT_DIST 256 // how close the squid has to get before starting to sprint and refusing to swerve

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_SQUID_HURTHOP = LAST_COMMON_SCHEDULE + 1,
	SCHED_SQUID_SMELLFOOD,
	SCHED_SQUID_SEECRAB,
	SCHED_SQUID_EAT,
	SCHED_SQUID_SNIFF_AND_EAT,
	SCHED_SQUID_WALLOW,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_SQUID_HOPTURN = LAST_COMMON_TASK + 1,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define BSQUID_AE_SPIT ( 1 )
#define BSQUID_AE_BITE ( 2 )
#define BSQUID_AE_BLINK ( 3 )
#define BSQUID_AE_TAILWHIP ( 4 )
#define BSQUID_AE_HOP ( 5 )
#define BSQUID_AE_THROW ( 6 )

class CBullsquid : public CBaseMonster
{
  public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int ISoundMask( void );
	int Classify( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void IdleSound( void );
	void PainSound( void );
	void DeathSound( void );
	void AlertSound( void );
	void AttackSound( void );
	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );
	BOOL CheckMeleeAttack1( float flDot, float flDist );
	BOOL CheckMeleeAttack2( float flDot, float flDist );
	BOOL CheckRangeAttack1( float flDot, float flDist );
	void RunAI( void );
	BOOL FValidateHintType( short sHint );
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int IRelationship( CBaseEntity *pTarget );
	int IgnoreConditions( void );
	MONSTERSTATE GetIdealState( void );

	int Save( CSave &save );
	int Restore( CRestore &restore );

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fCanThreatDisplay; // this is so the squid only does the "I see a headcrab!" dance one time.

	float m_flLastHurtTime; // we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime; // last time the bullsquid used the spit attack.
};

class CSquidSpit : public CBaseEntity
{
  public:
	void Spawn( void );

	static void Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	void Touch( CBaseEntity *pOther );
	void EXPORT Animate( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_maxFrame;
};

#endif // MONSTERS_BULLSQUID_H
