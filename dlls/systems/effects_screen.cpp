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
#include "systems/effects_screen.h"
#include "shake.h"

// Screen shake definitions
LINK_ENTITY_TO_CLASS( env_shake, CShake );

#define SF_SHAKE_EVERYONE 0x0001 // Don't check radius
#define SF_SHAKE_DISRUPT 0x0002  // Disrupt controls
#define SF_SHAKE_INAIR 0x0004    // Shake players in air

void CShake::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects  = 0;
	pev->frame    = 0;

	if ( pev->spawnflags & SF_SHAKE_EVERYONE )
		pev->dmg = 0;
}

void CShake::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "amplitude" ) )
	{
		SetAmplitude( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "frequency" ) )
	{
		SetFrequency( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "duration" ) )
	{
		SetDuration( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "radius" ) )
	{
		SetRadius( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

void CShake::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	UTIL_ScreenShake( pev->origin, Amplitude(), Frequency(), Duration(), Radius() );
}

// Screen fade definitions
LINK_ENTITY_TO_CLASS( env_fade, CFade );

#define SF_FADE_IN 0x0001       // Fade in, not out
#define SF_FADE_MODULATE 0x0002 // Modulate, don't blend
#define SF_FADE_ONLYONE 0x0004

void CFade::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects  = 0;
	pev->frame    = 0;
}

void CFade::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "duration" ) )
	{
		SetDuration( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "holdtime" ) )
	{
		SetHoldTime( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

void CFade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int fadeFlags = 0;

	if ( !( pev->spawnflags & SF_FADE_IN ) )
		fadeFlags |= FFADE_OUT;

	if ( pev->spawnflags & SF_FADE_MODULATE )
		fadeFlags |= FFADE_MODULATE;

	if ( pev->spawnflags & SF_FADE_ONLYONE )
	{
		if ( pActivator && pActivator->IsNetClient() )
		{
			UTIL_ScreenFade( pActivator, pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags );
		}
	}
	else
	{
		UTIL_ScreenFadeAll( pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags );
	}
	SUB_UseTargets( this, USE_TOGGLE, 0 );
}

// Message definitions
LINK_ENTITY_TO_CLASS( env_message, CMessage );

void CMessage::Spawn( void )
{
	Precache();

	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	switch ( pev->impulse )
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2: // Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3: // EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if ( pev->scale <= 0 )
		pev->scale = 1.0;
}

void CMessage::Precache( void )
{
	if ( pev->noise )
		PRECACHE_SOUND( (char *)STRING( pev->noise ) );
}

void CMessage::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "messagesound" ) )
	{
		pev->noise     = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "messagevolume" ) )
	{
		pev->scale     = atof( pkvd->szValue ) * 0.1;
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "messageattenuation" ) )
	{
		pev->impulse   = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

void CMessage::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = NULL;

	if ( pev->spawnflags & SF_MESSAGE_ALL )
		UTIL_ShowMessageAll( STRING( pev->message ) );
	else
	{
		if ( pActivator && pActivator->IsPlayer() )
			pPlayer = pActivator;
		else
		{
			pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
		}
		if ( pPlayer )
			UTIL_ShowMessage( STRING( pev->message ), pPlayer );
	}
	if ( pev->noise )
	{
		EMIT_SOUND( edict(), CHAN_BODY, STRING( pev->noise ), pev->scale, pev->speed );
	}
	if ( pev->spawnflags & SF_MESSAGE_ONCE )
		UTIL_Remove( this );

	SUB_UseTargets( this, USE_TOGGLE, 0 );
}
