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
#include "systems/env_spark.h"

TYPEDESCRIPTION CEnvSpark::m_SaveData[] =
    {
        DEFINE_FIELD( CEnvSpark, m_flDelay, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CEnvSpark, CBaseEntity );

LINK_ENTITY_TO_CLASS( env_spark, CEnvSpark );
LINK_ENTITY_TO_CLASS( env_debris, CEnvSpark );

void CEnvSpark::Spawn( void )
{
	SetThink( NULL );
	SetUse( NULL );

	if ( FBitSet( pev->spawnflags, 32 ) ) // Use for on/off
	{
		if ( FBitSet( pev->spawnflags, 64 ) ) // Start on
		{
			SetThink( &CEnvSpark::SparkThink ); // start sparking
			SetUse( &CEnvSpark::SparkStop );    // set up +USE to stop sparking
		}
		else
		{
			SetUse( &CEnvSpark::SparkStart );
		}
	}
	else
	{
		SetThink( &CEnvSpark::SparkThink );
	}

	pev->nextthink = gpGlobals->time + ( 0.1 + RANDOM_FLOAT( 0, 1.5 ) );

	if ( m_flDelay <= 0 )
		m_flDelay = 1.5;

	Precache();
}

void CEnvSpark::Precache( void )
{
	PRECACHE_SOUND( "buttons/spark1.wav" );
	PRECACHE_SOUND( "buttons/spark2.wav" );
	PRECACHE_SOUND( "buttons/spark3.wav" );
	PRECACHE_SOUND( "buttons/spark4.wav" );
	PRECACHE_SOUND( "buttons/spark5.wav" );
	PRECACHE_SOUND( "buttons/spark6.wav" );
}

void CEnvSpark::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "MaxDelay" ) )
	{
		m_flDelay      = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "style" ) ||
	          FStrEq( pkvd->szKeyName, "height" ) ||
	          FStrEq( pkvd->szKeyName, "killtarget" ) ||
	          FStrEq( pkvd->szKeyName, "value1" ) ||
	          FStrEq( pkvd->szKeyName, "value2" ) ||
	          FStrEq( pkvd->szKeyName, "value3" ) )
	{
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CEnvSpark::SparkThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1 + RANDOM_FLOAT( 0, m_flDelay );
	DoSpark( pev, pev->origin );
}

void CEnvSpark::SparkStart( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse( &CEnvSpark::SparkStop );
	SetThink( &CEnvSpark::SparkThink );
	pev->nextthink = gpGlobals->time + ( 0.1 + RANDOM_FLOAT( 0, m_flDelay ) );
}

void CEnvSpark::SparkStop( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse( &CEnvSpark::SparkStart );
	SetThink( NULL );
}
