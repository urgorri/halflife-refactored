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
#ifndef PROJECTILE_BOLT_H
#define PROJECTILE_BOLT_H

#include "core/cbase.h"

#define BOLT_AIR_VELOCITY 2000
#define BOLT_WATER_VELOCITY 1000

class CCrossbowBolt : public CBaseEntity
{
  public:
	void Spawn( void );
	void Precache( void );
	int Classify( void );
	void EXPORT BubbleThink( void );
	void EXPORT BoltTouch( CBaseEntity *pOther );
	void EXPORT ExplodeThink( void );

	int m_iTrail;

	static CCrossbowBolt *BoltCreate( void );
};

#endif // PROJECTILE_BOLT_H
