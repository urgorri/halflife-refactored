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
#include "weapons/weapon_damage.h"

MULTIDAMAGE gMultiDamage;

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage( void )
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount  = 0;
	gMultiDamage.type    = 0;
}

//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
void ApplyMultiDamage( entvars_t *pevInflictor, entvars_t *pevAttacker )
{
	if ( !gMultiDamage.pEntity )
		return;

	gMultiDamage.pEntity->TakeDamage( pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type );
}

//
// AddMultiDamage - accumulates damage into the multi damage structure
//
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType )
{
	if ( !pEntity )
		return;

	gMultiDamage.type |= bitsDamageType;

	if ( pEntity != gMultiDamage.pEntity )
	{
		ApplyMultiDamage( pevInflictor, pevInflictor );
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount  = 0;
	}

	gMultiDamage.amount += flDamage;
}

/*
================
SpawnBlood
================
*/
void SpawnBlood( Vector vecSpot, int bloodColor, float flDamage )
{
	UTIL_BloodDrips( vecSpot, g_vecAttackDir, bloodColor, (int)flDamage );
}

int DamageDecal( CBaseEntity *pEntity, int bitsDamageType )
{
	if ( !pEntity )
		return ( DECAL_GUNSHOT1 + RANDOM_LONG( 0, 4 ) );

	return pEntity->DamageDecal( bitsDamageType );
}

void DecalGunshot( TraceResult *pTrace, int iBulletType )
{
	if ( !UTIL_IsValidEntity( pTrace->pHit ) )
		return;

	if ( VARS( pTrace->pHit )->solid == SOLID_BSP || VARS( pTrace->pHit )->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		if ( !FNullEnt( pTrace->pHit ) )
			pEntity = CBaseEntity::Instance( pTrace->pHit );

		switch ( iBulletType )
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_MONSTER_12MM:
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_PLAYER_CROWBAR:
			UTIL_DecalTrace( pTrace, DamageDecal( pEntity, DMG_CLUB ) );
			break;
		}
	}
}
