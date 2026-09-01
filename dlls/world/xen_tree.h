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
#ifndef XEN_TREE_H
#define XEN_TREE_H

#include "world/xen.h"
#include "ai/monsters.h"

#define TREE_AE_ATTACK 1

class CXenTreeTrigger : public CBaseEntity
{
  public:
	void Touch( CBaseEntity *pOther );
	static CXenTreeTrigger *TriggerCreate( edict_t *pOwner, const Vector &position );
};

class CXenTree : public CActAnimating
{
  public:
	void Spawn( void );
	void Precache( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
	{
		Attack();
		return 0;
	}
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Attack( void );
	int Classify( void ) { return CLASS_BARNACLE; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

  private:
	CXenTreeTrigger *m_pTrigger;
};

#endif // XEN_TREE_H
