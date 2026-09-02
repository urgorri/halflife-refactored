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
/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove( void )
{
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	vec3_t start, dest;
	vec3_t temp;
	pmtrace_t trace;

	float speed, newspeed, addspeed, accelspeed;

	//
	// user intentions
	//
	for ( i = 0; i < 3; i++ )
		wishvel[i] = pmove->forward[i] * pmove->cmd.forwardmove + pmove->right[i] * pmove->cmd.sidemove;

	// Sinking after no other movement occurs
	if ( !pmove->cmd.forwardmove && !pmove->cmd.sidemove && !pmove->cmd.upmove )
		wishvel[2] -= 60; // drift towards bottom
	else                  // Go straight up by upmove amount.
		wishvel[2] += pmove->cmd.upmove;

	// Copy it over and determine speed
	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	// Cap speed.
	if ( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed / wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}
	// Slow us down a bit.
	wishspeed *= 0.8;

	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );
	// Water friction
	VectorCopy( pmove->velocity, temp );
	speed = VectorNormalize( temp );
	if ( speed )
	{
		newspeed = speed - pmove->frametime * speed * pmove->movevars->friction * pmove->friction;

		if ( newspeed < 0 )
			newspeed = 0;
		VectorScale( pmove->velocity, newspeed / speed, pmove->velocity );
	}
	else
		newspeed = 0;

	//
	// water acceleration
	//
	if ( wishspeed < 0.1f )
	{
		return;
	}

	addspeed = wishspeed - newspeed;
	if ( addspeed > 0 )
	{

		VectorNormalize( wishvel );
		accelspeed = pmove->movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
		if ( accelspeed > addspeed )
			accelspeed = addspeed;

		for ( i = 0; i < 3; i++ )
			pmove->velocity[i] += accelspeed * wishvel[i];
	}

	// Now move
	// assume it is a stair or a slope, so press down from stepheight above
	VectorMA( pmove->origin, pmove->frametime, pmove->velocity, dest );
	VectorCopy( dest, start );
	start[2] += pmove->movevars->stepsize + 1;
	trace = pmove->PM_PlayerTrace( start, dest, PM_NORMAL, -1 );
	if ( !trace.startsolid && !trace.allsolid ) // FIXME: check steep slope?
	{                                           // walked up the step, so just keep result and exit
		VectorCopy( trace.endpos, pmove->origin );
		return;
	}

	// Try moving straight along out normal path.
	PM_FlyMove();
}


qboolean PM_InWater( void )
{
	return ( pmove->waterlevel > 1 );
}

/*
=============
PM_CheckWater

Sets pmove->waterlevel and pmove->watertype values.
=============
*/
qboolean PM_CheckWater()
{
	vec3_t point;
	int cont;
	int truecont;
	float height;
	float heightover2;

	// Pick a spot just above the players feet.
	point[0] = pmove->origin[0] + ( pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0] ) * 0.5;
	point[1] = pmove->origin[1] + ( pmove->player_mins[pmove->usehull][1] + pmove->player_maxs[pmove->usehull][1] ) * 0.5;
	point[2] = pmove->origin[2] + pmove->player_mins[pmove->usehull][2] + 1;

	// Assume that we are not in water at all.
	pmove->waterlevel = 0;
	pmove->watertype  = CONTENTS_EMPTY;

	// Grab point contents.
	cont = pmove->PM_PointContents( point, &truecont );
	// Are we under water? (not solid and not empty?)
	if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
	{
		// Set water type
		pmove->watertype = cont;

		// We are at least at level one
		pmove->waterlevel = 1;

		height      = ( pmove->player_mins[pmove->usehull][2] + pmove->player_maxs[pmove->usehull][2] );
		heightover2 = height * 0.5;

		// Now check a point that is at the player hull midpoint.
		point[2] = pmove->origin[2] + heightover2;
		cont     = pmove->PM_PointContents( point, NULL );
		// If that point is also under water...
		if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
		{
			// Set a higher water level.
			pmove->waterlevel = 2;

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = pmove->origin[2] + pmove->view_ofs[2];

			cont = pmove->PM_PointContents( point, NULL );
			if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
				pmove->waterlevel = 3; // In over our eyes
		}

		// Adjust velocity based on water current, if any.
		if ( ( truecont <= CONTENTS_CURRENT_0 ) &&
		     ( truecont >= CONTENTS_CURRENT_DOWN ) )
		{
			// The deeper we are, the stronger the current.
			static vec3_t current_table[] =
			    {
			        { 1, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };

			VectorMA( pmove->basevelocity, 50.0 * pmove->waterlevel, current_table[CONTENTS_CURRENT_0 - truecont], pmove->basevelocity );
		}
	}

	return pmove->waterlevel > 1;
}



void PM_WaterJump( void )
{
	if ( pmove->waterjumptime > 10000 )
	{
		pmove->waterjumptime = 10000;
	}

	if ( !pmove->waterjumptime )
		return;

	pmove->waterjumptime -= pmove->cmd.msec;
	if ( pmove->waterjumptime < 0 ||
	     !pmove->waterlevel )
	{
		pmove->waterjumptime = 0;
		pmove->flags &= ~FL_WATERJUMP;
	}

	pmove->velocity[0] = pmove->movedir[0];
	pmove->velocity[1] = pmove->movedir[1];
}

/*
=============
PM_CheckWaterJump
=============
*/
#define WJ_HEIGHT 8
void PM_CheckWaterJump( void )
{
	vec3_t vecStart, vecEnd;
	vec3_t flatforward;
	vec3_t flatvelocity;
	float curspeed;
	pmtrace_t tr;
	int savehull;

	// Already water jumping.
	if ( pmove->waterjumptime )
		return;

	// Don't hop out if we just jumped in
	if ( pmove->velocity[2] < -180 )
		return; // only hop out if we are moving up

	// See if we are backing up
	flatvelocity[0] = pmove->velocity[0];
	flatvelocity[1] = pmove->velocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize( flatvelocity );

	// see if near an edge
	flatforward[0] = pmove->forward[0];
	flatforward[1] = pmove->forward[1];
	flatforward[2] = 0;
	VectorNormalize( flatforward );

	// Are we backing into water from steps or something?  If so, don't pop forward
	if ( curspeed != 0.0 && ( DotProduct( flatvelocity, flatforward ) < 0.0 ) )
		return;

	VectorCopy( pmove->origin, vecStart );
	vecStart[2] += WJ_HEIGHT;

	VectorMA( vecStart, 24, flatforward, vecEnd );

	// Trace, this trace should use the point sized collision hull
	savehull       = pmove->usehull;
	pmove->usehull = 2;
	tr             = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, -1 );
	if ( tr.fraction < 1.0 && fabs( tr.plane.normal[2] ) < 0.1f ) // Facing a near vertical wall?
	{
		vecStart[2] += pmove->player_maxs[savehull][2] - WJ_HEIGHT;
		VectorMA( vecStart, 24, flatforward, vecEnd );
		VectorMA( vec3_origin, -50, tr.plane.normal, pmove->movedir );

		tr = pmove->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, -1 );
		if ( tr.fraction == 1.0 )
		{
			pmove->waterjumptime = 2000;
			pmove->velocity[2]   = 225;
			pmove->oldbuttons |= IN_JUMP;
			pmove->flags |= FL_WATERJUMP;
		}
	}

	// Reset the collision hull
	pmove->usehull = savehull;
}
