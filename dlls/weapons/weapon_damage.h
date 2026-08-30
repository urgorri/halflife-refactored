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
#ifndef WEAPON_DAMAGE_H
#define WEAPON_DAMAGE_H

#include "weapons/weapon_defs.h"

class CBaseEntity;

typedef struct
{
	CBaseEntity *pEntity;
	float amount;
	int type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

void ClearMultiDamage( void );
void ApplyMultiDamage( entvars_t *pevInflictor, entvars_t *pevAttacker );
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType );

void DecalGunshot( TraceResult *pTrace, int iBulletType );
void SpawnBlood( Vector vecSpot, int bloodColor, float flDamage );
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

#endif // WEAPON_DAMAGE_H
