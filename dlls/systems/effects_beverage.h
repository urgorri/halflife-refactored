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
#ifndef EFFECTS_BEVERAGE_H
#define EFFECTS_BEVERAGE_H

#include "core/cbase.h"

// Beverage dispenser
class CEnvBeverage : public CBaseDelay
{
  public:
	void Spawn( void );
	void Precache( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

// Soda can dispensed from machine
class CItemSoda : public CBaseEntity
{
  public:
	void Spawn( void );
	void Precache( void );
	void EXPORT CanThink( void );
	void EXPORT CanTouch( CBaseEntity *pOther );
};

#endif // EFFECTS_BEVERAGE_H
