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

#ifndef WEAPONS_AMMO_BASE_H
#define WEAPONS_AMMO_BASE_H

#include "weapons/weapon_base.h"

//=========================================================
// Reusable standard player ammo entity definition helper
//=========================================================
#define IMPLEMENT_SIMPLE_AMMO( className, entityName, modelPath, ammoName, giveAmount, maxCarry, soundPath ) \
class className : public CBasePlayerAmmo \
{ \
public: \
	void Spawn( void ) \
	{ \
		Precache(); \
		SET_MODEL( ENT( pev ), modelPath ); \
		CBasePlayerAmmo::Spawn(); \
	} \
	void Precache( void ) \
	{ \
		PRECACHE_MODEL( modelPath ); \
		PRECACHE_SOUND( soundPath ); \
	} \
	BOOL AddAmmo( CBaseEntity *pOther ) \
	{ \
		int iResult = ( pOther->GiveAmmo( giveAmount, ammoName, maxCarry ) != -1 ); \
		if ( iResult ) \
		{ \
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, soundPath, 1, ATTN_NORM ); \
		} \
		return iResult; \
	} \
}; \
LINK_ENTITY_TO_CLASS( entityName, className );

#endif // WEAPONS_AMMO_BASE_H
