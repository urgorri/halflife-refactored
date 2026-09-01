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
#ifndef MONSTERS_BIGMOMMA_MORTAR_H
#define MONSTERS_BIGMOMMA_MORTAR_H

#include "core/cbase.h"

extern int gSpitSprite;
extern int gSpitDebrisSprite;
void MortarSpray( const Vector &position, const Vector &direction, int spriteModel, int count );

class CBMortar : public CBaseEntity
{
  public:
	void Spawn( void );

	static CBMortar *Shoot( edict_t *pOwner, Vector vecStart, Vector vecVelocity );
	void Touch( CBaseEntity *pOther );
	void EXPORT Animate( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_maxFrame;
};

#endif // MONSTERS_BIGMOMMA_MORTAR_H
