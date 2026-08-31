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
#include "core/player.h"
#include "gameplay/spectator.h"
#include "core/client.h"
#include "gameplay/gamerules.h"

extern cvar_t allow_spectators;

void SpectatorConnect( edict_t *pEntity )
{
	entvars_t *pev          = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

	if ( pPlayer )
		pPlayer->SpectatorConnect();
}

/*
================
SpectatorConnect

A spectator has left the game
================



void SpectatorDisconnect( edict_t *pEntity )
{
	entvars_t *pev          = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

	if ( pPlayer )
		pPlayer->SpectatorDisconnect();
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================



void SpectatorThink( edict_t *pEntity )
{
	entvars_t *pev          = &pEntity->v;
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

	if ( pPlayer )
		pPlayer->SpectatorThink();
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
