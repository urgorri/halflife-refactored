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
#ifndef MONSTERS_CONTROLLER_ENERGY_H
#define MONSTERS_CONTROLLER_ENERGY_H

#include "ai/monsters.h"

//=========================================================
// Controller bouncy ball attack
//=========================================================
class CControllerHeadBall : public CBaseMonster
{
  public:
	void Spawn( void );
	void Precache( void );
	void EXPORT HuntThink( void );
	void EXPORT DieThink( void );
	void EXPORT BounceTouch( CBaseEntity *pOther );
	void MovetoTarget( Vector vecTarget );
	void Crawl( void );
	int m_iTrail;
	int m_flNextAttack;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
};

//=========================================================
// Controller direct zap energy ball
//=========================================================
class CControllerZapBall : public CBaseMonster
{
  public:
	void Spawn( void );
	void Precache( void );
	void EXPORT AnimateThink( void );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );

	EHANDLE m_hOwner;
};

#endif // MONSTERS_CONTROLLER_ENERGY_H
