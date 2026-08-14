/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
/*

===== trigger_monsterjump.cpp ========================================================

  spawn and use functions for editor-placed triggers

*/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "player.h"
#include "core/saverestore.h"
#include "trains.h" // trigger_camera has train functionality
#include "gameplay/gamerules.h"
#include "trigger_base.h"

extern void SetMovedir( entvars_t *pev );

class CTriggerMonsterJump : public CBaseTrigger
{
  public:
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
};

LINK_ENTITY_TO_CLASS( trigger_monsterjump, CTriggerMonsterJump );

void CTriggerMonsterJump ::Spawn( void )
{
	SetMovedir( pev );

	InitTrigger();

	pev->nextthink = 0;
	pev->speed     = 200;
	m_flHeight     = 150;

	if ( !FStringNull( pev->targetname ) )
	{ // if targetted, spawn turned off
		pev->solid = SOLID_NOT;
		UTIL_SetOrigin( pev, pev->origin ); // Unlink from trigger list
		SetUse( &CBaseTrigger::ToggleUse );
	}
}

void CTriggerMonsterJump ::Think( void )
{
	pev->solid = SOLID_NOT;             // kill the trigger for now !!!UNDONE
	UTIL_SetOrigin( pev, pev->origin ); // Unlink from trigger list
	SetThink( NULL );
}

void CTriggerMonsterJump ::Touch( CBaseEntity *pOther )
{
	entvars_t *pevOther = pOther->pev;

	if ( !FBitSet( pevOther->flags, FL_MONSTER ) )
	{ // touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if ( FBitSet( pevOther->flags, FL_ONGROUND ) )
	{ // clear the onground so physics don't bitch
		pevOther->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;
	pev->nextthink = gpGlobals->time;
}

//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
