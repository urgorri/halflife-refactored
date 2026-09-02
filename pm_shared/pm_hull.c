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

#include "pm_local.h"
static vec3_t rgv3tStuckTable[54];
static int rgStuckLast[MAX_CLIENTS][2];


/*
================
PM_AddToTouched

Add's the trace result to touch list, if contact is not already in list.
================
*/
qboolean PM_AddToTouched( pmtrace_t tr, vec3_t impactvelocity )
{
	int i;

	for ( i = 0; i < pmove->numtouch; i++ )
	{
		if ( pmove->touchindex[i].ent == tr.ent )
			break;
	}
	if ( i != pmove->numtouch ) // Already in list.
		return false;

	VectorCopy( impactvelocity, tr.deltavelocity );

	if ( pmove->numtouch >= MAX_PHYSENTS )
		pmove->Con_DPrintf( "Too many entities were touched!\n" );

	pmove->touchindex[pmove->numtouch++] = tr;
	return true;
}

/*
=============
PM_CatagorizePosition
=============
*/
void PM_CatagorizePosition( void )
{
	vec3_t point;
	pmtrace_t tr;

	// if the player hull point one unit down is solid, the player
	// is on ground

	// see if standing on something solid

	// Doing this before we move may introduce a potential latency in water detection, but
	// doing it after can get us stuck on the bottom in water if the amount we move up
	// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
	// this several times per frame, so we really need to avoid sticking to the bottom of
	// water on each call, and the converse case will correct itself if called twice.
	PM_CheckWater();

	point[0] = pmove->origin[0];
	point[1] = pmove->origin[1];
	point[2] = pmove->origin[2] - 2;

	if ( pmove->velocity[2] > 180 ) // Shooting up really fast.  Definitely not on ground.
	{
		pmove->onground = -1;
	}
	else
	{
		// Try and move down.
		tr = pmove->PM_PlayerTrace( pmove->origin, point, PM_NORMAL, -1 );
		// If we hit a steep plane, we are not on ground
		if ( tr.plane.normal[2] < 0.7 )
			pmove->onground = -1; // too steep
		else
			pmove->onground = tr.ent; // Otherwise, point to index of ent under us.

		// If we are on something...
		if ( pmove->onground != -1 )
		{
			// Then we are not in water jump sequence
			pmove->waterjumptime = 0;
			// If we could make the move, drop us down that 1 pixel
			if ( pmove->waterlevel < 2 && !tr.startsolid && !tr.allsolid )
				VectorCopy( tr.endpos, pmove->origin );
		}

		// Standing on an entity other than the world
		if ( tr.ent > 0 ) // So signal that we are touching something.
		{
			PM_AddToTouched( tr, pmove->velocity );
		}
	}
}

/*
=================
PM_GetRandomStuckOffsets

When a player is stuck, it's costly to try and unstick them
Grab a test offset for the player based on a passed in index
=================
*/
int PM_GetRandomStuckOffsets( int nIndex, int server, vec3_t offset )
{
	// Last time we did a full
	int idx;
	idx = rgStuckLast[nIndex][server]++;

	VectorCopy( rgv3tStuckTable[idx % 54], offset );

	return ( idx % 54 );
}

void PM_ResetStuckOffsets( int nIndex, int server )
{
	rgStuckLast[nIndex][server] = 0;
}

/*
=================
NudgePosition

If pmove->origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/
#define PM_CHECKSTUCK_MINTIME 0.05 // Don't check again too quickly.

int PM_CheckStuck( void )
{
	vec3_t base;
	vec3_t offset;
	vec3_t test;
	int hitent;
	int idx;
	float fTime;
	int i;
	pmtrace_t traceresult;

	static float rgStuckCheckTime[MAX_CLIENTS][2]; // Last time we did a full

	// If position is okay, exit
	hitent = pmove->PM_TestPlayerPosition( pmove->origin, &traceresult );
	if ( hitent == -1 )
	{
		PM_ResetStuckOffsets( pmove->player_index, pmove->server );
		return 0;
	}

	VectorCopy( pmove->origin, base );

	//
	// Deal with precision error in network and cases where the player can get stuck on level transitions in singleplayer.
	//
	if ( !pmove->server || !pmove->multiplayer )
	{
		// World or BSP model
		if ( ( hitent == 0 ) ||
		     ( pmove->physents[hitent].model != NULL ) )
		{
			int nReps = 0;
			PM_ResetStuckOffsets( pmove->player_index, pmove->server );
			do
			{
				i = PM_GetRandomStuckOffsets( pmove->player_index, pmove->server, offset );

				VectorAdd( base, offset, test );
				if ( pmove->PM_TestPlayerPosition( test, &traceresult ) == -1 )
				{
					PM_ResetStuckOffsets( pmove->player_index, pmove->server );

					VectorCopy( test, pmove->origin );
					return 0;
				}
				nReps++;
			} while ( nReps < 54 );
		}
	}

	if ( pmove->server )
		idx = 0;
	else
		idx = 1;

	fTime = pmove->Sys_FloatTime();
	// Too soon?
	if ( rgStuckCheckTime[pmove->player_index][idx] >=
	     ( fTime - PM_CHECKSTUCK_MINTIME ) )
	{
		return 1;
	}
	rgStuckCheckTime[pmove->player_index][idx] = fTime;

	pmove->PM_StuckTouch( hitent, &traceresult );

	i = PM_GetRandomStuckOffsets( pmove->player_index, pmove->server, offset );

	VectorAdd( base, offset, test );
	if ( ( hitent = pmove->PM_TestPlayerPosition( test, NULL ) ) == -1 )
	{
		// Con_DPrintf("Nudged\n");

		PM_ResetStuckOffsets( pmove->player_index, pmove->server );

		if ( i >= 27 )
			VectorCopy( test, pmove->origin );

		return 0;
	}

	// If player is flailing while stuck in another player ( should never happen ), then see
	//  if we can't "unstick" them forceably.
	if ( pmove->cmd.buttons & ( IN_JUMP | IN_DUCK | IN_ATTACK ) && ( pmove->physents[hitent].player != 0 ) )
	{
		float x, y, z;
		float xystep   = 8.0;
		float zstep    = 18.0;
		float xyminmax = xystep;
		float zminmax  = 4 * zstep;

		for ( z = 0; z <= zminmax; z += zstep )
		{
			for ( x = -xyminmax; x <= xyminmax; x += xystep )
			{
				for ( y = -xyminmax; y <= xyminmax; y += xystep )
				{
					VectorCopy( base, test );
					test[0] += x;
					test[1] += y;
					test[2] += z;

					if ( pmove->PM_TestPlayerPosition( test, NULL ) == -1 )
					{
						VectorCopy( test, pmove->origin );
						return 0;
					}
				}
			}
		}
	}

	// VectorCopy (base, pmove->origin);

	return 1;
}

/*
============
PM_PushEntity

Does not change the entities velocity at all
============
*/
pmtrace_t PM_PushEntity( vec3_t push )
{
	pmtrace_t trace;
	vec3_t end;

	VectorAdd( pmove->origin, push, end );

	trace = pmove->PM_PlayerTrace( pmove->origin, end, PM_NORMAL, -1 );

	VectorCopy( trace.endpos, pmove->origin );

	// So we can run impact function afterwards.
	if ( trace.fraction < 1.0 &&
	     !trace.allsolid )
	{
		PM_AddToTouched( trace, pmove->velocity );
	}

	return trace;
}


void PM_CreateStuckTable( void )
{
	float x, y, z;
	int idx;
	int i;
	float zi[3];

	memset( rgv3tStuckTable, 0, 54 * sizeof( vec3_t ) );

	idx = 0;
	// Little Moves.
	x = y = 0;
	// Z moves
	for ( z = -0.125; z <= 0.125; z += 0.125 )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for ( y = -0.125; y <= 0.125; y += 0.125 )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for ( x = -0.125; x <= 0.125; x += 0.125 )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for ( x = -0.125; x <= 0.125; x += 0.250 )
	{
		for ( y = -0.125; y <= 0.125; y += 0.250 )
		{
			for ( z = -0.125; z <= 0.125; z += 0.250 )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;
	zi[0] = 0.0f;
	zi[1] = 1.0f;
	zi[2] = 6.0f;

	for ( i = 0; i < 3; i++ )
	{
		// Z moves
		z                       = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for ( y = -2.0f; y <= 2.0f; y += 2.0 )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for ( x = -2.0f; x <= 2.0f; x += 2.0f )
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for ( i = 0; i < 3; i++ )
	{
		z = zi[i];

		for ( x = -2.0f; x <= 2.0f; x += 2.0f )
		{
			for ( y = -2.0f; y <= 2.0f; y += 2.0 )
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
}

/*
This modume implements the shared player physics code between any particular game and
the engine.  The same PM_Move routine is built into the game .dll and the client .dll and is
invoked by each side as appropriate.  There should be no distinction, internally, between server
and client.  This will ensure that prediction behaves appropriately.
*/
