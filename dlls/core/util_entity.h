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

#ifndef UTIL_ENTITY_H
#define UTIL_ENTITY_H

#ifdef _WIN32
#pragma once
#endif

class CBaseEntity;

extern CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius );
extern CBaseEntity *UTIL_FindEntityByString( CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue );
extern CBaseEntity *UTIL_FindEntityByClassname( CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity *UTIL_FindEntityByTargetname( CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity *UTIL_FindEntityGeneric( const char *szName, Vector &vecSrc, float flRadius );

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
extern CBaseEntity *UTIL_PlayerByIndex( int playerIndex );

// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
extern int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius );
extern int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask );

typedef enum
{
	ignore_monsters      = 1,
	dont_ignore_monsters = 0,
	missile              = 2
} IGNORE_MONSTERS;
typedef enum
{
	ignore_glass      = 1,
	dont_ignore_glass = 0
} IGNORE_GLASS;
extern void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr );
extern void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr );
typedef enum
{
	point_hull = 0,
	human_hull = 1,
	large_hull = 2,
	head_hull  = 3
};
extern void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr );
extern TraceResult UTIL_GetGlobalTrace( void );
extern void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr );
extern int UTIL_PointContents( const Vector &vec );

extern int g_groupmask;
extern int g_groupop;

class UTIL_GroupTrace
{
  public:
	UTIL_GroupTrace( int groupmask, int op );
	~UTIL_GroupTrace( void );

  private:
	int m_oldgroupmask, m_oldgroupop;
};

void UTIL_SetGroupTrace( int groupmask, int op );
void UTIL_UnsetGroupTrace( void );

#endif // UTIL_ENTITY_H
