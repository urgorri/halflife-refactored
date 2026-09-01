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
#ifndef MONSTERS_TENTACLE_MAW_H
#define MONSTERS_TENTACLE_MAW_H

#include "ai/monsters.h"

class CTentacleMaw : public CBaseMonster
{
  public:
	void Spawn();
	void Precache();
};

#endif // MONSTERS_TENTACLE_MAW_H
