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
#ifndef EFFECTS_SCREEN_H
#define EFFECTS_SCREEN_H

#include "core/cbase.h"

// Screen shake
class CShake : public CPointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	inline float Amplitude( void ) { return pev->scale; }
	inline float Frequency( void ) { return pev->dmg_save; }
	inline float Duration( void ) { return pev->dmg_take; }
	inline float Radius( void ) { return pev->dmg; }

	inline void SetAmplitude( float amplitude ) { pev->scale = amplitude; }
	inline void SetFrequency( float frequency ) { pev->dmg_save = frequency; }
	inline void SetDuration( float duration ) { pev->dmg_take = duration; }
	inline void SetRadius( float radius ) { pev->dmg = radius; }
};

// Screen fade
class CFade : public CPointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	inline float Duration( void ) { return pev->dmg_take; }
	inline float HoldTime( void ) { return pev->dmg_save; }

	inline void SetDuration( float duration ) { pev->dmg_take = duration; }
	inline void SetHoldTime( float hold ) { pev->dmg_save = hold; }
};

// HUD / Audio message
class CMessage : public CPointEntity
{
  public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );
};

#endif // EFFECTS_SCREEN_H
