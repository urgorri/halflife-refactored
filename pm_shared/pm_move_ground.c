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
================
PM_CheckVelocity

See if the player has a bogus velocity value.
================
*/
void PM_CheckVelocity()
{
	int i;

	//
	// bound velocity
	//
	for ( i = 0; i < 3; i++ )
	{
		// See if it's bogus.
		if ( IS_NAN( pmove->velocity[i] ) )
		{
			pmove->Con_Printf( "PM  Got a NaN velocity %i\n", i );
			pmove->velocity[i] = 0;
		}
		if ( IS_NAN( pmove->origin[i] ) )
		{
			pmove->Con_Printf( "PM  Got a NaN origin on %i\n", i );
			pmove->origin[i] = 0;
		}

		// Bound it.
		if ( pmove->velocity[i] > pmove->movevars->maxvelocity )
		{
			pmove->Con_DPrintf( "PM  Got a velocity too high on %i\n", i );
			pmove->velocity[i] = pmove->movevars->maxvelocity;
		}
		else if ( pmove->velocity[i] < -pmove->movevars->maxvelocity )
		{
			pmove->Con_DPrintf( "PM  Got a velocity too low on %i\n", i );
			pmove->velocity[i] = -pmove->movevars->maxvelocity;
		}
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags:
0x01 == floor
0x02 == step / wall
==================
*/
int PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce )
{
	float backoff;
	float change;
	float angle;
	int i, blocked;

	angle = normal[2];

	blocked = 0x00;      // Assume unblocked.
	if ( angle > 0 )     // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01; //
	if ( !angle )        // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02; //

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = DotProduct( in, normal ) * overbounce;

	for ( i = 0; i < 3; i++ )
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		// If out velocity is too small, zero it out.
		if ( out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON )
			out[i] = 0;
	}

	// Return blocking flags.
	return blocked;
}

void PM_AddCorrectGravity()
{
	float ent_gravity;

	if ( pmove->waterjumptime )
		return;

	if ( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.
	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * 0.5 * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;

	PM_CheckVelocity();
}

void PM_FixupGravityVelocity()
{
	float ent_gravity;

	if ( pmove->waterjumptime )
		return;

	if ( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Get the correct velocity for the end of the dt
	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * pmove->frametime * 0.5 );

	PM_CheckVelocity();
}

/*
============
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
int PM_FlyMove( void )
{
	int bumpcount, numbumps;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity, original_velocity;
	vec3_t new_velocity;
	int i, j;
	pmtrace_t trace;
	vec3_t end;
	float time_left, allFraction;
	int blocked;

	numbumps = 4; // Bump up to four times

	blocked   = 0;                                    // Assume not blocked
	numplanes = 0;                                    //  and not sliding along any planes
	VectorCopy( pmove->velocity, original_velocity ); // Store original velocity
	VectorCopy( pmove->velocity, primal_velocity );

	allFraction = 0;
	time_left   = pmove->frametime; // Total time for this movement operation.

	for ( bumpcount = 0; bumpcount < numbumps; bumpcount++ )
	{
		if ( !pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2] )
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		for ( i = 0; i < 3; i++ )
			end[i] = pmove->origin[i] + time_left * pmove->velocity[i];

		// See if we can make it from origin to end point.
		trace = pmove->PM_PlayerTrace( pmove->origin, end, PM_NORMAL, -1 );

		allFraction += trace.fraction;
		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if ( trace.allsolid )
		{ // entity is trapped in another solid
			VectorCopy( vec3_origin, pmove->velocity );
			// Con_DPrintf("Trapped 4\n");
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove->origin and
		//  zero the plane counter.
		if ( trace.fraction > 0 )
		{ // actually covered some distance
			VectorCopy( trace.endpos, pmove->origin );
			VectorCopy( pmove->velocity, original_velocity );
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if ( trace.fraction == 1 )
			break; // moved the entire distance

		// if (!trace.ent)
		//	Sys_Error ("PM_PlayerTrace: !trace.ent");

		// Save entity that blocked us (since fraction was < 1.0)
		//  for contact
		// Add it if it's not already in the list!!!
		PM_AddToTouched( trace, pmove->velocity );

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if ( trace.plane.normal[2] > 0.7 )
		{
			blocked |= 1; // floor
		}
		// If the plane has a zero z component in the normal, then it's a
		//  step or wall
		if ( !trace.plane.normal[2] )
		{
			blocked |= 2; // step / wall
			              // Con_DPrintf("Blocked by %i\n", trace.ent);
		}

		// Reduce amount of pmove->frametime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * trace.fraction;

		// Did we run out of planes to clip against?
		if ( numplanes >= MAX_CLIP_PLANES )
		{ // this shouldn't really happen
			//  Stop our movement if so.
			VectorCopy( vec3_origin, pmove->velocity );
			// Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		VectorCopy( trace.plane.normal, planes[numplanes] );
		numplanes++;
		//

		// modify original_velocity so it parallels all of the clip planes
		//
		// relfect player velocity
		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
		if ( numplanes == 1 &&
		     pmove->movetype == MOVETYPE_WALK &&
		     ( ( pmove->onground == -1 ) || ( pmove->friction != 1 ) ) )
		{
			for ( i = 0; i < numplanes; i++ )
			{
				if ( planes[i][2] > 0.7 )
				{ // floor or slope
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
					VectorCopy( new_velocity, original_velocity );
				}
				else
					PM_ClipVelocity( original_velocity, planes[i], new_velocity, 1.0 + pmove->movevars->bounce * ( 1 - pmove->friction ) );
			}

			VectorCopy( new_velocity, pmove->velocity );
			VectorCopy( new_velocity, original_velocity );
		}
		else
		{
			for ( i = 0; i < numplanes; i++ )
			{
				PM_ClipVelocity(
				    original_velocity,
				    planes[i],
				    pmove->velocity,
				    1 );
				for ( j = 0; j < numplanes; j++ )
					if ( j != i )
					{
						// Are we now moving against this plane?
						if ( DotProduct( pmove->velocity, planes[j] ) < 0 )
							break; // not ok
					}
				if ( j == numplanes ) // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if ( i != numplanes )
			{ // go along this plane
				// pmove->velocity is set in clipping call, no need to set again.
				;
			}
			else
			{ // go along the crease
				if ( numplanes != 2 )
				{
					// Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
					VectorCopy( vec3_origin, pmove->velocity );
					// Con_DPrintf("Trapped 4\n");

					break;
				}
				CrossProduct( planes[0], planes[1], dir );
				d = DotProduct( dir, pmove->velocity );
				VectorScale( dir, d, pmove->velocity );
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			if ( DotProduct( pmove->velocity, primal_velocity ) <= 0 )
			{
				// Con_DPrintf("Back\n");
				VectorCopy( vec3_origin, pmove->velocity );
				break;
			}
		}
	}

	if ( allFraction == 0 )
	{
		VectorCopy( vec3_origin, pmove->velocity );
		// Con_DPrintf( "Don't stick\n" );
	}

	return blocked;
}

/*
==============
PM_Accelerate
==============
*/
void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed;

	// Dead player's don't accelerate
	if ( pmove->dead )
		return;

	// If waterjumping, don't accelerate
	if ( pmove->waterjumptime )
		return;

	// See if we are changing direction a bit
	currentspeed = DotProduct( pmove->velocity, wishdir );

	// Reduce wishspeed by the amount of veer.
	addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if ( addspeed <= 0 )
		return;

	// Determine amount of accleration.
	accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;

	// Cap at addspeed
	if ( accelspeed > addspeed )
		accelspeed = addspeed;

	// Adjust velocity.
	for ( i = 0; i < 3; i++ )
	{
		pmove->velocity[i] += accelspeed * wishdir[i];
	}
}

/*
=====================
PM_WalkMove

Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
======================
*/
void PM_WalkMove()
{
	int clip;
	int oldonground;
	int i;

	vec3_t wishvel;
	float spd;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;

	vec3_t dest, start;
	vec3_t original, originalvel;
	vec3_t down, downvel;
	float downdist, updist;

	pmtrace_t trace;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2]   = 0;

	VectorNormalize( pmove->forward ); // Normalize remainder of vectors.
	VectorNormalize( pmove->right );   //

	for ( i = 0; i < 2; i++ ) // Determine x and y parts of velocity
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;

	wishvel[2] = 0; // Zero out z part of velocity

	VectorCopy( wishvel, wishdir ); // Determine maginitude of speed of move
	wishspeed = VectorNormalize( wishdir );

	//
	// Clamp to server defined max speed
	//
	if ( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed / wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}

	// Set pmove velocity
	pmove->velocity[2] = 0;
	PM_Accelerate( wishdir, wishspeed, pmove->movevars->accelerate );
	pmove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	spd = Length( pmove->velocity );

	if ( spd < 1.0f )
	{
		VectorClear( pmove->velocity );
		return;
	}

	// If we are not moving, do nothing
	// if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
	//	return;

	oldonground = pmove->onground;

	// first try just moving to the destination
	dest[0] = pmove->origin[0] + pmove->velocity[0] * pmove->frametime;
	dest[1] = pmove->origin[1] + pmove->velocity[1] * pmove->frametime;
	dest[2] = pmove->origin[2];

	// first try moving directly to the next spot
	VectorCopy( dest, start );
	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, -1 );
	// If we made it all the way, then copy trace end
	//  as new player position.
	if ( trace.fraction == 1 )
	{
		VectorCopy( trace.endpos, pmove->origin );
		return;
	}

	if ( oldonground == -1 && // Don't walk up stairs if not on ground.
	     pmove->waterlevel == 0 )
		return;

	if ( pmove->waterjumptime ) // If we are jumping out of water, don't do anything more.
		return;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	VectorCopy( pmove->origin, original );      // Save out original pos &
	VectorCopy( pmove->velocity, originalvel ); //  velocity.

	// Slide move
	clip = PM_FlyMove();

	// Copy the results out
	VectorCopy( pmove->origin, down );
	VectorCopy( pmove->velocity, downvel );

	// Reset original values.
	VectorCopy( original, pmove->origin );

	VectorCopy( originalvel, pmove->velocity );

	// Start out up one stair height
	VectorCopy( pmove->origin, dest );
	dest[2] += pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, -1 );
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if ( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, pmove->origin );
	}

	// slide move the rest of the way.
	clip = PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	VectorCopy( pmove->origin, dest );
	dest[2] -= pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace( pmove->origin, dest, PM_NORMAL, -1 );

	// If we are not on the ground any more then
	//  use the original movement attempt
	if ( trace.plane.normal[2] < 0.7 )
		goto usedown;
	// If the trace ended up in empty space, copy the end
	//  over to the origin.
	if ( !trace.startsolid && !trace.allsolid )
	{
		VectorCopy( trace.endpos, pmove->origin );
	}
	// Copy this origion to up.
	VectorCopy( pmove->origin, pmove->up );

	// decide which one went farther
	downdist = ( down[0] - original[0] ) * ( down[0] - original[0] ) + ( down[1] - original[1] ) * ( down[1] - original[1] );
	updist   = ( pmove->up[0] - original[0] ) * ( pmove->up[0] - original[0] ) + ( pmove->up[1] - original[1] ) * ( pmove->up[1] - original[1] );

	if ( downdist > updist )
	{
	usedown:
		VectorCopy( down, pmove->origin );
		VectorCopy( downvel, pmove->velocity );
	}
	else // copy z value from slide move
		pmove->velocity[2] = downvel[2];
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction( void )
{
	float *vel;
	float speed, newspeed, control;
	float friction;
	float drop;
	vec3_t newvel;

	// If we are in water jump cycle, don't apply friction
	if ( pmove->waterjumptime )
		return;

	// Get velocity
	vel = pmove->velocity;

	// Calculate speed
	speed = sqrt( vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2] );

	// If too slow, return
	if ( speed < 0.1f )
	{
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pmove->onground != -1 ) // On an entity that is the ground
	{
		vec3_t start, stop;
		pmtrace_t trace;

		start[0] = stop[0] = pmove->origin[0] + vel[0] / speed * 16;
		start[1] = stop[1] = pmove->origin[1] + vel[1] / speed * 16;
		start[2]           = pmove->origin[2] + pmove->player_mins[pmove->usehull][2];
		stop[2]            = start[2] - 34;

		trace = pmove->PM_PlayerTrace( start, stop, PM_NORMAL, -1 );

		if ( trace.fraction == 1.0 )
			friction = pmove->movevars->friction * pmove->movevars->edgefriction;
		else
			friction = pmove->movevars->friction;

		// Grab friction value.
		// friction = pmove->movevars->friction;

		friction *= pmove->friction; // player friction?

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		control = ( speed < pmove->movevars->stopspeed ) ? pmove->movevars->stopspeed : speed;
		// Add the amount to t'he drop amount.
		drop += control * friction * pmove->frametime;
	}

	// apply water friction
	//	if (pmove->waterlevel)
	//		drop += speed * pmove->movevars->waterfriction * waterlevel * pmove->frametime;

	// scale the velocity
	newspeed = speed - drop;
	if ( newspeed < 0 )
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = vel[0] * newspeed;
	newvel[1] = vel[1] * newspeed;
	newvel[2] = vel[2] * newspeed;

	VectorCopy( newvel, pmove->velocity );
}


/*
============
PM_AddGravity

============
*/
void PM_AddGravity()
{
	float ent_gravity;

	if ( pmove->gravity )
		ent_gravity = pmove->gravity;
	else
		ent_gravity = 1.0;

	// Add gravity incorrectly
	pmove->velocity[2] -= ( ent_gravity * pmove->movevars->gravity * pmove->frametime );
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;
	PM_CheckVelocity();
}
