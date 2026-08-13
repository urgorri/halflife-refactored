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

//	-------------------------------------------
//
//	maprules.cpp
//
//	This module contains entities for implementing/changing game
//	rules dynamically within each map (.BSP)
//
//	-------------------------------------------

#include "core/extdll.h"
#include "eiface.h"
#include "core/util.h"
#include "gameplay/gamerules.h"
#include "gameplay/maprules.h"
#include "core/cbase.h"
#include "player.h"
class CGameCounterSet : public CRulePointEntity
{
public:
	void		Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	inline BOOL RemoveOnFire( void ) { return (pev->spawnflags & SF_GAMECOUNTSET_FIREONCE) ? TRUE : FALSE; }

private:
};

LINK_ENTITY_TO_CLASS( game_counter_set, CGameCounterSet );


void CGameCounterSet::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !CanFireForActivator( pActivator ) )
		return;

	SUB_UseTargets( pActivator, USE_SET, pev->frags );

	if ( RemoveOnFire() )
	{
		UTIL_Remove( this );
	}
}


//
// Flag: USE Only
