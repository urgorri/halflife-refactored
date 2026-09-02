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
#ifndef XEN_H
#define XEN_H

#include "core/cbase.h"
#include "systems/effects.h"

#define XEN_PLANT_GLOW_SPRITE "sprites/flare3.spr"
#define XEN_PLANT_HIDE_TIME 5

class CActAnimating : public CBaseAnimating
{
  public:
	void SetActivity( Activity act );
	inline Activity GetActivity( void ) { return m_Activity; }

	virtual int ObjectCaps( void ) { return CBaseAnimating::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

  private:
	Activity m_Activity;
};

class CXenPLight : public CActAnimating
{
  public:
	void Spawn( void );
	void Precache( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );

	void LightOn( void );
	void LightOff( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

  private:
	CSprite *m_pGlow;
};

#define SF_HAIR_SYNC 0x0001

class CXenHair : public CActAnimating
{
  public:
	void Spawn( void );
	void Precache( void );
	void Think( void );
};

// Fake collision box for big spores
class CXenHull : public CPointEntity
{
  public:
	static CXenHull *CreateHull( CBaseEntity *source, const Vector &mins, const Vector &maxs, const Vector &offset );
	int Classify( void ) { return CLASS_BARNACLE; }
};

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

class CXenSpore : public CActAnimating
{
  public:
	void Spawn( void );
	void Precache( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
	int Classify( void ) { return CLASS_BARNACLE; }

	static const char *pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
  public:
	void Spawn( void );
};

class CXenSporeMed : public CXenSpore
{
  public:
	void Spawn( void );
};

class CXenSporeLarge : public CXenSpore
{
  public:
	void Spawn( void );

	static const Vector m_hullSizes[];
};

#endif // XEN_H
