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
#ifndef EFFECTS_ENV_H
#define EFFECTS_ENV_H

#include "core/cbase.h"

// Bubble spawner
class CBubbling : public CBaseEntity
{
  public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );

	void EXPORT FizzThink( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static TYPEDESCRIPTION m_SaveData[];

	int m_density;
	int m_frequency;
	int m_bubbleModel;
	int m_state;
};

// Lightning test effect
class CBeam;
class CTestEffect : public CBaseDelay
{
  public:
	void Spawn( void );
	void Precache( void );
	void EXPORT TestThink( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int m_iLoop;
	int m_iBeam;
	CBeam *m_pBeam[24];
	float m_flBeamTime[24];
	float m_flStartTime;
};

// Blood emitter
class CBlood : public CPointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	inline int Color( void ) { return pev->impulse; }
	inline float BloodAmount( void ) { return pev->dmg; }

	inline void SetColor( int color ) { pev->impulse = color; }
	inline void SetBloodAmount( float amount ) { pev->dmg = amount; }

	Vector Direction( void );
	Vector BloodPosition( CBaseEntity *pActivator );
};

// Particle funnel effect
#define SF_FUNNEL_REVERSE 1

class CEnvFunnel : public CBaseDelay
{
  public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int m_iSprite;
};

#endif // EFFECTS_ENV_H
