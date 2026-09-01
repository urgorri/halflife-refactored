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

===== plats.cpp ========================================================

  spawn, think, and touch functions for trains, etc

*/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "world/plats.h"
#include "world/trains.h"
#include "core/saverestore.h"

static void PlatSpawnInsideTrigger( entvars_t *pevPlatform );

TYPEDESCRIPTION CBasePlatTrain::m_SaveData[] =
    {
        DEFINE_FIELD( CBasePlatTrain, m_bMoveSnd, FIELD_CHARACTER ),
        DEFINE_FIELD( CBasePlatTrain, m_bStopSnd, FIELD_CHARACTER ),
        DEFINE_FIELD( CBasePlatTrain, m_volume, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CBasePlatTrain, CBaseToggle );

void CBasePlatTrain ::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "lip" ) )
	{
		m_flLip        = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "wait" ) )
	{
		m_flWait       = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "height" ) )
	{
		m_flHeight     = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "rotation" ) )
	{
		m_vecFinalAngle.x = atof( pkvd->szValue );
		pkvd->fHandled    = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "movesnd" ) )
	{
		m_bMoveSnd     = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "stopsnd" ) )
	{
		m_bStopSnd     = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "volume" ) )
	{
		m_volume       = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

#define noiseMoving noise
#define noiseArrived noise1

void CBasePlatTrain::Precache( void )
{
	// set the plat's "in-motion" sound
	switch ( m_bMoveSnd )
	{
	case 0:
		pev->noiseMoving = MAKE_STRING( "common/null.wav" );
		break;
	case 1:
		PRECACHE_SOUND( "plats/bigmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/bigmove1.wav" );
		break;
	case 2:
		PRECACHE_SOUND( "plats/bigmove2.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/bigmove2.wav" );
		break;
	case 3:
		PRECACHE_SOUND( "plats/elevmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/elevmove1.wav" );
		break;
	case 4:
		PRECACHE_SOUND( "plats/elevmove2.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/elevmove2.wav" );
		break;
	case 5:
		PRECACHE_SOUND( "plats/elevmove3.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/elevmove3.wav" );
		break;
	case 6:
		PRECACHE_SOUND( "plats/freightmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/freightmove1.wav" );
		break;
	case 7:
		PRECACHE_SOUND( "plats/freightmove2.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/freightmove2.wav" );
		break;
	case 8:
		PRECACHE_SOUND( "plats/heavymove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/heavymove1.wav" );
		break;
	case 9:
		PRECACHE_SOUND( "plats/rackmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/rackmove1.wav" );
		break;
	case 10:
		PRECACHE_SOUND( "plats/railmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/railmove1.wav" );
		break;
	case 11:
		PRECACHE_SOUND( "plats/squeekmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/squeekmove1.wav" );
		break;
	case 12:
		PRECACHE_SOUND( "plats/talkmove1.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/talkmove1.wav" );
		break;
	case 13:
		PRECACHE_SOUND( "plats/talkmove2.wav" );
		pev->noiseMoving = MAKE_STRING( "plats/talkmove2.wav" );
		break;
	default:
		pev->noiseMoving = MAKE_STRING( "common/null.wav" );
		break;
	}

	// set the plat's 'reached destination' stop sound
	switch ( m_bStopSnd )
	{
	case 0:
		pev->noiseArrived = MAKE_STRING( "common/null.wav" );
		break;
	case 1:
		PRECACHE_SOUND( "plats/bigstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/bigstop1.wav" );
		break;
	case 2:
		PRECACHE_SOUND( "plats/bigstop2.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/bigstop2.wav" );
		break;
	case 3:
		PRECACHE_SOUND( "plats/freightstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/freightstop1.wav" );
		break;
	case 4:
		PRECACHE_SOUND( "plats/heavystop2.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/heavystop2.wav" );
		break;
	case 5:
		PRECACHE_SOUND( "plats/rackstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/rackstop1.wav" );
		break;
	case 6:
		PRECACHE_SOUND( "plats/railstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/railstop1.wav" );
		break;
	case 7:
		PRECACHE_SOUND( "plats/squeekstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/squeekstop1.wav" );
		break;
	case 8:
		PRECACHE_SOUND( "plats/talkstop1.wav" );
		pev->noiseArrived = MAKE_STRING( "plats/talkstop1.wav" );
		break;

	default:
		pev->noiseArrived = MAKE_STRING( "common/null.wav" );
		break;
	}
}

//
//====================== PLAT code ====================================================
//

#define noiseMovement noise
#define noiseStopMoving noise1

LINK_ENTITY_TO_CLASS( func_plat, CFuncPlat );

/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in
the extended position until it is trigger, when it will lower and become a normal plat.

If the "height" key is set, that will determine the amount the plat moves, instead of
being implicitly determined by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/

void CFuncPlat ::Setup( void )
{
	// pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
	// pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

	if ( m_flTLength == 0 )
		m_flTLength = 80;
	if ( m_flTWidth == 0 )
		m_flTWidth = 10;

	pev->angles = g_vecZero;

	pev->solid    = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin( pev, pev->origin ); // set size and link into world
	UTIL_SetSize( pev, pev->mins, pev->maxs );
	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	// vecPosition1 is the top position, vecPosition2 is the bottom
	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;
	if ( m_flHeight != 0 )
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;
	if ( pev->speed == 0 )
		pev->speed = 150;

	if ( m_volume == 0 )
		m_volume = 0.85;
}

void CFuncPlat ::Precache()
{
	CBasePlatTrain::Precache();
	// PRECACHE_SOUND("plats/platmove1.wav");
	// PRECACHE_SOUND("plats/platstop1.wav");
	if ( !IsTogglePlat() )
		PlatSpawnInsideTrigger( pev ); // the "start moving" trigger
}

void CFuncPlat ::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if ( !FStringNull( pev->targetname ) )
	{
		UTIL_SetOrigin( pev, m_vecPosition1 );
		m_toggle_state = TS_AT_TOP;
		SetUse( &CFuncPlat::PlatUse );
	}
	else
	{
		UTIL_SetOrigin( pev, m_vecPosition2 );
		m_toggle_state = TS_AT_BOTTOM;
	}
}

static void PlatSpawnInsideTrigger( entvars_t *pevPlatform )
{
	GetClassPtr( (CPlatTrigger *)NULL )->SpawnInsideTrigger( GetClassPtr( (CFuncPlat *)pevPlatform ) );
}

//
// Create a trigger entity for a platform.
//
void CPlatTrigger ::SpawnInsideTrigger( CFuncPlat *pPlatform )
{
	m_pPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	pev->solid    = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->origin   = pPlatform->pev->origin;

	// Establish the trigger field's size
	Vector vecTMin = m_pPlatform->pev->mins + Vector( 25, 25, 0 );
	Vector vecTMax = m_pPlatform->pev->maxs + Vector( 25, 25, 8 );
	vecTMin.z      = vecTMax.z - ( m_pPlatform->m_vecPosition1.z - m_pPlatform->m_vecPosition2.z + 8 );
	if ( m_pPlatform->pev->size.x <= 50 )
	{
		vecTMin.x = ( m_pPlatform->pev->mins.x + m_pPlatform->pev->maxs.x ) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if ( m_pPlatform->pev->size.y <= 50 )
	{
		vecTMin.y = ( m_pPlatform->pev->mins.y + m_pPlatform->pev->maxs.y ) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	UTIL_SetSize( pev, vecTMin, vecTMax );
}

//
// When the platform's trigger field is touched, the platform ???
//
void CPlatTrigger ::Touch( CBaseEntity *pOther )
{
	// Ignore touches by non-players
	entvars_t *pevToucher = pOther->pev;
	if ( !FClassnameIs( pevToucher, "player" ) )
		return;

	// Ignore touches by corpses
	if ( !pOther->IsAlive() || !m_pPlatform || !m_pPlatform->pev )
		return;

	// Make linked platform go up/down.
	if ( m_pPlatform->m_toggle_state == TS_AT_BOTTOM )
		m_pPlatform->GoUp();
	else if ( m_pPlatform->m_toggle_state == TS_AT_TOP )
		m_pPlatform->pev->nextthink = m_pPlatform->pev->ltime + 1; // delay going down
}

//
// Used by SUB_UseTargets, when a platform is the target of a button.
// Start bringing platform down.
//
void CFuncPlat ::PlatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( IsTogglePlat() )
	{
		// Top is off, bottom is on
		BOOL on = ( m_toggle_state == TS_AT_BOTTOM ) ? TRUE : FALSE;

		if ( !ShouldToggle( useType, on ) )
			return;

		if ( m_toggle_state == TS_AT_TOP )
			GoDown();
		else if ( m_toggle_state == TS_AT_BOTTOM )
			GoUp();
	}
	else
	{
		SetUse( NULL );

		if ( m_toggle_state == TS_AT_TOP )
			GoDown();
	}
}

//
// Platform is at top, now starts moving down.
//
void CFuncPlat ::GoDown( void )
{
	if ( pev->noiseMovement )
		EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP );
	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone( &CFuncPlat::CallHitBottom );
	LinearMove( m_vecPosition2, pev->speed );
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlat ::HitBottom( void )
{
	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );

	if ( pev->noiseStopMoving )
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_GOING_DOWN );
	m_toggle_state = TS_AT_BOTTOM;
}

//
// Platform is at bottom, now starts moving up
//
void CFuncPlat ::GoUp( void )
{
	if ( pev->noiseMovement )
		EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN );
	m_toggle_state = TS_GOING_UP;
	SetMoveDone( &CFuncPlat::CallHitTop );
	LinearMove( m_vecPosition1, pev->speed );
}

//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlat ::HitTop( void )
{
	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );

	if ( pev->noiseStopMoving )
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_GOING_UP );
	m_toggle_state = TS_AT_TOP;

	if ( !IsTogglePlat() )
	{
		// After a delay, the platform will automatically start going down again.
		SetThink( &CFuncPlat::CallGoDown );
		pev->nextthink = pev->ltime + 3;
	}
}

void CFuncPlat ::Blocked( CBaseEntity *pOther )
{
	ALERT( at_aiconsole, "%s Blocked by %s\n", STRING( pev->classname ), STRING( pOther->pev->classname ) );
	// Hurt the blocker a little
	pOther->TakeDamage( pev, pev, 1, DMG_CRUSH );

	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );

	// Send the platform back where it came from
	ASSERT( m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN );
	if ( m_toggle_state == TS_GOING_UP )
		GoDown();
	else if ( m_toggle_state == TS_GOING_DOWN )
		GoUp();
}

LINK_ENTITY_TO_CLASS( func_platrot, CFuncPlatRot );
TYPEDESCRIPTION CFuncPlatRot::m_SaveData[] =
    {
        DEFINE_FIELD( CFuncPlatRot, m_end, FIELD_VECTOR ),
        DEFINE_FIELD( CFuncPlatRot, m_start, FIELD_VECTOR ),
};

IMPLEMENT_SAVERESTORE( CFuncPlatRot, CFuncPlat );

void CFuncPlatRot ::SetupRotation( void )
{
	if ( m_vecFinalAngle.x != 0 ) // This plat rotates too!
	{
		CBaseToggle ::AxisDir( pev );
		m_start = pev->angles;
		m_end   = pev->angles + pev->movedir * m_vecFinalAngle.x;
	}
	else
	{
		m_start = g_vecZero;
		m_end   = g_vecZero;
	}
	if ( !FStringNull( pev->targetname ) ) // Start at top
	{
		pev->angles = m_end;
	}
}

void CFuncPlatRot ::Spawn( void )
{
	CFuncPlat ::Spawn();
	SetupRotation();
}

void CFuncPlatRot ::GoDown( void )
{
	CFuncPlat ::GoDown();
	RotMove( m_start, pev->nextthink - pev->ltime );
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlatRot ::HitBottom( void )
{
	CFuncPlat ::HitBottom();
	pev->avelocity = g_vecZero;
	pev->angles    = m_start;
}

//
// Platform is at bottom, now starts moving up
//
void CFuncPlatRot ::GoUp( void )
{
	CFuncPlat ::GoUp();
	RotMove( m_end, pev->nextthink - pev->ltime );
}

//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlatRot ::HitTop( void )
{
	CFuncPlat ::HitTop();
	pev->avelocity = g_vecZero;
	pev->angles    = m_end;
}

void CFuncPlatRot ::RotMove( Vector &destAngle, float time )
{
	// set destdelta to the vector needed to move
	Vector vecDestDelta = destAngle - pev->angles;

	// Travel time is so short, we're practically there already;  so make it so.
	if ( time >= 0.1 )
		pev->avelocity = vecDestDelta / time;
	else
	{
		pev->avelocity = vecDestDelta;
		pev->nextthink = pev->ltime + 1;
	}
}

