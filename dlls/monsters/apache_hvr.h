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
#ifndef MONSTERS_APACHE_HVR_H
#define MONSTERS_APACHE_HVR_H

#include "weapons/projectile_grenade.h"

class CApacheHVR : public CGrenade
{
  public:
	void Spawn( void );
	void Precache( void );
	void EXPORT IgniteThink( void );
	void EXPORT AccelerateThink( void );

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_iTrail;
	Vector m_vecForward;
};

#endif // MONSTERS_APACHE_HVR_H
