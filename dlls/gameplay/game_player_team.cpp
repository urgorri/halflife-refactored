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
class CGamePlayerTeam : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

  private:
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_PTEAM_FIREONCE ) ? TRUE : FALSE; }
	inline BOOL ShouldKillPlayer( void ) { return ( pev->spawnflags & SF_PTEAM_KILL ) ? TRUE : FALSE; }
	inline BOOL ShouldGibPlayer( void ) { return ( pev->spawnflags & SF_PTEAM_GIB ) ? TRUE : FALSE; }

	const char *TargetTeamName( const char *pszTargetName );
};

LINK_ENTITY_TO_CLASS( game_player_team, CGamePlayerTeam );

const char *CGamePlayerTeam::TargetTeamName( const char *pszTargetName )
{
	CBaseEntity *pTeamEntity = NULL;

	while ( ( pTeamEntity = UTIL_FindEntityByTargetname( pTeamEntity, pszTargetName ) ) != NULL )
	{
		if ( FClassnameIs( pTeamEntity->pev, "game_team_master" ) )
			return pTeamEntity->TeamID();
	}

	return NULL;
}

void CGamePlayerTeam::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !CanFireForActivator( pActivator ) )
		return;

	if ( pActivator->IsPlayer() )
	{
		const char *pszTargetTeam = TargetTeamName( STRING( pev->target ) );
		if ( pszTargetTeam )
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pActivator;
			g_pGameRules->ChangePlayerTeam( pPlayer, pszTargetTeam, ShouldKillPlayer(), ShouldGibPlayer() );
		}
	}

	if ( RemoveOnFire() )
	{
		UTIL_Remove( this );
	}
}
