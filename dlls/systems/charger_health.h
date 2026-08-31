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
#ifndef CHARGER_HEALTH_H
#define CHARGER_HEALTH_H

#include "systems/charger_base.h"

class CWallHealth : public CBaseWallCharger
{
  protected:
	int GetCapacity( void ) const override { return gSkillData.healthchargerCapacity; }
	float GetRechargeTime( void ) const override { return g_pGameRules->FlHealthChargerRechargeTime(); }
	BOOL GiveResource( CBaseEntity *pActivator ) override { return pActivator->TakeHealth( 1, DMG_GENERIC ); }
	const char *GetStartSound( void ) const override { return "items/medshot4.wav"; }
	const char *GetLoopSound( void ) const override { return "items/medcharge4.wav"; }
	const char *GetDenySound( void ) const override { return "items/medshotno1.wav"; }
};

#endif // CHARGER_HEALTH_H
