/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"

#include "player.h"
#include "weapons/weapon_base.h"

#include "decals.h"


int g_groupmask = 0;
int g_groupop   = 0;

// Normal overrides
void UTIL_SetGroupTrace( int groupmask, int op )
{
	g_groupmask = groupmask;
	g_groupop   = op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

void UTIL_UnsetGroupTrace( void )
{
	g_groupmask = 0;
	g_groupop   = 0;

	ENGINE_SETGROUPMASK( 0, 0 );
}

// Smart version, it'll clean itself up when it pops off stack
UTIL_GroupTrace::UTIL_GroupTrace( int groupmask, int op )
{
	m_oldgroupmask = g_groupmask;
	m_oldgroupop   = g_groupop;

	g_groupmask = groupmask;
	g_groupop   = op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

UTIL_GroupTrace::~UTIL_GroupTrace( void )
{
	g_groupmask = m_oldgroupmask;
	g_groupop   = m_oldgroupop;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask )
{
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int count;

	count = 0;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free ) // Not in use
			continue;

		if ( flagMask && !( pEdict->v.flags & flagMask ) ) // Does it meet the criteria?
			continue;

		if ( mins.x > pEdict->v.absmax.x ||
		     mins.y > pEdict->v.absmax.y ||
		     mins.z > pEdict->v.absmax.z ||
		     maxs.x < pEdict->v.absmin.x ||
		     maxs.y < pEdict->v.absmin.y ||
		     maxs.z < pEdict->v.absmin.z )
			continue;

		pEntity = CBaseEntity::Instance( pEdict );
		if ( !pEntity )
			continue;

		pList[count] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}

	return count;
}

int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius )
{
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int count;
	float distance, delta;

	count               = 0;
	float radiusSquared = radius * radius;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free ) // Not in use
			continue;

		if ( !( pEdict->v.flags & ( FL_CLIENT | FL_MONSTER ) ) ) // Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - pEdict->v.origin.x; //(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if ( delta > radiusSquared )
			continue;
		distance = delta;

		// Now Y
		delta = center.y - pEdict->v.origin.y; //(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		// Now Z
		delta = center.z - ( pEdict->v.absmin.z + pEdict->v.absmax.z ) * 0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		pEntity = CBaseEntity::Instance( pEdict );
		if ( !pEntity )
			continue;

		pList[count] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}

	return count;
}

CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius )
{
	edict_t *pentEntity;

	if ( pStartEntity )
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_IN_SPHERE( pentEntity, vecCenter, flRadius );

	if ( !FNullEnt( pentEntity ) )
		return CBaseEntity::Instance( pentEntity );
	return NULL;
}

CBaseEntity *UTIL_FindEntityByString( CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue )
{
	edict_t *pentEntity;

	if ( pStartEntity )
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_BY_STRING( pentEntity, szKeyword, szValue );

	if ( !FNullEnt( pentEntity ) )
		return CBaseEntity::Instance( pentEntity );
	return NULL;
}

CBaseEntity *UTIL_FindEntityByClassname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "classname", szName );
}

CBaseEntity *UTIL_FindEntityByTargetname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "targetname", szName );
}

CBaseEntity *UTIL_FindEntityGeneric( const char *szWhatever, Vector &vecSrc, float flRadius )
{
	CBaseEntity *pEntity = NULL;

	pEntity = UTIL_FindEntityByTargetname( NULL, szWhatever );
	if ( pEntity )
		return pEntity;

	CBaseEntity *pSearch = NULL;
	float flMaxDist2     = flRadius * flRadius;
	while ( ( pSearch = UTIL_FindEntityByClassname( pSearch, szWhatever ) ) != NULL )
	{
		float flDist2 = ( pSearch->pev->origin - vecSrc ).Length();
		flDist2       = flDist2 * flDist2;
		if ( flMaxDist2 > flDist2 )
		{
			pEntity    = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
CBaseEntity *UTIL_PlayerByIndex( int playerIndex )
{
	CBaseEntity *pPlayer = NULL;

	if ( playerIndex > 0 && playerIndex <= gpGlobals->maxClients )
	{
		edict_t *pPlayerEdict = INDEXENT( playerIndex );
		if ( pPlayerEdict && !pPlayerEdict->free )
		{
			pPlayer = CBaseEntity::Instance( pPlayerEdict );
		}
	}

	return pPlayer;
}

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, ( igmon == ignore_monsters ? TRUE : FALSE ) | ( ignoreGlass ? 0x100 : 0 ), pentIgnore, ptr );
}

void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, ( igmon == ignore_monsters ? TRUE : FALSE ), pentIgnore, ptr );
}

void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_HULL( vecStart, vecEnd, ( igmon == ignore_monsters ? TRUE : FALSE ), hullNumber, pentIgnore, ptr );
}

void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr )
{
	g_engfuncs.pfnTraceModel( vecStart, vecEnd, hullNumber, pentModel, ptr );
}

TraceResult UTIL_GetGlobalTrace()
{
	TraceResult tr;

	tr.fAllSolid      = gpGlobals->trace_allsolid;
	tr.fStartSolid    = gpGlobals->trace_startsolid;
	tr.fInOpen        = gpGlobals->trace_inopen;
	tr.fInWater       = gpGlobals->trace_inwater;
	tr.flFraction     = gpGlobals->trace_fraction;
	tr.flPlaneDist    = gpGlobals->trace_plane_dist;
	tr.pHit           = gpGlobals->trace_ent;
	tr.vecEndPos      = gpGlobals->trace_endpos;
	tr.vecPlaneNormal = gpGlobals->trace_plane_normal;
	tr.iHitgroup      = gpGlobals->trace_hitgroup;
	return tr;
}

int UTIL_PointContents( const Vector &vec )
{
	return POINT_CONTENTS( vec );
}
