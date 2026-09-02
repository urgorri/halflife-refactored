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

void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel )
{
	int i;
	float addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if ( pmove->dead )
		return;
	if ( pmove->waterjumptime )
		return;

	// Cap speed
	// wishspd = VectorNormalize (pmove->wishveloc);

	if ( wishspd > 30 )
		wishspd = 30;
	// Determine veer amount
	currentspeed = DotProduct( pmove->velocity, wishdir );
	// See how much to add
	addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if ( addspeed <= 0 )
		return;
	// Determine acceleration speed after acceleration

	accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
	// Cap it
	if ( accelspeed > addspeed )
		accelspeed = addspeed;

	// Adjust pmove vel.
	for ( i = 0; i < 3; i++ )
	{
		pmove->velocity[i] += accelspeed * wishdir[i];
	}
}



/*
===================
PM_AirMove

===================
*/
void PM_AirMove( void )
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2]   = 0;
	// Renormalize
	VectorNormalize( pmove->forward );
	VectorNormalize( pmove->right );

	// Determine x and y parts of velocity
	for ( i = 0; i < 2; i++ )
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	// Zero out z part of velocity
	wishvel[2] = 0;

	// Determine maginitude of speed of move
	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	// Clamp to server defined max speed
	if ( wishspeed > pmove->maxspeed )
	{
		VectorScale( wishvel, pmove->maxspeed / wishspeed, wishvel );
		wishspeed = pmove->maxspeed;
	}

	PM_AirAccelerate( wishdir, wishspeed, pmove->movevars->airaccelerate );

	// Add in any base velocity to the current velocity.
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	PM_FlyMove();
}


/*
===============
PM_SpectatorMove
===============
*/
void PM_SpectatorMove( void )
{
	float speed, drop, friction, control, newspeed;
	// float   accel;
	float currentspeed, addspeed, accelspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	// this routine keeps track of the spectators psoition
	// there a two different main move types : track player or moce freely (OBS_ROAMING)
	// doesn't need excate track position, only to generate PVS, so just copy
	// targets position and real view position is calculated on client (saves server CPU)

	if ( pmove->iuser1 == OBS_ROAMING )
	{

#ifdef CLIENT_DLL
		// jump only in roaming mode
		if ( iJumpSpectator )
		{
			VectorCopy( vJumpOrigin, pmove->origin );
			VectorCopy( vJumpAngles, pmove->angles );
			VectorCopy( vec3_origin, pmove->velocity );
			iJumpSpectator = 0;
			return;
		}
#endif
		// Move around in normal spectator method

		speed = Length( pmove->velocity );
		if ( speed < 1 )
		{
			VectorCopy( vec3_origin, pmove->velocity )
		}
		else
		{
			drop = 0;

			friction = pmove->movevars->friction * 1.5; // extra friction
			control  = speed < pmove->movevars->stopspeed ? pmove->movevars->stopspeed : speed;
			drop += control * friction * pmove->frametime;

			// scale the velocity
			newspeed = speed - drop;
			if ( newspeed < 0 )
				newspeed = 0;
			newspeed /= speed;

			VectorScale( pmove->velocity, newspeed, pmove->velocity );
		}

		// accelerate
		fmove = pmove->cmd.forwardmove;
		smove = pmove->cmd.sidemove;

		VectorNormalize( pmove->forward );
		VectorNormalize( pmove->right );

		for ( i = 0; i < 3; i++ )
		{
			wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
		}
		wishvel[2] += pmove->cmd.upmove;

		VectorCopy( wishvel, wishdir );
		wishspeed = VectorNormalize( wishdir );

		//
		// clamp to server defined max speed
		//
		if ( wishspeed > pmove->movevars->spectatormaxspeed )
		{
			VectorScale( wishvel, pmove->movevars->spectatormaxspeed / wishspeed, wishvel );
			wishspeed = pmove->movevars->spectatormaxspeed;
		}

		currentspeed = DotProduct( pmove->velocity, wishdir );
		addspeed     = wishspeed - currentspeed;
		if ( addspeed <= 0 )
			return;

		accelspeed = pmove->movevars->accelerate * pmove->frametime * wishspeed;
		if ( accelspeed > addspeed )
			accelspeed = addspeed;

		for ( i = 0; i < 3; i++ )
			pmove->velocity[i] += accelspeed * wishdir[i];

		// move
		VectorMA( pmove->origin, pmove->frametime, pmove->velocity, pmove->origin );
	}
	else
	{
		// all other modes just track some kind of target, so spectator PVS = target PVS

		int target;

		// no valid target ?
		if ( pmove->iuser2 <= 0 )
			return;

		// Find the client this player's targeting
		for ( target = 0; target < pmove->numphysent; target++ )
		{
			if ( pmove->physents[target].info == pmove->iuser2 )
				break;
		}

		if ( target == pmove->numphysent )
			return;

		// use targets position as own origin for PVS
		VectorCopy( pmove->physents[target].angles, pmove->angles );
		VectorCopy( pmove->physents[target].origin, pmove->origin );

		// no velocity
		VectorCopy( vec3_origin, pmove->velocity );
	}
}


/*
============
PM_Physics_Toss()

Dead player flying through air., e.g.
============
*/
void PM_Physics_Toss()
{
	pmtrace_t trace;
	vec3_t move;
	float backoff;

	PM_CheckWater();

	if ( pmove->velocity[2] > 0 )
		pmove->onground = -1;

	// If on ground and not moving, return.
	if ( pmove->onground != -1 )
	{
		if ( VectorCompare( pmove->basevelocity, vec3_origin ) &&
		     VectorCompare( pmove->velocity, vec3_origin ) )
			return;
	}

	PM_CheckVelocity();

	// add gravity
	if ( pmove->movetype != MOVETYPE_FLY &&
	     pmove->movetype != MOVETYPE_BOUNCEMISSILE &&
	     pmove->movetype != MOVETYPE_FLYMISSILE )
		PM_AddGravity();

	// move origin
	// Base velocity is not properly accounted for since this entity will move again after the bounce without
	// taking it into account
	VectorAdd( pmove->velocity, pmove->basevelocity, pmove->velocity );

	PM_CheckVelocity();
	VectorScale( pmove->velocity, pmove->frametime, move );
	VectorSubtract( pmove->velocity, pmove->basevelocity, pmove->velocity );

	trace = PM_PushEntity( move ); // Should this clear basevelocity

	PM_CheckVelocity();

	if ( trace.allsolid )
	{
		// entity is trapped in another solid
		pmove->onground = trace.ent;
		VectorCopy( vec3_origin, pmove->velocity );
		return;
	}

	if ( trace.fraction == 1 )
	{
		PM_CheckWater();
		return;
	}

	if ( pmove->movetype == MOVETYPE_BOUNCE )
		backoff = 2.0 - pmove->friction;
	else if ( pmove->movetype == MOVETYPE_BOUNCEMISSILE )
		backoff = 2.0;
	else
		backoff = 1;

	PM_ClipVelocity( pmove->velocity, trace.plane.normal, pmove->velocity, backoff );

	// stop if on ground
	if ( trace.plane.normal[2] > 0.7 )
	{
		float vel;
		vec3_t base;

		VectorClear( base );
		if ( pmove->velocity[2] < pmove->movevars->gravity * pmove->frametime )
		{
			// we're rolling on the ground, add static friction.
			pmove->onground    = trace.ent;
			pmove->velocity[2] = 0;
		}

		vel = DotProduct( pmove->velocity, pmove->velocity );

		// Con_DPrintf("%f %f: %.0f %.0f %.0f\n", vel, trace.fraction, ent->velocity[0], ent->velocity[1], ent->velocity[2] );

		if ( vel < ( 30 * 30 ) || ( pmove->movetype != MOVETYPE_BOUNCE && pmove->movetype != MOVETYPE_BOUNCEMISSILE ) )
		{
			pmove->onground = trace.ent;
			VectorCopy( vec3_origin, pmove->velocity );
		}
		else
		{
			VectorScale( pmove->velocity, ( 1.0 - trace.fraction ) * pmove->frametime * 0.9, move );
			trace = PM_PushEntity( move );
		}
		VectorSubtract( pmove->velocity, base, pmove->velocity )
	}

	// check for in water
	PM_CheckWater();
}

/*
====================
PM_NoClip

====================
*/
void PM_NoClip()
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	//	float		currentspeed, addspeed, accelspeed;

	// Copy movement amounts
	fmove = pmove->cmd.forwardmove;
	smove = pmove->cmd.sidemove;

	VectorNormalize( pmove->forward );
	VectorNormalize( pmove->right );

	for ( i = 0; i < 3; i++ ) // Determine x and y parts of velocity
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	wishvel[2] += pmove->cmd.upmove;

	VectorMA( pmove->origin, pmove->frametime, wishvel, pmove->origin );

	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	//  gravity, etc.
	VectorClear( pmove->velocity );
}

// Only allow bunny jumping up to 1.7x server / player maxspeed setting
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.7f

//-----------------------------------------------------------------------------
// Purpose: Corrects bunny jumping ( where player initiates a bunny jump before other
//  movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
//  running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
//  movement logic does.
//-----------------------------------------------------------------------------
void PM_PreventMegaBunnyJumping( void )
{
	// Current player speed
	float spd;
	// If we have to crop, apply this cropping fraction to velocity
	float fraction;
	// Speed at which bunny jumping is limited
	float maxscaledspeed;

	maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;

	// Don't divide by zero
	if ( maxscaledspeed <= 0.0f )
		return;

	spd = Length( pmove->velocity );

	if ( spd <= maxscaledspeed )
		return;

	fraction = ( maxscaledspeed / spd ) * 0.65; // Returns the modifier for the velocity

	VectorScale( pmove->velocity, fraction, pmove->velocity ); // Crop it down!.
}

/*
=============
PM_Jump
=============
*/
void PM_Jump( void )
{
	int i;
	qboolean tfc = false;

	qboolean cansuperjump = false;

	if ( pmove->dead )
	{
		pmove->oldbuttons |= IN_JUMP; // don't jump again until released
		return;
	}

	tfc = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "tfc" ) ) == 1 ? true : false;

	// Spy that's feigning death cannot jump
	if ( tfc &&
	     ( pmove->deadflag == ( DEAD_DISCARDBODY + 1 ) ) )
	{
		return;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if ( pmove->waterjumptime )
	{
		pmove->waterjumptime -= pmove->cmd.msec;
		if ( pmove->waterjumptime < 0 )
		{
			pmove->waterjumptime = 0;
		}
		return;
	}

	// If we are in the water most of the way...
	if ( pmove->waterlevel >= 2 )
	{ // swimming, not jumping
		pmove->onground = -1;

		if ( pmove->watertype == CONTENTS_WATER ) // We move up a certain amount
			pmove->velocity[2] = 100;
		else if ( pmove->watertype == CONTENTS_SLIME )
			pmove->velocity[2] = 80;
		else // LAVA
			pmove->velocity[2] = 50;

		// play swiming sound
		if ( pmove->flSwimTime <= 0 )
		{
			// Don't play sound again for 1 second
			pmove->flSwimTime = 1000;
			switch ( pmove->RandomLong( 0, 3 ) )
			{
			case 0:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 1:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 2:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 3:
				pmove->PM_PlaySound( CHAN_BODY, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			}
		}

		return;
	}

	// No more effect
	if ( pmove->onground == -1 )
	{
		// Flag that we jumped.
		// HACK HACK HACK
		// Remove this when the game .dll no longer does physics code!!!!
		pmove->oldbuttons |= IN_JUMP; // don't jump again until released
		return;                       // in air, so no effect
	}

	if ( pmove->oldbuttons & IN_JUMP )
		return; // don't pogo stick

	// In the air now.
	pmove->onground = -1;

	PM_PreventMegaBunnyJumping();

	if ( tfc )
	{
		pmove->PM_PlaySound( CHAN_BODY, "player/plyrjmp8.wav", 0.5, ATTN_NORM, 0, PITCH_NORM );
	}
	else
	{
		PM_PlayStepSound( PM_MapTextureTypeStepType( pmove->chtexturetype ), 1.0 );
	}

	// See if user can super long jump?
	cansuperjump = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "slj" ) ) == 1 ? true : false;

	// Acclerate upward
	// If we are ducking...
	if ( ( pmove->bInDuck ) || ( pmove->flags & FL_DUCKING ) )
	{
		// Adjust for super long jump module
		// UNDONE -- note this should be based on forward angles, not current velocity.
		if ( cansuperjump &&
		     ( pmove->cmd.buttons & IN_DUCK ) &&
		     ( pmove->flDuckTime > 0 ) &&
		     Length( pmove->velocity ) > 50 )
		{
			pmove->punchangle[0] = -5;

			for ( i = 0; i < 2; i++ )
			{
				pmove->velocity[i] = pmove->forward[i] * PLAYER_LONGJUMP_SPEED * 1.6;
			}

			pmove->velocity[2] = sqrt( 2 * 800 * 56.0 );
		}
		else
		{
			pmove->velocity[2] = sqrt( 2 * 800 * 45.0 );
		}
	}
	else
	{
		pmove->velocity[2] = sqrt( 2 * 800 * 45.0 );
	}

	// Decay it for simulation
	PM_FixupGravityVelocity();

	// Flag that we jumped.
	pmove->oldbuttons |= IN_JUMP; // don't jump again until released
}



void PM_CheckFalling( void )
{
	if ( pmove->onground != -1 &&
	     !pmove->dead &&
	     pmove->flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		float fvol = 0.5;

		if ( pmove->waterlevel > 0 )
		{
		}
		else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{
			// NOTE:  In the original game dll , there were no breaks after these cases, causing the first one to
			// cascade into the second
			// switch ( RandomLong(0,1) )
			//{
			// case 0:
			// pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			// break;
			// case 1:
			pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			//	break;
			//}
			fvol = 1.0;
		}
		else if ( pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2 )
		{
			qboolean tfc = false;
			tfc          = atoi( pmove->PM_Info_ValueForKey( pmove->physinfo, "tfc" ) ) == 1 ? true : false;

			if ( tfc )
			{
				pmove->PM_PlaySound( CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM );
			}

			fvol = 0.85;
		}
		else if ( pmove->flFallVelocity < PLAYER_MIN_BOUNCE_SPEED )
		{
			fvol = 0;
		}

		if ( fvol > 0.0 )
		{
			// Play landing step right away
			pmove->flTimeStepSound = 0;

			PM_UpdateStepSound();

			// play step sound for current texture
			PM_PlayStepSound( PM_MapTextureTypeStepType( pmove->chtexturetype ), fvol );

			// Knock the screen around a little bit, temporary effect
			pmove->punchangle[2] = pmove->flFallVelocity * 0.013; // punch z axis

			if ( pmove->punchangle[0] > 8 )
			{
				pmove->punchangle[0] = 8;
			}
		}
	}

	if ( pmove->onground != -1 )
	{
		pmove->flFallVelocity = 0;
	}
}
