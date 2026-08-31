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
#ifndef GUNTARGET_H
#define GUNTARGET_H

#include "core/cbase.h"
#include "ai/monsters.h"

#define FGUNTARGET_START_ON 0x0001

class CGunTarget : public CBaseMonster
{
  public:
	void Spawn( void );
	void Activate( void );
	void EXPORT Next( void );
	void EXPORT Start( void );
	void EXPORT Wait( void );
	void Stop( void );

	int BloodColor( void ) { return DONT_BLEED; }
	int Classify( void ) { return CLASS_MACHINE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_on;
};

#endif // GUNTARGET_H
