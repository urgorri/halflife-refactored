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
#ifndef MONSTERS_LEECH_H
#define MONSTERS_LEECH_H

#include "ai/monsters.h"

// Animation events
#define LEECH_AE_ATTACK 1
#define LEECH_AE_FLOP 2

// Movement constants
#define LEECH_ACCELERATE 10
#define LEECH_CHECK_DIST 45
#define LEECH_SWIM_SPEED 50
#define LEECH_SWIM_ACCEL 80
#define LEECH_SWIM_DECEL 10
#define LEECH_TURN_RATE 90
#define LEECH_SIZEX 10
#define LEECH_FRAMETIME 0.1

class CLeech : public CBaseMonster
{
  public:
	void Spawn( void );
	void Precache( void );

	void EXPORT SwimThink( void );
	void EXPORT DeadThink( void );
	void Touch( CBaseEntity *pOther )
	{
		if ( pOther->IsPlayer() )
		{
			// If the client is pushing me, give me some base velocity
			if ( gpGlobals->trace_ent && gpGlobals->trace_ent == edict() )
			{
				pev->basevelocity = pOther->pev->velocity;
				pev->flags |= FL_BASEVELOCITY;
			}
		}
	}

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -8, -8, 0 );
		pev->absmax = pev->origin + Vector( 8, 8, 2 );
	}

	void AttackSound( void );
	void AlertSound( void );
	void UpdateMotion( void );
	float ObstacleDistance( CBaseEntity *pTarget );
	void MakeVectors( void );
	void RecalculateWaterlevel( void );
	void SwitchLeechState( void );

	// Base entity functions
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int BloodColor( void ) { return DONT_BLEED; }
	void Killed( entvars_t *pevAttacker, int iGib );
	void Activate( void );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int Classify( void ) { return CLASS_INSECT; }
	int IRelationship( CBaseEntity *pTarget );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	static const char *pAttackSounds[];
	static const char *pAlertSounds[];

  private:
	// UNDONE: Remove unused boid vars, do group behavior
	float m_flTurning;   // is this boid turning?
	BOOL m_fPathBlocked; // TRUE if there is an obstacle ahead
	float m_flAccelerate;
	float m_obstacle;
	float m_top;
	float m_bottom;
	float m_height;
	float m_waterTime;
	float m_sideTime; // Timer to randomly check clearance on sides
	float m_zTime;
	float m_stateTime;
	float m_attackSoundTime;
};

#endif // MONSTERS_LEECH_H
