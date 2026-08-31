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
#include "core/saverestore.h"
#include "world/bmodels.h"

//
// CFuncWall
//
LINK_ENTITY_TO_CLASS( func_wall, CFuncWall );

void CFuncWall::Spawn( void )
{
	pev->angles   = g_vecZero;
	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything
	pev->solid    = SOLID_BSP;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

void CFuncWall::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, (int)( pev->frame ) ) )
		pev->frame = 1 - pev->frame;
}

//
// CFuncWallToggle
//
LINK_ENTITY_TO_CLASS( func_wall_toggle, CFuncWallToggle );

void CFuncWallToggle::Spawn( void )
{
	CFuncWall::Spawn();
	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
}

void CFuncWallToggle::TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}

void CFuncWallToggle::TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}

BOOL CFuncWallToggle::IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}

void CFuncWallToggle::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}

//
// CFuncIllusionary
//
LINK_ENTITY_TO_CLASS( func_illusionary, CFuncIllusionary );

void CFuncIllusionary::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "skin" ) ) // skin is used for content type
	{
		pev->skin      = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CFuncIllusionary::Spawn( void )
{
	pev->angles   = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid    = SOLID_NOT; // always solid_not
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

//
// CFuncMonsterClip
//
LINK_ENTITY_TO_CLASS( func_monsterclip, CFuncMonsterClip );

void CFuncMonsterClip::Spawn( void )
{
	CFuncWall::Spawn();
	if ( CVAR_GET_FLOAT( "showtriggers" ) == 0 )
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}
