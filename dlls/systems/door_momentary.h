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
#ifndef DOOR_MOMENTARY_H
#define DOOR_MOMENTARY_H

#include "systems/doors.h"

class CMomentaryDoor : public CBaseToggle
{
  public:
	void Spawn( void );
	void Precache( void );

	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT DoorMoveDone( void );

	BYTE m_bMoveSnd; // sound a door makes while moving
};

#endif // DOOR_MOMENTARY_H
