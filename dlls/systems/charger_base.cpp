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
#include "systems/charger_base.h"
#include "weapons/weapon_base.h"

TYPEDESCRIPTION CBaseWallCharger::m_SaveData[] =
    {
        DEFINE_FIELD( CBaseWallCharger, m_flNextCharge, FIELD_TIME ),
        DEFINE_FIELD( CBaseWallCharger, m_iReactivate, FIELD_INTEGER ),
        DEFINE_FIELD( CBaseWallCharger, m_iJuice, FIELD_INTEGER ),
        DEFINE_FIELD( CBaseWallCharger, m_iOn, FIELD_INTEGER ),
        DEFINE_FIELD( CBaseWallCharger, m_flSoundTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CBaseWallCharger, CBaseToggle );

void CBaseWallCharger::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "style" ) ||
	     FStrEq( pkvd->szKeyName, "height" ) ||
	     FStrEq( pkvd->szKeyName, "value1" ) ||
	     FStrEq( pkvd->szKeyName, "value2" ) ||
	     FStrEq( pkvd->szKeyName, "value3" ) )
	{
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "dmdelay" ) )
	{
		m_iReactivate  = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseToggle::KeyValue( pkvd );
	}
}

void CBaseWallCharger::Spawn( void )
{
	Precache();

	pev->solid    = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize( pev, pev->mins, pev->maxs );
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	m_iJuice   = GetCapacity();
	pev->frame = 0;
}

void CBaseWallCharger::Precache( void )
{
	if ( GetStartSound() && *GetStartSound() )
		PRECACHE_SOUND( (char *)GetStartSound() );
	if ( GetLoopSound() && *GetLoopSound() )
		PRECACHE_SOUND( (char *)GetLoopSound() );
	if ( GetDenySound() && *GetDenySound() )
		PRECACHE_SOUND( (char *)GetDenySound() );
}

void CBaseWallCharger::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
		return;

	if ( m_iJuice <= 0 )
	{
		pev->frame = 1;
		Off();
	}

	// If player does not have suit, or there is no juice left, play deny sound
	if ( ( m_iJuice <= 0 ) || ( !( pActivator->pev->weapons & ( 1 << WEAPON_SUIT ) ) ) )
	{
		if ( m_flSoundTime <= gpGlobals->time )
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, (char *)GetDenySound(), GetSoundVolume(), ATTN_NORM );
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink( &CBaseWallCharger::Off );

	if ( m_flNextCharge >= gpGlobals->time )
		return;

	// Play start sound or looping sound
	if ( !m_iOn )
	{
		m_iOn++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, (char *)GetStartSound(), GetSoundVolume(), ATTN_NORM );
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ( ( m_iOn == 1 ) && ( m_flSoundTime <= gpGlobals->time ) )
	{
		m_iOn++;
		EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char *)GetLoopSound(), GetSoundVolume(), ATTN_NORM );
	}

	if ( GiveResource( pActivator ) )
	{
		m_iJuice--;
	}

	m_flNextCharge = gpGlobals->time + 0.1;
}

void CBaseWallCharger::Recharge( void )
{
	EMIT_SOUND( ENT( pev ), CHAN_ITEM, (char *)GetStartSound(), GetSoundVolume(), ATTN_NORM );
	m_iJuice   = GetCapacity();
	pev->frame = 0;
	SetThink( &CBaseWallCharger::SUB_DoNothing );
}

void CBaseWallCharger::Off( void )
{
	if ( m_iOn > 1 && GetLoopSound() && *GetLoopSound() )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char *)GetLoopSound() );

	m_iOn = 0;

	if ( ( !m_iJuice ) && ( ( m_iReactivate = (int)GetRechargeTime() ) > 0 ) )
	{
		pev->nextthink = pev->ltime + m_iReactivate;
		SetThink( &CBaseWallCharger::Recharge );
	}
	else
	{
		SetThink( &CBaseWallCharger::SUB_DoNothing );
	}
}
