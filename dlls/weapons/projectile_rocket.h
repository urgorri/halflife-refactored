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
#ifndef PROJECTILE_ROCKET_H
#define PROJECTILE_ROCKET_H

#include "weapons/projectile_grenade.h"

class CRpg;

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int ObjectCaps( void ) { return FCAP_DONT_SAVE; }

  public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );

	static CLaserSpot *CreateSpot( void );
};

class CRpgRocket : public CGrenade
{
  public:
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
	void Spawn( void );
	void Precache( void );
	void EXPORT FollowThink( void );
	void EXPORT IgniteThink( void );
	void EXPORT RocketTouch( CBaseEntity *pOther );

	virtual void Explode( TraceResult *pTrace, int bitsDamageType );

	static CRpgRocket *CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher );

	CRpg *GetLauncher();

	int m_iTrail;
	float m_flIgniteTime;

	EHANDLE m_hLauncher; // pointer back to the launcher that fired me
};

#endif // PROJECTILE_ROCKET_H
