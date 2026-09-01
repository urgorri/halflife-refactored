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
#ifndef PLATS_H
#define PLATS_H

#include "core/cbase.h"

#define SF_PLAT_TOGGLE 0x0001

class CBasePlatTrain : public CBaseToggle
{
  public:
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue( KeyValueData *pkvd );
	void Precache( void );

	virtual BOOL IsTogglePlat( void ) { return ( pev->spawnflags & SF_PLAT_TOGGLE ) ? TRUE : FALSE; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	BYTE m_bMoveSnd; // sound a plat makes while moving
	BYTE m_bStopSnd; // sound a plat makes when it stops
	float m_volume;  // Sound volume
};

class CFuncPlat : public CBasePlatTrain
{
  public:
	void Spawn( void );
	void Precache( void );
	void Setup( void );

	virtual void Blocked( CBaseEntity *pOther );

	void EXPORT PlatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void EXPORT CallGoDown( void ) { GoDown(); }
	void EXPORT CallHitTop( void ) { HitTop(); }
	void EXPORT CallHitBottom( void ) { HitBottom(); }

	virtual void GoUp( void );
	virtual void GoDown( void );
	virtual void HitTop( void );
	virtual void HitBottom( void );
};

class CPlatTrigger : public CBaseEntity
{
  public:
	virtual int ObjectCaps( void ) { return ( CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION ) | FCAP_DONT_SAVE; }
	void SpawnInsideTrigger( CFuncPlat *pPlatform );
	void Touch( CBaseEntity *pOther );
	CFuncPlat *m_pPlatform;
};

class CFuncPlatRot : public CFuncPlat
{
  public:
	void Spawn( void );
	void SetupRotation( void );

	virtual void GoUp( void );
	virtual void GoDown( void );
	virtual void HitTop( void );
	virtual void HitBottom( void );

	void RotMove( Vector &destAngle, float time );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	Vector m_end, m_start;
};

#endif // PLATS_H
