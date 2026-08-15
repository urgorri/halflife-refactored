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
#include "core/player.h"

TYPEDESCRIPTION CRuleEntity::m_SaveData[] =
    {
        DEFINE_FIELD( CRuleEntity, m_iszMaster, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CRuleEntity, CBaseEntity );

void CRuleEntity::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects  = EF_NODRAW;
}

void CRuleEntity::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "master" ) )
	{
		SetMaster( ALLOC_STRING( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

BOOL CRuleEntity::CanFireForActivator( CBaseEntity *pActivator )
{
	if ( m_iszMaster )
	{
		if ( UTIL_IsMasterTriggered( m_iszMaster, pActivator ) )
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

//
// CRulePointEntity -- base class for all rule "point" entities (not brushes)
//
void CRulePointEntity::Spawn( void )
{
	CRuleEntity::Spawn();
	pev->frame = 0;
	pev->model = 0;
}

//
// CRuleBrushEntity -- base class for all rule "brush" entities (not brushes)
// Default behavior is to set up like a trigger, invisible, but keep the model for volume testing
//
void CRuleBrushEntity::Spawn( void )
{
	SET_MODEL( edict(), STRING( pev->model ) );
	CRuleEntity::Spawn();
}
