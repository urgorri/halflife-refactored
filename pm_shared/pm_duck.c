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
==================
PM_SplineFraction

Use for ease-in, ease-out style interpolation (accel/decel)
Used by ducking code.
==================
*/
float PM_SplineFraction( float value, float scale )
{
	float valueSquared;

	value        = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void PM_FixPlayerCrouchStuck( int direction )
{
	int hitent;
	int i;
	vec3_t test;

	hitent = pmove->PM_TestPlayerPosition( pmove->origin, NULL );
	if ( hitent == -1 )
		return;

	VectorCopy( pmove->origin, test );
	for ( i = 0; i < 36; i++ )
	{
		pmove->origin[2] += direction;
		hitent = pmove->PM_TestPlayerPosition( pmove->origin, NULL );
		if ( hitent == -1 )
			return;
	}

	VectorCopy( test, pmove->origin ); // Failed
}

void PM_UnDuck( void )
{
	int i;
	pmtrace_t trace;
	vec3_t newOrigin;

	VectorCopy( pmove->origin, newOrigin );

	if ( pmove->onground != -1 )
	{
		for ( i = 0; i < 3; i++ )
		{
			newOrigin[i] += ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
		}
	}

	trace = pmove->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1 );

	if ( !trace.startsolid )
	{
		pmove->usehull = 0;

		// Oh, no, changing hulls stuck us into something, try unsticking downward first.
		trace = pmove->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1 );
		if ( trace.startsolid )
		{
			// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
			// Con_Printf( "unstick got stuck\n" );
			pmove->usehull = 1;
			return;
		}

		pmove->flags &= ~FL_DUCKING;
		pmove->bInDuck     = false;
		pmove->view_ofs[2] = VEC_VIEW;
		pmove->flDuckTime  = 0;

		VectorCopy( newOrigin, pmove->origin );

		// Recatagorize position since ducking can change origin
		PM_CatagorizePosition();
	}
}

void PM_Duck( void )
{
	int i;
	float time;
	float duckFraction;

	int buttonsChanged = ( pmove->oldbuttons ^ pmove->cmd.buttons ); // These buttons have changed this frame
	int nButtonPressed = buttonsChanged & pmove->cmd.buttons;        // The changed ones still down are "pressed"

	int duckchange  = buttonsChanged & IN_DUCK ? 1 : 0;
	int duckpressed = nButtonPressed & IN_DUCK ? 1 : 0;

	if ( pmove->cmd.buttons & IN_DUCK )
	{
		pmove->oldbuttons |= IN_DUCK;
	}
	else
	{
		pmove->oldbuttons &= ~IN_DUCK;
	}

	// Prevent ducking if the iuser3 variable is set
	if ( pmove->iuser3 || pmove->dead )
	{
		// Try to unduck
		if ( pmove->flags & FL_DUCKING )
		{
			PM_UnDuck();
		}
		return;
	}

	if ( pmove->flags & FL_DUCKING )
	{
		pmove->cmd.forwardmove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.sidemove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.upmove *= PLAYER_DUCKING_MULTIPLIER;
	}

	if ( ( pmove->cmd.buttons & IN_DUCK ) || ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
	{
		if ( pmove->cmd.buttons & IN_DUCK )
		{
			if ( ( nButtonPressed & IN_DUCK ) && !( pmove->flags & FL_DUCKING ) )
			{
				// Use 1 second so super long jump will work
				pmove->flDuckTime = 1000;
				pmove->bInDuck    = true;
			}

			time = max( 0.0, ( 1.0 - (float)pmove->flDuckTime / 1000.0 ) );

			if ( pmove->bInDuck )
			{
				// Finish ducking immediately if duck time is over or not on ground
				if ( ( (float)pmove->flDuckTime / 1000.0 <= ( 1.0 - TIME_TO_DUCK ) ) ||
				     ( pmove->onground == -1 ) )
				{
					pmove->usehull     = 1;
					pmove->view_ofs[2] = VEC_DUCK_VIEW;
					pmove->flags |= FL_DUCKING;
					pmove->bInDuck = false;

					// HACKHACK - Fudge for collision bug - no time to fix this properly
					if ( pmove->onground != -1 )
					{
						for ( i = 0; i < 3; i++ )
						{
							pmove->origin[i] -= ( pmove->player_mins[1][i] - pmove->player_mins[0][i] );
						}
						// See if we are stuck?
						PM_FixPlayerCrouchStuck( STUCK_MOVEUP );

						// Recatagorize position since ducking can change origin
						PM_CatagorizePosition();
					}
				}
				else
				{
					float fMore = ( VEC_DUCK_HULL_MIN - VEC_HULL_MIN );

					// Calc parametric time
					duckFraction       = PM_SplineFraction( time, ( 1.0 / TIME_TO_DUCK ) );
					pmove->view_ofs[2] = ( ( VEC_DUCK_VIEW - fMore ) * duckFraction ) + ( VEC_VIEW * ( 1 - duckFraction ) );
				}
			}
		}
		else
		{
			// Try to unduck
			PM_UnDuck();
		}
	}
}
