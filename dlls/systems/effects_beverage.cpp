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
#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "systems/effects_beverage.h"

//
// Beverage Dispenser
// overloaded pev->frags, is now a flag for whether or not a can is stuck in the dispenser.
// overloaded pev->health, is now how many cans remain in the machine.
//
LINK_ENTITY_TO_CLASS( env_beverage, CEnvBeverage );

void CEnvBeverage::Precache( void )
{
	PRECACHE_MODEL( "models/can.mdl" );
	PRECACHE_SOUND( "weapons/g_bounce3.wav" );
}

void CEnvBeverage::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->frags != 0 || pev->health <= 0 )
	{
		// no more cans while one is waiting in the dispenser, or if I'm out of cans.
		return;
	}

	CBaseEntity *pCan = CBaseEntity::Create( "item_sodacan", pev->origin, pev->angles, edict() );

	if ( pev->skin == 6 )
	{
		// random
		pCan->pev->skin = RANDOM_LONG( 0, 5 );
	}
	else
	{
		pCan->pev->skin = pev->skin;
	}

	pev->frags = 1;
	pev->health--;
}

void CEnvBeverage::Spawn( void )
{
	Precache();
	pev->solid   = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->frags   = 0;

	if ( pev->health == 0 )
	{
		pev->health = 10;
	}
}

//
// Soda can
//
LINK_ENTITY_TO_CLASS( item_sodacan, CItemSoda );

void CItemSoda::Precache( void )
{
}

void CItemSoda::Spawn( void )
{
	Precache();
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;

	SET_MODEL( ENT( pev ), "models/can.mdl" );
	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	SetThink( &CItemSoda::CanThink );
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::CanThink( void )
{
	EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/g_bounce3.wav", 1, ATTN_NORM );

	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize( pev, Vector( -8, -8, 0 ), Vector( 8, 8, 8 ) );
	SetThink( NULL );
	SetTouch( &CItemSoda::CanTouch );
}

void CItemSoda::CanTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	pOther->TakeHealth( 1, DMG_GENERIC ); // a bit of health.

	if ( !FNullEnt( pev->owner ) )
	{
		// tell the machine the can was taken
		pev->owner->v.frags = 0;
	}

	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects  = EF_NODRAW;
	SetTouch( NULL );
	SetThink( &CItemSoda::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}
