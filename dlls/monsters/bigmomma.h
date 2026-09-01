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
#ifndef MONSTERS_BIGMOMMA_H
#define MONSTERS_BIGMOMMA_H

#include "ai/monsters.h"

#define SF_INFOBM_RUN 0x0001
#define SF_INFOBM_WAIT 0x0002

// AI Nodes for Big Momma
class CInfoBM : public CPointEntity
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_preSequence;
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define BIG_AE_STEP1 1      // Footstep left
#define BIG_AE_STEP2 2      // Footstep right
#define BIG_AE_STEP3 3      // Footstep back left
#define BIG_AE_STEP4 4      // Footstep back right
#define BIG_AE_SACK 5       // Sack slosh
#define BIG_AE_DEATHSOUND 6 // Death sound

#define BIG_AE_MELEE_ATTACKBR 8  // Leg attack
#define BIG_AE_MELEE_ATTACKBL 9  // Leg attack
#define BIG_AE_MELEE_ATTACK1 10  // Leg attack
#define BIG_AE_MORTAR_ATTACK1 11 // Launch a mortar
#define BIG_AE_LAY_CRAB 12       // Lay a headcrab
#define BIG_AE_JUMP_FORWARD 13   // Jump up and forward
#define BIG_AE_SCREAM 14         // alert sound
#define BIG_AE_PAIN_SOUND 15     // pain sound
#define BIG_AE_ATTACK_SOUND 16   // attack sound
#define BIG_AE_BIRTH_SOUND 17    // birth sound
#define BIG_AE_EARLY_TARGET 50   // Fire target early

// User defined conditions
#define bits_COND_NODE_SEQUENCE ( bits_COND_SPECIAL1 ) // pev->netname contains the name of a sequence to play

// Attack distance constants
#define BIG_ATTACKDIST 170
#define BIG_MORTARDIST 800
#define BIG_MAXCHILDREN 20 // Max # of live headcrab children

#define bits_MEMORY_CHILDPAIR ( bits_MEMORY_CUSTOM1 )
#define bits_MEMORY_ADVANCE_NODE ( bits_MEMORY_CUSTOM2 )
#define bits_MEMORY_COMPLETED_NODE ( bits_MEMORY_CUSTOM3 )
#define bits_MEMORY_FIRED_NODE ( bits_MEMORY_CUSTOM4 )

#define BIG_CHILDCLASS "monster_babycrab"

class CBigMomma : public CBaseMonster
{
  public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void Activate( void );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );

	void NodeStart( int iszNextNode );
	void NodeReach( void );
	BOOL ShouldGoToNode( void );

	void SetYawSpeed( void );
	int Classify( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void LayHeadcrab( void );

	int GetNodeSequence( void )
	{
		CBaseEntity *pTarget = m_hTargetEnt;
		if ( pTarget )
		{
			return pTarget->pev->netname; // netname holds node sequence
		}
		return 0;
	}

	int GetNodePresequence( void )
	{
		CInfoBM *pTarget = (CInfoBM *)(CBaseEntity *)m_hTargetEnt;
		if ( pTarget )
		{
			return pTarget->m_preSequence;
		}
		return 0;
	}

	float GetNodeDelay( void )
	{
		CBaseEntity *pTarget = m_hTargetEnt;
		if ( pTarget )
		{
			return pTarget->pev->speed; // Speed holds node delay
		}
		return 0;
	}

	float GetNodeRange( void )
	{
		CBaseEntity *pTarget = m_hTargetEnt;
		if ( pTarget )
		{
			return pTarget->pev->scale; // Scale holds node delay
		}
		return 1e6;
	}

	float GetNodeYaw( void )
	{
		CBaseEntity *pTarget = m_hTargetEnt;
		if ( pTarget )
		{
			if ( pTarget->pev->angles.y != 0 )
				return pTarget->pev->angles.y;
		}
		return pev->angles.y;
	}

	// Restart the crab count on each new level
	void OverrideReset( void )
	{
		m_crabCount = 0;
	}

	void DeathNotice( entvars_t *pevChild );

	BOOL CanLayCrab( void )
	{
		if ( m_crabTime < gpGlobals->time && m_crabCount < BIG_MAXCHILDREN )
		{
			// Don't spawn crabs inside each other
			Vector mins = pev->origin - Vector( 32, 32, 0 );
			Vector maxs = pev->origin + Vector( 32, 32, 0 );

			CBaseEntity *pList[2];
			int count = UTIL_EntitiesInBox( pList, 2, mins, maxs, FL_MONSTER );
			for ( int i = 0; i < count; i++ )
			{
				if ( pList[i] != this ) // Don't hurt yourself!
					return FALSE;
			}
			return TRUE;
		}

		return FALSE;
	}

	void LaunchMortar( void );

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -95, -95, 0 );
		pev->absmax = pev->origin + Vector( 95, 95, 190 );
	}

	BOOL CheckMeleeAttack1( float flDot, float flDist ); // Slash
	BOOL CheckMeleeAttack2( float flDot, float flDist ); // Lay a crab
	BOOL CheckRangeAttack1( float flDot, float flDist ); // Mortar launch

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	static const char *pChildDieSounds[];
	static const char *pSackSounds[];
	static const char *pDeathSounds[];
	static const char *pAttackSounds[];
	static const char *pAttackHitSounds[];
	static const char *pBirthSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pFootSounds[];

	CUSTOM_SCHEDULES;

  private:
	float m_nodeTime;
	float m_crabTime;
	float m_mortarTime;
	float m_painSoundTime;
	int m_crabCount;
};

#endif // MONSTERS_BIGMOMMA_H
