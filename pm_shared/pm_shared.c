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

#ifdef CLIENT_DLL
// Spectator Mode
int iJumpSpectator;
#ifndef DISABLE_JUMP_ORIGIN
float vJumpOrigin[3];
float vJumpAngles[3];
#else
extern float vJumpOrigin[3];
extern float vJumpAngles[3];
#endif
#endif

static int pm_shared_initialized = 0;
playermove_t *pmove = NULL;
int g_onladder = 0;

/*
===============
PM_CalcRoll

===============
*/
float PM_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
{
	float sign;
	float side;
	float value;
	vec3_t forward, right, up;

	AngleVectors( angles, forward, right, up );

	side = DotProduct( velocity, right );

	sign = side < 0 ? -1 : 1;

	side = fabs( side );

	value = rollangle;

	if ( side < rollspeed )
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}

	return side * sign;
}

/*
=============
PM_DropPunchAngle

=============
*/
void PM_DropPunchAngle( vec3_t punchangle )
{
	float len;

	len = VectorNormalize( punchangle );
	len -= ( 10.0 + len * 0.5 ) * pmove->frametime;
	len = max( len, 0.0f );
	VectorScale( punchangle, len, punchangle );
}

/*
==============
PM_CheckParamters

==============
*/
void PM_CheckParamters( void )
{
	float spd;
	float maxspeed;
	vec3_t v_angle;

	spd = ( pmove->cmd.forwardmove * pmove->cmd.forwardmove ) +
	      ( pmove->cmd.sidemove * pmove->cmd.sidemove ) +
	      ( pmove->cmd.upmove * pmove->cmd.upmove );
	spd = sqrt( spd );

	maxspeed = pmove->clientmaxspeed; // atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "maxspd" ) );
	if ( maxspeed != 0.0 )
	{
		pmove->maxspeed = min( maxspeed, pmove->maxspeed );
	}

#if !defined( _TFC )
	// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
	//
	// JoshA: Moved this to CheckParamters rather than working on the velocity,
	// as otherwise it affects every integration step incorrectly.
	if ( ( pmove->onground != -1 ) && ( pmove->cmd.buttons & IN_USE ) )
	{
		pmove->maxspeed *= 1.0f / 3.0f;
	}
#endif

	if ( ( spd != 0.0 ) &&
	     ( spd > pmove->maxspeed ) )
	{
		float fRatio = pmove->maxspeed / spd;
		pmove->cmd.forwardmove *= fRatio;
		pmove->cmd.sidemove *= fRatio;
		pmove->cmd.upmove *= fRatio;
	}

	if ( pmove->flags & FL_FROZEN ||
	     pmove->flags & FL_ONTRAIN ||
	     pmove->dead )
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove    = 0;
		pmove->cmd.upmove      = 0;
	}

	PM_DropPunchAngle( pmove->punchangle );

	// Take angles from command.
	if ( !pmove->dead )
	{
		VectorCopy( pmove->cmd.viewangles, v_angle );
		VectorAdd( v_angle, pmove->punchangle, v_angle );

		// Set up view angles.
		pmove->angles[ROLL]  = PM_CalcRoll( v_angle, pmove->velocity, pmove->movevars->rollangle, pmove->movevars->rollspeed ) * 4;
		pmove->angles[PITCH] = v_angle[PITCH];
		pmove->angles[YAW]   = v_angle[YAW];
	}
	else
	{
		VectorCopy( pmove->oldangles, pmove->angles );
	}

	// Set dead player view_offset
	if ( pmove->dead )
	{
		pmove->view_ofs[2] = PM_DEAD_VIEWHEIGHT;
	}

	// Adjust client view angles to match values used on server.
	if ( pmove->angles[YAW] > 180.0f )
	{
		pmove->angles[YAW] -= 360.0f;
	}
}

void PM_ReduceTimers( void )
{
	if ( pmove->flTimeStepSound > 0 )
	{
		pmove->flTimeStepSound -= pmove->cmd.msec;
		if ( pmove->flTimeStepSound < 0 )
		{
			pmove->flTimeStepSound = 0;
		}
	}
	if ( pmove->flDuckTime > 0 )
	{
		pmove->flDuckTime -= pmove->cmd.msec;
		if ( pmove->flDuckTime < 0 )
		{
			pmove->flDuckTime = 0;
		}
	}
	if ( pmove->flSwimTime > 0 )
	{
		pmove->flSwimTime -= pmove->cmd.msec;
		if ( pmove->flSwimTime < 0 )
		{
			pmove->flSwimTime = 0;
		}
	}
}

/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/
void PM_PlayerMove( qboolean server )
{
	physent_t *pLadder = NULL;

	// Are we running server code?
	pmove->server = server;

	// Adjust speeds etc.
	PM_CheckParamters();

	// Assume we don't touch anything
	pmove->numtouch = 0;

	// # of msec to apply movement
	pmove->frametime = pmove->cmd.msec * 0.001;

	PM_ReduceTimers();

	// Convert view angles to vectors
	AngleVectors( pmove->angles, pmove->forward, pmove->right, pmove->up );

	// PM_ShowClipBox();

	// Special handling for spectator and observers. (iuser1 is set if the player's in observer mode)
	if ( pmove->spectator || pmove->iuser1 > 0 )
	{
		PM_SpectatorMove();
		PM_CatagorizePosition();
		return;
	}

	// Always try and unstick us unless we are in NOCLIP mode
	if ( pmove->movetype != MOVETYPE_NOCLIP && pmove->movetype != MOVETYPE_NONE )
	{
		if ( PM_CheckStuck() )
		{
			// Let the user try to duck to get unstuck
			PM_Duck();

			if ( PM_CheckStuck() )
			{
				return; // Can't move, we're stuck
			}
		}
	}

	// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
	PM_CatagorizePosition();

	// Store off the starting water level
	pmove->oldwaterlevel = pmove->waterlevel;

	// If we are not on ground, store off how fast we are moving down
	if ( pmove->onground == -1 )
	{
		pmove->flFallVelocity = -pmove->velocity[2];
	}

	g_onladder = 0;
	// Don't run ladder code if dead or on a train
	if ( !pmove->dead && !( pmove->flags & FL_ONTRAIN ) )
	{
		pLadder = PM_Ladder();
		if ( pLadder )
		{
			g_onladder = 1;
		}
	}

	PM_UpdateStepSound();

	PM_Duck();

	// Don't run ladder code if dead or on a train
	if ( !pmove->dead && !( pmove->flags & FL_ONTRAIN ) )
	{
		if ( pLadder )
		{
			PM_LadderMove( pLadder );
		}
		else if ( pmove->movetype != MOVETYPE_WALK &&
		          pmove->movetype != MOVETYPE_NOCLIP )
		{
			// Clear ladder stuff unless player is noclipping
			//  it will be set immediately again next frame if necessary
			pmove->movetype = MOVETYPE_WALK;
		}
	}

	// Handle movement
	switch ( pmove->movetype )
	{
	default:
		pmove->Con_DPrintf( "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server );
		break;

	case MOVETYPE_NONE:
		break;

	case MOVETYPE_NOCLIP:
		PM_NoClip();
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		PM_Physics_Toss();
		break;

	case MOVETYPE_FLY:

		PM_CheckWater();

		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if ( pmove->cmd.buttons & IN_JUMP )
		{
			if ( !pLadder )
			{
				PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		// Perform the move accounting for any base velocity.
		VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );
		PM_FlyMove();
		VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );
		break;

	case MOVETYPE_WALK:
		if ( !PM_InWater() )
		{
			PM_AddCorrectGravity();
		}

		// If we are leaping out of the water, just update the counters.
		if ( pmove->waterjumptime )
		{
			PM_WaterJump();
			PM_FlyMove();

			// Make sure waterlevel is set correctly
			PM_CheckWater();
			return;
		}

		// If we are swimming in the water, see if we are nudging against a place we can jump up out
		//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
		if ( pmove->waterlevel >= 2 )
		{
			if ( pmove->waterlevel == 2 )
			{
				PM_CheckWaterJump();
			}

			// If we are falling again, then we must not trying to jump out of water any more.
			if ( pmove->velocity[2] < 0 && pmove->waterjumptime )
			{
				pmove->waterjumptime = 0;
			}

			// Was jump button pressed?
			if ( pmove->cmd.buttons & IN_JUMP )
			{
				PM_Jump();
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Perform regular water movement
			PM_WaterMove();

			VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );

			// Get a final position
			PM_CatagorizePosition();
		}
		else

		// Not underwater
		{
			// Was jump button pressed?
			if ( pmove->cmd.buttons & IN_JUMP )
			{
				if ( !pLadder )
				{
					PM_Jump();
				}
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
			//  we don't slow when standing still, relative to the conveyor.
			if ( pmove->onground != -1 )
			{
				pmove->velocity[2] = 0.0;
				PM_Friction();
			}

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Are we on ground now
			if ( pmove->onground != -1 )
			{
				PM_WalkMove();
			}
			else
			{
				PM_AirMove(); // Take into account movement when in air.
			}

			// Set final flags.
			PM_CatagorizePosition();

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			//  a conveyor (or maybe another monster?)
			VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Add any remaining gravitational component.
			if ( !PM_InWater() )
			{
				PM_FixupGravityVelocity();
			}

			// If we are on ground, no downward velocity.
			if ( pmove->onground != -1 )
			{
				pmove->velocity[2] = 0;
			}

			// See if we landed on the ground with enough force to play
			//  a landing sound.
			PM_CheckFalling();
		}

		// Did we enter or leave the water?
		PM_PlayWaterSounds();
		break;
	}
}


void PM_Move( struct playermove_s *ppmove, int server )
{
	assert( pm_shared_initialized );

	pmove = ppmove;

	PM_PlayerMove( ( server != 0 ) ? true : false );

	if ( pmove->onground != -1 )
	{
		pmove->flags |= FL_ONGROUND;
	}
	else
	{
		pmove->flags &= ~FL_ONGROUND;
	}

	// In single player, reset friction after each movement to FrictionModifier Triggers work still.
	if ( !pmove->multiplayer && ( pmove->movetype == MOVETYPE_WALK ) )
	{
		pmove->friction = 1.0f;
	}
}

int PM_GetVisEntInfo( int ent )
{
	if ( ent >= 0 && ent <= pmove->numvisent )
	{
		return pmove->visents[ent].info;
	}
	return -1;
}

int PM_GetPhysEntInfo( int ent )
{
	if ( ent >= 0 && ent <= pmove->numphysent )
	{
		return pmove->physents[ent].info;
	}
	return -1;
}

void PM_Init( struct playermove_s *ppmove )
{
	assert( !pm_shared_initialized );

	pmove = ppmove;

	PM_CreateStuckTable();
	PM_InitTextureTypes();

	pm_shared_initialized = 1;
}
