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
#ifndef CHARGER_BASE_H
#define CHARGER_BASE_H

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/saverestore.h"
#include "core/skill.h"
#include "gameplay/gamerules.h"

class CBaseWallCharger : public CBaseToggle
{
  public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps( void ) { return ( CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE ) & ~FCAP_ACROSS_TRANSITION; }

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT Off( void );
	void EXPORT Recharge( void );

  protected:
	virtual int GetCapacity( void ) const = 0;
	virtual float GetRechargeTime( void ) const = 0;
	virtual BOOL GiveResource( CBaseEntity *pActivator ) = 0;
	virtual const char *GetStartSound( void ) const = 0;
	virtual const char *GetLoopSound( void ) const = 0;
	virtual const char *GetDenySound( void ) const = 0;
	virtual float GetSoundVolume( void ) const { return 1.0f; }

	float m_flNextCharge;
	int m_iReactivate; // DeathMatch delay until reactivated
	int m_iJuice;
	int m_iOn; // 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
};

#endif // CHARGER_BASE_H
