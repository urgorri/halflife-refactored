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
#ifndef CHARGER_SUIT_H
#define CHARGER_SUIT_H

#include "systems/charger_base.h"

class CRecharge : public CBaseWallCharger
{
  protected:
	int GetCapacity( void ) const override { return gSkillData.suitchargerCapacity; }
	float GetRechargeTime( void ) const override { return g_pGameRules->FlHEVChargerRechargeTime(); }
	BOOL GiveResource( CBaseEntity *pActivator ) override
	{
		if ( pActivator->pev->armorvalue < 100 )
		{
			pActivator->pev->armorvalue += 1;
			if ( pActivator->pev->armorvalue > 100 )
				pActivator->pev->armorvalue = 100;
			return TRUE;
		}
		return FALSE;
	}
	const char *GetStartSound( void ) const override { return "items/suitchargeok1.wav"; }
	const char *GetLoopSound( void ) const override { return "items/suitcharge1.wav"; }
	const char *GetDenySound( void ) const override { return "items/suitchargeno1.wav"; }
	float GetSoundVolume( void ) const override { return 0.85f; }
};

#endif // CHARGER_SUIT_H
