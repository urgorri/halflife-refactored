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
#ifndef BUTTONS_H
#define BUTTONS_H

#include "core/cbase.h"
#include "systems/locksound.h"

#define SF_BUTTON_DONTMOVE 1
#define SF_ROTBUTTON_NOTSOLID 1
#define SF_BUTTON_TOGGLE 32       // button stays pushed until reactivated
#define SF_BUTTON_SPARK_IF_OFF 64 // button sparks in OFF state
#define SF_BUTTON_TOUCH_ONLY 256  // button only fires as a result of USE key.

#define SF_MOMENTARY_DOOR 0x0001

char *ButtonSound( int sound ); // get string of button sound number

//
// Generic Button
//
class CBaseButton : public CBaseToggle
{
  public:
	void Spawn( void );
	virtual void Precache( void );
	void RotSpawn( void );
	virtual void KeyValue( KeyValueData *pkvd );

	void ButtonActivate();
	void SparkSoundCache( void );

	void EXPORT ButtonShot( void );
	void EXPORT ButtonTouch( CBaseEntity *pOther );
	void EXPORT ButtonSpark( void );
	void EXPORT TriggerAndWait( void );
	void EXPORT ButtonReturn( void );
	void EXPORT ButtonBackHome( void );
	void EXPORT ButtonUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	enum BUTTON_CODE
	{
		BUTTON_NOTHING,
		BUTTON_ACTIVATE,
		BUTTON_RETURN
	};
	BUTTON_CODE ButtonResponseToTouch( void );

	static TYPEDESCRIPTION m_SaveData[];
	// Buttons that don't take damage can be IMPULSE used
	virtual int ObjectCaps( void ) { return ( CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION ) | ( pev->takedamage ? 0 : FCAP_IMPULSE_USE ); }
	virtual BOOL IsAllowedToSpeak() { return TRUE; }

	BOOL m_fStayPushed; // button stays pushed in until touched again?
	BOOL m_fRotating;   // a rotating button?  default is a sliding button.

	string_t m_strChangeTarget; // if this field is not null, this is an index into the engine string array.
	                            // when this button is touched, it's target entity's TARGET field will be set
	                            // to the button's ChangeTarget. This allows you to make a func_train switch paths, etc.

	locksound_t m_ls; // door lock sounds

	BYTE m_bLockedSound; // ordinals from entity selection
	BYTE m_bLockedSentence;
	BYTE m_bUnlockedSound;
	BYTE m_bUnlockedSentence;
	int m_sounds;
};

class CRotButton : public CBaseButton
{
  public:
	void Spawn( void );
};

class CMomentaryRotButton : public CBaseToggle
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	virtual int ObjectCaps( void )
	{
		int flags = CBaseToggle::ObjectCaps() & ( ~FCAP_ACROSS_TRANSITION );
		if ( pev->spawnflags & SF_MOMENTARY_DOOR )
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Off( void );
	void EXPORT Return( void );
	void UpdateSelf( float value );
	void UpdateSelfReturn( float value );
	void UpdateAllButtons( float value, int start );

	void PlaySound( void );
	void UpdateTarget( float value );

	static CMomentaryRotButton *Instance( edict_t *pent ) { return (CMomentaryRotButton *)GET_PRIVATE( pent ); };
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	int m_lastUsed;
	int m_direction;
	float m_returnSpeed;
	vec3_t m_start;
	vec3_t m_end;
	int m_sounds;
};

#define SF_BTARGET_USE 0x0001
#define SF_BTARGET_ON 0x0002

class CButtonTarget : public CBaseEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int ObjectCaps( void );
};

void DoSpark( entvars_t *pev, const Vector &location );

#endif // BUTTONS_H
