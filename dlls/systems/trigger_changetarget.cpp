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
/*

===== trigger_changetarget.cpp ========================================================

  spawn and use functions for editor-placed triggers

*/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "player.h"
#include "core/saverestore.h"
#include "trains.h" // trigger_camera has train functionality
#include "gameplay/gamerules.h"

class CTriggerChangeTarget : public CBaseDelay
{
  public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int ObjectCaps( void ) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

  private:
	int m_iszNewTarget;
};
LINK_ENTITY_TO_CLASS( trigger_changetarget, CTriggerChangeTarget );

TYPEDESCRIPTION CTriggerChangeTarget::m_SaveData[] =
    {
        DEFINE_FIELD( CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CTriggerChangeTarget, CBaseDelay );

void CTriggerChangeTarget::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iszNewTarget" ) )
	{
		m_iszNewTarget = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CTriggerChangeTarget::Spawn( void )
{
}

void CTriggerChangeTarget::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pTarget = UTIL_FindEntityByString( NULL, "targetname", STRING( pev->target ) );

	if ( pTarget )
	{
		pTarget->pev->target   = m_iszNewTarget;
		CBaseMonster *pMonster = pTarget->MyMonsterPointer();
		if ( pMonster )
		{
			pMonster->m_pGoalEnt = NULL;
		}
	}
}
