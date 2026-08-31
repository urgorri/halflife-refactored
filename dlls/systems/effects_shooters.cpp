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
#include "ai/monsters.h"
#include "systems/effects_shooters.h"
#include "systems/func_break.h"

TYPEDESCRIPTION CGibShooter::m_SaveData[] =
    {
        DEFINE_FIELD( CGibShooter, m_iGibs, FIELD_INTEGER ),
        DEFINE_FIELD( CGibShooter, m_iGibCapacity, FIELD_INTEGER ),
        DEFINE_FIELD( CGibShooter, m_iGibMaterial, FIELD_INTEGER ),
        DEFINE_FIELD( CGibShooter, m_iGibModelIndex, FIELD_INTEGER ),
        DEFINE_FIELD( CGibShooter, m_flGibVelocity, FIELD_FLOAT ),
        DEFINE_FIELD( CGibShooter, m_flVariance, FIELD_FLOAT ),
        DEFINE_FIELD( CGibShooter, m_flGibLife, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CGibShooter, CBaseDelay );
LINK_ENTITY_TO_CLASS( gibshooter, CGibShooter );

void CGibShooter::Precache( void )
{
	if ( g_Language == LANGUAGE_GERMAN )
	{
		m_iGibModelIndex = PRECACHE_MODEL( "models/germanygibs.mdl" );
	}
	else
	{
		m_iGibModelIndex = PRECACHE_MODEL( "models/hgibs.mdl" );
	}
}

void CGibShooter::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iGibs" ) )
	{
		m_iGibs = m_iGibCapacity = atoi( pkvd->szValue );
		pkvd->fHandled           = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flVelocity" ) )
	{
		m_flGibVelocity = atof( pkvd->szValue );
		pkvd->fHandled  = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flVariance" ) )
	{
		m_flVariance   = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flGibLife" ) )
	{
		m_flGibLife    = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseDelay::KeyValue( pkvd );
	}
}

void CGibShooter::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGibShooter::ShootThink );
	pev->nextthink = gpGlobals->time;
}

void CGibShooter::Spawn( void )
{
	Precache();

	pev->solid   = SOLID_NOT;
	pev->effects = EF_NODRAW;

	if ( m_flDelay == 0 )
	{
		m_flDelay = 0.1;
	}

	if ( m_flGibLife == 0 )
	{
		m_flGibLife = 25;
	}

	SetMovedir( pev );
	pev->body = MODEL_FRAMES( m_iGibModelIndex );
}

CGib *CGibShooter::CreateGib( void )
{
	if ( CVAR_GET_FLOAT( "violence_hgibs" ) == 0 )
		return NULL;

	CGib *pGib = GetClassPtr( (CGib *)NULL );
	pGib->Spawn( "models/hgibs.mdl" );
	pGib->m_bloodColor = BLOOD_COLOR_RED;

	if ( pev->body <= 1 )
	{
		ALERT( at_aiconsole, "GibShooter Body is <= 1!\n" );
	}

	pGib->pev->body = RANDOM_LONG( 1, pev->body - 1 ); // avoid throwing random amounts of the 0th gib. (skull).

	return pGib;
}

void CGibShooter::ShootThink( void )
{
	pev->nextthink = gpGlobals->time + m_flDelay;

	Vector vecShootDir;

	vecShootDir = pev->movedir;

	vecShootDir = vecShootDir + gpGlobals->v_right * RANDOM_FLOAT( -1, 1 ) * m_flVariance;
	vecShootDir = vecShootDir + gpGlobals->v_forward * RANDOM_FLOAT( -1, 1 ) * m_flVariance;
	vecShootDir = vecShootDir + gpGlobals->v_up * RANDOM_FLOAT( -1, 1 ) * m_flVariance;

	vecShootDir = vecShootDir.Normalize();
	CGib *pGib  = CreateGib();

	if ( pGib )
	{
		pGib->pev->origin   = pev->origin;
		pGib->pev->velocity = vecShootDir * m_flGibVelocity;

		pGib->pev->avelocity.x = RANDOM_FLOAT( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT( 100, 300 );

		float thinkTime = pGib->pev->nextthink - gpGlobals->time;

		pGib->m_lifeTime = ( m_flGibLife * RANDOM_FLOAT( 0.95, 1.05 ) ); // +/- 5%
		if ( pGib->m_lifeTime < thinkTime )
		{
			pGib->pev->nextthink = gpGlobals->time + pGib->m_lifeTime;
			pGib->m_lifeTime     = 0;
		}
	}

	if ( --m_iGibs <= 0 )
	{
		if ( pev->spawnflags & SF_GIBSHOOTER_REPEATABLE )
		{
			m_iGibs = m_iGibCapacity;
			SetThink( NULL );
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			SetThink( &CGibShooter::SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}
	}
}

LINK_ENTITY_TO_CLASS( env_shooter, CEnvShooter );

void CEnvShooter::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "shootmodel" ) )
	{
		pev->model     = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "shootsounds" ) )
	{
		int iNoise     = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
		switch ( iNoise )
		{
		case 0:
			m_iGibMaterial = matGlass;
			break;
		case 1:
			m_iGibMaterial = matWood;
			break;
		case 2:
			m_iGibMaterial = matMetal;
			break;
		case 3:
			m_iGibMaterial = matFlesh;
			break;
		case 4:
			m_iGibMaterial = matRocks;
			break;

		default:
		case -1:
			m_iGibMaterial = matNone;
			break;
		}
	}
	else
	{
		CGibShooter::KeyValue( pkvd );
	}
}

void CEnvShooter::Precache( void )
{
	m_iGibModelIndex = PRECACHE_MODEL( (char *)STRING( pev->model ) );
	CBreakable::MaterialSoundPrecache( (Materials)m_iGibMaterial );
}

CGib *CEnvShooter::CreateGib( void )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( STRING( pev->model ) );

	int bodyPart = 0;

	if ( pev->body > 1 )
		bodyPart = RANDOM_LONG( 0, pev->body - 1 );

	pGib->pev->body    = bodyPart;
	pGib->m_bloodColor = DONT_BLEED;
	pGib->m_material   = m_iGibMaterial;

	pGib->pev->rendermode  = pev->rendermode;
	pGib->pev->renderamt   = pev->renderamt;
	pGib->pev->rendercolor = pev->rendercolor;
	pGib->pev->renderfx    = pev->renderfx;
	pGib->pev->scale       = pev->scale;
	pGib->pev->skin        = pev->skin;

	return pGib;
}
