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
#ifndef MONSTERS_GARGANTUA_EFFECTS_H
#define MONSTERS_GARGANTUA_EFFECTS_H

#include "core/cbase.h"

#define GARG_STOMP_SPRITE_NAME "sprites/gargeye1.spr"
#define GARG_STOMP_BUZZ_SOUND "weapons/mine_charge.wav"

void StreakSplash( const Vector &origin, const Vector &direction, int color, int count, int speed, int velocityRange );

// Spiral Effect
class CSpiral : public CBaseEntity
{
  public:
	void Spawn( void );
	void Think( void );
	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	static CSpiral *Create( const Vector &origin, float height, float radius, float duration );
};

// Stomp Effect
class CStomp : public CBaseEntity
{
  public:
	void Spawn( void );
	void Think( void );
	static CStomp *StompCreate( const Vector &origin, const Vector &end, float speed );
};

// Smoker Effect
class CSmoker : public CBaseEntity
{
  public:
	void Spawn( void );
	void Think( void );
};

#endif // MONSTERS_GARGANTUA_EFFECTS_H
