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
#include "core/decals.h"
#include "systems/effects_env.h"
#include "systems/effects.h"

//
// CBubbling implementation
//
LINK_ENTITY_TO_CLASS( env_bubbles, CBubbling );

TYPEDESCRIPTION CBubbling::m_SaveData[] =
    {
        DEFINE_FIELD( CBubbling, m_density, FIELD_INTEGER ),
        DEFINE_FIELD( CBubbling, m_frequency, FIELD_INTEGER ),
        DEFINE_FIELD( CBubbling, m_state, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CBubbling, CBaseEntity );

#define SF_BUBBLES_STARTOFF 0x0001

void CBubbling::Spawn( void )
{
	Precache();
	SET_MODEL( ENT( pev ), STRING( pev->model ) ); // Set size

	pev->solid      = SOLID_NOT; // Remove model & collisions
	pev->renderamt  = 0;         // The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;
	int speed       = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	pev->rendercolor.x = speed >> 8;
	pev->rendercolor.y = speed & 255;
	pev->rendercolor.z = ( pev->speed < 0 ) ? 1 : 0;

	if ( !( pev->spawnflags & SF_BUBBLES_STARTOFF ) )
	{
		SetThink( &CBubbling::FizzThink );
		pev->nextthink = gpGlobals->time + 2.0;
		m_state        = 1;
	}
	else
		m_state = 0;
}

void CBubbling::Precache( void )
{
	m_bubbleModel = PRECACHE_MODEL( "sprites/bubble.spr" ); // Precache bubble sprite
}

void CBubbling::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, m_state ) )
		m_state = !m_state;

	if ( m_state )
	{
		SetThink( &CBubbling::FizzThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetThink( NULL );
		pev->nextthink = 0;
	}
}

void CBubbling::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "density" ) )
	{
		m_density      = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "frequency" ) )
	{
		m_frequency    = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "current" ) )
	{
		pev->speed     = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CBubbling::FizzThink( void )
{
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, VecBModelOrigin( pev ) );
	WRITE_BYTE( TE_FIZZ );
	WRITE_SHORT( (short)ENTINDEX( edict() ) );
	WRITE_SHORT( (short)m_bubbleModel );
	WRITE_BYTE( m_density );
	MESSAGE_END();

	if ( m_frequency > 19 )
		pev->nextthink = gpGlobals->time + 0.5;
	else
		pev->nextthink = gpGlobals->time + 2.5 - ( 0.1 * m_frequency );
}

//
// CTestEffect implementation
//
LINK_ENTITY_TO_CLASS( test_effect, CTestEffect );

void CTestEffect::Spawn( void )
{
	Precache();
}

void CTestEffect::Precache( void )
{
	PRECACHE_MODEL( "sprites/lgtning.spr" );
}

void CTestEffect::TestThink( void )
{
	int i;
	float t = ( gpGlobals->time - m_flStartTime );

	if ( m_iBeam < 24 )
	{
		CBeam *pbeam = CBeam::BeamCreate( "sprites/lgtning.spr", 100 );

		TraceResult tr;

		Vector vecSrc = pev->origin;
		Vector vecDir = Vector( RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ) );
		vecDir        = vecDir.Normalize();
		UTIL_TraceLine( vecSrc, vecSrc + vecDir * 128, ignore_monsters, ENT( pev ), &tr );

		pbeam->PointsInit( vecSrc, tr.vecEndPos );
		pbeam->SetColor( 255, 180, 100 );
		pbeam->SetWidth( 100 );
		pbeam->SetScrollRate( 12 );

		m_flBeamTime[m_iBeam] = gpGlobals->time;
		m_pBeam[m_iBeam]      = pbeam;
		m_iBeam++;
	}

	if ( t < 3.0 )
	{
		for ( i = 0; i < m_iBeam; i++ )
		{
			t = ( gpGlobals->time - m_flBeamTime[i] ) / ( 3 + m_flStartTime - m_flBeamTime[i] );
			m_pBeam[i]->SetBrightness( 255 * t );
		}
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		for ( i = 0; i < m_iBeam; i++ )
		{
			UTIL_Remove( m_pBeam[i] );
		}
		m_flStartTime = gpGlobals->time;
		m_iBeam       = 0;
		SetThink( NULL );
	}
}

void CTestEffect::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CTestEffect::TestThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_flStartTime  = gpGlobals->time;
}

//
// CBlood implementation
//
LINK_ENTITY_TO_CLASS( env_blood, CBlood );

#define SF_BLOOD_RANDOM 0x0001
#define SF_BLOOD_STREAM 0x0002
#define SF_BLOOD_PLAYER 0x0004
#define SF_BLOOD_DECAL 0x0008

void CBlood::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects  = 0;
	pev->frame    = 0;
	SetMovedir( pev );
}

void CBlood::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "color" ) )
	{
		int color = atoi( pkvd->szValue );
		switch ( color )
		{
		case 1:
			SetColor( BLOOD_COLOR_YELLOW );
			break;
		default:
			SetColor( BLOOD_COLOR_RED );
			break;
		}

		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "amount" ) )
	{
		SetBloodAmount( atof( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

Vector CBlood::Direction( void )
{
	if ( pev->spawnflags & SF_BLOOD_RANDOM )
		return UTIL_RandomBloodVector();

	return pev->movedir;
}

Vector CBlood::BloodPosition( CBaseEntity *pActivator )
{
	if ( pev->spawnflags & SF_BLOOD_PLAYER )
	{
		edict_t *pPlayer;

		if ( pActivator && pActivator->IsPlayer() )
		{
			pPlayer = pActivator->edict();
		}
		else
			pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );
		if ( pPlayer )
			return ( pPlayer->v.origin + pPlayer->v.view_ofs ) + Vector( RANDOM_FLOAT( -10, 10 ), RANDOM_FLOAT( -10, 10 ), RANDOM_FLOAT( -10, 10 ) );
	}

	return pev->origin;
}

void CBlood::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->spawnflags & SF_BLOOD_STREAM )
		UTIL_BloodStream( BloodPosition( pActivator ), Direction(), ( Color() == BLOOD_COLOR_RED ) ? 70 : Color(), BloodAmount() );
	else
		UTIL_BloodDrips( BloodPosition( pActivator ), Direction(), Color(), BloodAmount() );

	if ( pev->spawnflags & SF_BLOOD_DECAL )
	{
		Vector forward = Direction();
		Vector start   = BloodPosition( pActivator );
		TraceResult tr;

		UTIL_TraceLine( start, start + forward * BloodAmount() * 2, ignore_monsters, NULL, &tr );
		if ( tr.flFraction != 1.0 )
			UTIL_BloodDecalTrace( &tr, Color() );
	}
}

//
// CEnvFunnel implementation
//
LINK_ENTITY_TO_CLASS( env_funnel, CEnvFunnel );

void CEnvFunnel::Precache( void )
{
	m_iSprite = PRECACHE_MODEL( "sprites/flare6.spr" );
}

void CEnvFunnel::Spawn( void )
{
	Precache();
	pev->solid   = SOLID_NOT;
	pev->effects = EF_NODRAW;
}

void CEnvFunnel::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
	WRITE_BYTE( TE_LARGEFUNNEL );
	WRITE_COORD( pev->origin.x );
	WRITE_COORD( pev->origin.y );
	WRITE_COORD( pev->origin.z );
	WRITE_SHORT( m_iSprite );

	if ( pev->spawnflags & SF_FUNNEL_REVERSE ) // funnel flows in reverse?
	{
		WRITE_SHORT( 1 );
	}
	else
	{
		WRITE_SHORT( 0 );
	}

	MESSAGE_END();

	SetThink( &CEnvFunnel::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}
