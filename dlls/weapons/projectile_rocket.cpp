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
#if !defined( OEM_BUILD )

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "ai/monsters.h"
#include "weapons/weapon_base.h"
#include "weapons/weapon_rpg.h"
#include "weapons/projectile_rocket.h"
#include "ai/nodes.h"
#include "core/player.h"
#include "gameplay/gamerules.h"

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( laser_spot, CLaserSpot );

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING( "laser_spot" );

	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn( void )
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid    = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx   = kRenderFxNoDissipation;
	pev->renderamt  = 255;

	SET_MODEL( ENT( pev ), "sprites/laserdot.spr" );
	UTIL_SetOrigin( pev, pev->origin );
}

//=========================================================
// Suspend- make the laser sight invisible.
//=========================================================
void CLaserSpot::Suspend( float flSuspendTime )
{
	pev->effects |= EF_NODRAW;

	SetThink( &CLaserSpot::Revive );
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive( void )
{
	pev->effects &= ~EF_NODRAW;

	SetThink( NULL );
}

void CLaserSpot::Precache( void )
{
	PRECACHE_MODEL( "sprites/laserdot.spr" );
}

LINK_ENTITY_TO_CLASS( rpg_rocket, CRpgRocket );

//=========================================================
//=========================================================
CRpgRocket *CRpgRocket::CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher )
{
	CRpgRocket *pRocket = GetClassPtr( (CRpgRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch( &CRpgRocket::RocketTouch );

	pLauncher->m_cActiveRockets++;
	pRocket->m_hLauncher = pLauncher; // remember what RPG fired me.
	pRocket->pev->owner  = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket::Spawn( void )
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid    = SOLID_BBOX;

	SET_MODEL( ENT( pev ), "models/rpgrocket.mdl" );
	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING( "rpg_rocket" );

	SetThink( &CRpgRocket::IgniteThink );
	SetTouch( &CRpgRocket::ExplodeTouch );

	pev->angles.x -= 30;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -( pev->angles.x + 30 );

	pev->velocity = gpGlobals->v_forward * 250;
	pev->gravity  = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgRPG;
}

//=========================================================
//=========================================================
void CRpgRocket::RocketTouch( CBaseEntity *pOther )
{
	if ( GetLauncher() )
	{
		// my launcher is still around, tell it I'm dead.
		GetLauncher()->m_cActiveRockets--;
		m_hLauncher = NULL;
	}

	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouch( pOther );
}

//=========================================================
void CRpgRocket::Explode( TraceResult *pTrace, int bitsDamageType )
{
	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );

	if ( GetLauncher() )
	{
		// my launcher is still around, tell it I'm dead.
		GetLauncher()->m_cActiveRockets--;
		m_hLauncher = NULL;
	}

	CGrenade::Explode( pTrace, bitsDamageType );
}

//=========================================================
//=========================================================
void CRpgRocket::Precache( void )
{
	PRECACHE_MODEL( "models/rpgrocket.mdl" );
	m_iTrail = PRECACHE_MODEL( "sprites/smoke.spr" );
	PRECACHE_SOUND( "weapons/rocket1.wav" );
}

void CRpgRocket::IgniteThink( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );

	// rocket trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );

	WRITE_BYTE( TE_BEAMFOLLOW );
	WRITE_SHORT( entindex() ); // entity
	WRITE_SHORT( m_iTrail );   // model
	WRITE_BYTE( 40 );          // life
	WRITE_BYTE( 5 );           // width
	WRITE_BYTE( 224 );         // r, g, b
	WRITE_BYTE( 224 );         // r, g, b
	WRITE_BYTE( 255 );         // r, g, b
	WRITE_BYTE( 255 );         // brightness

	MESSAGE_END();

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink( &CRpgRocket::FollowThink );
	pev->nextthink = gpGlobals->time + 0.1;
}

CRpg *CRpgRocket::GetLauncher()
{
	if ( !m_hLauncher )
		return NULL;

	return (CRpg *)( (CBaseEntity *)m_hLauncher );
}

void CRpgRocket::FollowThink( void )
{
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors( pev->angles );

	vecTarget = gpGlobals->v_forward;
	flMax     = 4096;

	// Examine all entities within a reasonable radius
	while ( ( pOther = UTIL_FindEntityByClassname( pOther, "laser_spot" ) ) != NULL )
	{
		Vector vSpotLocation = pOther->pev->origin;

		if ( UTIL_PointContents( vSpotLocation ) == CONTENTS_SKY )
		{
		}

		UTIL_TraceLine( pev->origin, vSpotLocation, dont_ignore_monsters, ENT( pev ), &tr );

		if ( tr.flFraction >= 0.90 )
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			flDot  = DotProduct( gpGlobals->v_forward, vecDir );
			if ( ( flDot > 0 ) && ( flDist * ( 1 - flDot ) < flMax ) )
			{
				flMax     = flDist * ( 1 - flDot );
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles( vecTarget );

	float flSpeed = pev->velocity.Length();
	if ( gpGlobals->time - m_flIgniteTime < 1.0 )
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * ( flSpeed * 0.8 + 400 );
		if ( pev->waterlevel == 3 )
		{
			// go slow underwater
			if ( pev->velocity.Length() > 300 )
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 4 );
		}
		else
		{
			if ( pev->velocity.Length() > 2000 )
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if ( pev->effects & EF_LIGHT )
		{
			pev->effects = 0;
			STOP_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav" );
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if ( pev->waterlevel == 0 && pev->velocity.Length() < 1500 )
		{
			Detonate();
		}
	}

	if ( GetLauncher() )
	{
		float flDistance = ( pev->origin - GetLauncher()->pev->origin ).Length();

		// if we've travelled more than max distance the player can send a spot, stop tracking the original launcher (allow it to reload)
		if ( flDistance > 8192.0f || gpGlobals->time - m_flIgniteTime > 6.0f )
		{
			GetLauncher()->m_cActiveRockets--;
			m_hLauncher = NULL;
		}
	}

	if ( ( UTIL_PointContents( pev->origin ) == CONTENTS_SKY ) )
	{
		Detonate();
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

TYPEDESCRIPTION CRpgRocket::m_SaveData[] =
    {
        DEFINE_FIELD( CRpgRocket, m_flIgniteTime, FIELD_TIME ),
        DEFINE_FIELD( CRpgRocket, m_hLauncher, FIELD_EHANDLE ),
};

IMPLEMENT_SAVERESTORE( CRpgRocket, CGrenade );

#endif // CLIENT_DLL

#endif // OEM_BUILD
