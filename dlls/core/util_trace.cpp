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

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/saverestore.h"
#include "core/skill.h"
#include "core/decals.h"
#include "gameplay/gamerules.h"
#include "shake.h"
#include "core/player.h"

extern short g_sModelIndexBloodSpray;
extern short g_sModelIndexBloodDrop;

BOOL UTIL_ShouldShowBlood( int color )
{
	if ( color != DONT_BLEED )
	{
		if ( color == BLOOD_COLOR_RED )
		{
			if ( CVAR_GET_FLOAT( "violence_hblood" ) != 0 )
				return TRUE;
		}
		else
		{
			if ( CVAR_GET_FLOAT( "violence_ablood" ) != 0 )
				return TRUE;
		}
	}
	return FALSE;
}

void UTIL_BloodStream( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
	WRITE_BYTE( TE_BLOODSTREAM );
	WRITE_COORD( origin.x );
	WRITE_COORD( origin.y );
	WRITE_COORD( origin.z );
	WRITE_COORD( direction.x );
	WRITE_COORD( direction.y );
	WRITE_COORD( direction.z );
	WRITE_BYTE( color );
	WRITE_BYTE( min( amount, 255 ) );
	MESSAGE_END();
}

void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	if ( g_pGameRules->IsMultiplayer() )
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if ( amount > 255 )
		amount = 255;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
	WRITE_BYTE( TE_BLOODSPRITE );
	WRITE_COORD( origin.x ); // pos
	WRITE_COORD( origin.y );
	WRITE_COORD( origin.z );
	WRITE_SHORT( g_sModelIndexBloodSpray );         // initial sprite model
	WRITE_SHORT( g_sModelIndexBloodDrop );          // droplet sprite models
	WRITE_BYTE( color );                            // color index into host_basepal
	WRITE_BYTE( min( max( 3, amount / 10 ), 16 ) ); // size
	MESSAGE_END();
}

Vector UTIL_RandomBloodVector( void )
{
	Vector direction;

	direction.x = RANDOM_FLOAT( -1, 1 );
	direction.y = RANDOM_FLOAT( -1, 1 );
	direction.z = RANDOM_FLOAT( 0, 1 );

	return direction;
}

void UTIL_BloodDecalTrace( TraceResult *pTrace, int bloodColor )
{
	if ( UTIL_ShouldShowBlood( bloodColor ) )
	{
		if ( bloodColor == BLOOD_COLOR_RED )
			UTIL_DecalTrace( pTrace, DECAL_BLOOD1 + RANDOM_LONG( 0, 5 ) );
		else
			UTIL_DecalTrace( pTrace, DECAL_YBLOOD1 + RANDOM_LONG( 0, 5 ) );
	}
}

void UTIL_DecalTrace( TraceResult *pTrace, int decalNumber )
{
	short entityIndex;
	int index;
	int message;

	if ( decalNumber < 0 )
		return;

	index = gDecals[decalNumber].index;

	if ( index < 0 )
		return;

	if ( pTrace->flFraction == 1.0 )
		return;

	// Only decal BSP models
	if ( pTrace->pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( pTrace->pHit );
		if ( pEntity && !pEntity->IsBSPModel() )
			return;
		entityIndex = ENTINDEX( pTrace->pHit );
	}
	else
		entityIndex = 0;

	message = TE_DECAL;
	if ( entityIndex != 0 )
	{
		if ( index > 255 )
		{
			message = TE_DECALHIGH;
			index -= 256;
		}
	}
	else
	{
		message = TE_WORLDDECAL;
		if ( index > 255 )
		{
			message = TE_WORLDDECALHIGH;
			index -= 256;
		}
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
	WRITE_BYTE( message );
	WRITE_COORD( pTrace->vecEndPos.x );
	WRITE_COORD( pTrace->vecEndPos.y );
	WRITE_COORD( pTrace->vecEndPos.z );
	WRITE_BYTE( index );
	if ( entityIndex )
		WRITE_SHORT( entityIndex );
	MESSAGE_END();
}

/*
==============
UTIL_PlayerDecalTrace

A player is trying to apply his custom decal for the spray can.
Tell connected clients to display it, or use the default spray can decal
if the custom can't be loaded.
==============
*/
void UTIL_PlayerDecalTrace( TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom )
{
	int index;

	if ( !bIsCustom )
	{
		if ( decalNumber < 0 )
			return;

		index = gDecals[decalNumber].index;
		if ( index < 0 )
			return;
	}
	else
		index = decalNumber;

	if ( pTrace->flFraction == 1.0 )
		return;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
	WRITE_BYTE( TE_PLAYERDECAL );
	WRITE_BYTE( playernum );
	WRITE_COORD( pTrace->vecEndPos.x );
	WRITE_COORD( pTrace->vecEndPos.y );
	WRITE_COORD( pTrace->vecEndPos.z );
	WRITE_SHORT( (short)ENTINDEX( pTrace->pHit ) );
	WRITE_BYTE( index );
	MESSAGE_END();
}

void UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber )
{
	if ( decalNumber < 0 )
		return;

	int index = gDecals[decalNumber].index;
	if ( index < 0 )
		return;

	if ( pTrace->flFraction == 1.0 )
		return;

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pTrace->vecEndPos );
	WRITE_BYTE( TE_GUNSHOTDECAL );
	WRITE_COORD( pTrace->vecEndPos.x );
	WRITE_COORD( pTrace->vecEndPos.y );
	WRITE_COORD( pTrace->vecEndPos.z );
	WRITE_SHORT( (short)ENTINDEX( pTrace->pHit ) );
	WRITE_BYTE( index );
	MESSAGE_END();
}

void UTIL_Sparks( const Vector &position )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
	WRITE_BYTE( TE_SPARKS );
	WRITE_COORD( position.x );
	WRITE_COORD( position.y );
	WRITE_COORD( position.z );
	MESSAGE_END();
}

void UTIL_Ricochet( const Vector &position, float scale )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
	WRITE_BYTE( TE_ARMOR_RICOCHET );
	WRITE_COORD( position.x );
	WRITE_COORD( position.y );
	WRITE_COORD( position.z );
	WRITE_BYTE( (int)( scale * 10 ) );
	MESSAGE_END();
}
