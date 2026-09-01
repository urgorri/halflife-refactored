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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/skill.h"
#include "monsters/gargantua_effects.h"
#include "systems/effects.h"

extern short g_sModelIndexSmoke;

LINK_ENTITY_TO_CLASS( streak_spiral, CSpiral );
LINK_ENTITY_TO_CLASS( garg_stomp, CStomp );
LINK_ENTITY_TO_CLASS( env_smoker, CSmoker );

void StreakSplash( const Vector &origin, const Vector &direction, int color, int count, int speed, int velocityRange )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
	WRITE_BYTE( TE_STREAK_SPLASH );
	WRITE_COORD( origin.x ); // origin
	WRITE_COORD( origin.y );
	WRITE_COORD( origin.z );
	WRITE_COORD( direction.x ); // direction
	WRITE_COORD( direction.y );
	WRITE_COORD( direction.z );
	WRITE_BYTE( color );  // Streak color 6
	WRITE_SHORT( count ); // count
	WRITE_SHORT( speed );
	WRITE_SHORT( velocityRange ); // Random velocity modifier
	MESSAGE_END();
}

CStomp *CStomp::StompCreate( const Vector &origin, const Vector &end, float speed )
{
	CStomp *pStomp = GetClassPtr( (CStomp *)NULL );

	pStomp->pev->origin  = origin;
	Vector dir           = ( end - origin );
	pStomp->pev->scale   = dir.Length();
	pStomp->pev->movedir = dir.Normalize();
	pStomp->pev->speed   = speed;
	pStomp->Spawn();

	return pStomp;
}

void CStomp::Spawn( void )
{
	pev->nextthink = gpGlobals->time;
	pev->classname = MAKE_STRING( "garg_stomp" );
	pev->dmgtime   = gpGlobals->time;

	pev->framerate  = 30;
	pev->model      = MAKE_STRING( GARG_STOMP_SPRITE_NAME );
	pev->rendermode = kRenderTransTexture;
	pev->renderamt  = 0;
	EMIT_SOUND_DYN( edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND, 1, ATTN_NORM, 0, PITCH_NORM * 0.55 );
}

#define STOMP_INTERVAL 0.025

void CStomp::Think( void )
{
	TraceResult tr;

	pev->nextthink = gpGlobals->time + 0.1;

	// Do damage for this frame
	Vector vecStart = pev->origin;
	vecStart.z += 30;
	Vector vecEnd = vecStart + ( pev->movedir * pev->speed * gpGlobals->frametime );

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT( pev ), &tr );

	if ( tr.pHit && tr.pHit != pev->owner )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		entvars_t *pevOwner  = pev;
		if ( pev->owner )
			pevOwner = VARS( pev->owner );

		if ( pEntity )
			pEntity->TakeDamage( pev, pevOwner, gSkillData.gargantuaDmgStomp, DMG_SONIC );
	}

	// Accelerate the effect
	pev->speed     = pev->speed + ( gpGlobals->frametime ) * pev->framerate;
	pev->framerate = pev->framerate + ( gpGlobals->frametime ) * 1500;

	// Move and spawn trails
	while ( gpGlobals->time - pev->dmgtime > STOMP_INTERVAL )
	{
		pev->origin = pev->origin + pev->movedir * pev->speed * STOMP_INTERVAL;
		for ( int i = 0; i < 2; i++ )
		{
			CSprite *pSprite = CSprite::SpriteCreate( GARG_STOMP_SPRITE_NAME, pev->origin, TRUE );
			if ( pSprite )
			{
				UTIL_TraceLine( pev->origin, pev->origin - Vector( 0, 0, 500 ), ignore_monsters, edict(), &tr );
				pSprite->pev->origin   = tr.vecEndPos;
				pSprite->pev->velocity = Vector( RANDOM_FLOAT( -200, 200 ), RANDOM_FLOAT( -200, 200 ), 175 );
				// pSprite->AnimateAndDie( RANDOM_FLOAT( 8.0, 12.0 ) );
				pSprite->pev->nextthink = gpGlobals->time + 0.3;
				pSprite->SetThink( &CSprite::SUB_Remove );
				pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxFadeFast );
			}
		}
		pev->dmgtime += STOMP_INTERVAL;
		// Scale has the "life" of this effect
		pev->scale -= STOMP_INTERVAL * pev->speed;
		if ( pev->scale <= 0 )
		{
			// Life has run out
			UTIL_Remove( this );
			STOP_SOUND( edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND );
		}
	}
}

void CSmoker::Spawn( void )
{
	pev->movetype  = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time;
	pev->solid     = SOLID_NOT;
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}

void CSmoker::Think( void )
{
	// lots of smoke
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
	WRITE_BYTE( TE_SMOKE );
	WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -pev->dmg, pev->dmg ) );
	WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -pev->dmg, pev->dmg ) );
	WRITE_COORD( pev->origin.z );
	WRITE_SHORT( g_sModelIndexSmoke );
	WRITE_BYTE( RANDOM_LONG( pev->scale, pev->scale * 1.1 ) );
	WRITE_BYTE( RANDOM_LONG( 8, 14 ) ); // framerate
	MESSAGE_END();

	pev->health--;
	if ( pev->health > 0 )
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.2 );
	else
		UTIL_Remove( this );
}

void CSpiral::Spawn( void )
{
	pev->movetype  = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time;
	pev->solid     = SOLID_NOT;
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}

CSpiral *CSpiral::Create( const Vector &origin, float height, float radius, float duration )
{
	if ( duration <= 0 )
		return NULL;

	CSpiral *pSpiral = GetClassPtr( (CSpiral *)NULL );
	pSpiral->Spawn();
	pSpiral->pev->dmgtime = pSpiral->pev->nextthink;
	pSpiral->pev->origin  = origin;
	pSpiral->pev->scale   = radius;
	pSpiral->pev->dmg     = height;
	pSpiral->pev->speed   = duration;
	pSpiral->pev->health  = 0;
	pSpiral->pev->angles  = g_vecZero;

	return pSpiral;
}

#define SPIRAL_INTERVAL 0.1 // 025

void CSpiral::Think( void )
{
	float time = gpGlobals->time - pev->dmgtime;

	while ( time > SPIRAL_INTERVAL )
	{
		Vector position  = pev->origin;
		Vector direction = Vector( 0, 0, 1 );

		float fraction = 1.0 / pev->speed;

		float radius = ( pev->scale * pev->health ) * fraction;

		position.z += ( pev->health * pev->dmg ) * fraction;
		pev->angles.y = ( pev->health * 360 * 8 ) * fraction;
		UTIL_MakeVectors( pev->angles );
		position  = position + gpGlobals->v_forward * radius;
		direction = ( direction + gpGlobals->v_forward ).Normalize();

		StreakSplash( position, Vector( 0, 0, 1 ), RANDOM_LONG( 8, 11 ), 20, RANDOM_LONG( 50, 150 ), 400 );

		// Jeez, how many counters should this take ? :)
		pev->dmgtime += SPIRAL_INTERVAL;
		pev->health += SPIRAL_INTERVAL;
		time -= SPIRAL_INTERVAL;
	}

	pev->nextthink = gpGlobals->time;

	if ( pev->health >= pev->speed )
		UTIL_Remove( this );
}

#endif
