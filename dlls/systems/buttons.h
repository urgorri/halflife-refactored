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
#ifndef BUTTONS_H
#define BUTTONS_H

#include "core/cbase.h"

#define SF_BUTTON_DONTMOVE 1
#define SF_ROTBUTTON_NOTSOLID 1
#define SF_BUTTON_TOGGLE 32       // button stays pushed until reactivated
#define SF_BUTTON_SPARK_IF_OFF 64 // button sparks in OFF state
#define SF_BUTTON_TOUCH_ONLY 256  // button only fires as a result of USE key.

class CRotButton : public CBaseButton
{
  public:
	void Spawn( void );
};

class CMomentaryRotButton : public CBaseToggle
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	virtual int ObjectCaps( void )
	{
		int flags = CBaseToggle::ObjectCaps();
		if ( pev->spawnflags & SF_MOMENTARY_ROT_BUTTON_AUTO_RETURN )
			return ( flags | FCAP_CONTINUOUS_USE ) & ~FCAP_ACROSS_TRANSITION;
		return ( flags | FCAP_IMPULSE_USE ) & ~FCAP_ACROSS_TRANSITION;
	}
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Off( void );
	void EXPORT UpdateSelf( void );
	void EXPORT UpdateSelfMove( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	int m_lastUsed;
	int m_direction;
	float m_returnSpeed;
	vec3_t m_start;
	vec3_t m_end;
	vec3_t m_ideal;
};

#define SF_BTARGET_USE 0x0001
#define SF_BTARGET_ON 0x0002

class CButtonTarget : public CBaseEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int ObjectCaps( void );
};

#endif // BUTTONS_H
