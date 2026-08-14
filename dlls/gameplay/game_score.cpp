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
class CGameScore : public CRulePointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	inline int Points( void ) { return pev->frags; }
	inline BOOL AllowNegativeScore( void ) { return pev->spawnflags & SF_SCORE_NEGATIVE; }
	inline BOOL AwardToTeam( void ) { return pev->spawnflags & SF_SCORE_TEAM; }

	inline void SetPoints( int points ) { pev->frags = points; }

  private:
};

LINK_ENTITY_TO_CLASS( game_score, CGameScore );

void CGameScore::Spawn( void )
{
	CRulePointEntity::Spawn();
}

void CGameScore::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "points" ) )
	{
		SetPoints( atoi( pkvd->szValue ) );
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue( pkvd );
}

void CGameScore::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !CanFireForActivator( pActivator ) )
		return;

	// Only players can use this
	if ( pActivator->IsPlayer() )
	{
		if ( AwardToTeam() )
		{
			pActivator->AddPointsToTeam( Points(), AllowNegativeScore() );
		}
		else
		{
			pActivator->AddPoints( Points(), AllowNegativeScore() );
		}
	}
}
