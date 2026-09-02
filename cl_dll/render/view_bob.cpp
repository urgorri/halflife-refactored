#include "view_local.h"


// Quakeworld bob code, this fixes jitters in the mutliplayer since the clock (pparams->time) isn't quite linear
float V_CalcBob( struct ref_params_s *pparams )
{
	static double bobtime;
	static float bob;
	float cycle;
	static float lasttime;
	vec3_t vel;

	if ( pparams->onground == -1 ||
	     pparams->time == lasttime )
	{
		// just use old value
		return bob;
	}

	lasttime = pparams->time;

	bobtime += pparams->frametime;
	cycle = bobtime - (int)( bobtime / cl_bobcycle->value ) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;

	if ( cycle < cl_bobup->value )
	{
		cycle = M_PI * cycle / cl_bobup->value;
	}
	else
	{
		cycle = M_PI + M_PI * ( cycle - cl_bobup->value ) / ( 1.0 - cl_bobup->value );
	}

	// bob is proportional to simulated velocity in the xy plane
	// (don't count Z, or jumping messes it up)
	VectorCopy( pparams->simvel, vel );
	vel[2] = 0;

	bob = sqrt( vel[0] * vel[0] + vel[1] * vel[1] ) * cl_bob->value;
	bob = bob * 0.3 + bob * 0.7 * sin( cycle );
	bob = min( bob, 4.0f );
	bob = max( bob, -7.0f );
	return bob;
}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed )
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

typedef struct pitchdrift_s
{
	float pitchvel;
	int nodrift;
	float driftmove;
	double laststop;
} pitchdrift_t;

static pitchdrift_t pd;

void V_StartPitchDrift( void )
{
	if ( pd.laststop == gEngfuncs.GetClientTime() )
	{
		return; // something else is keeping it from drifting
	}

	if ( pd.nodrift || !pd.pitchvel )
	{
		pd.pitchvel  = v_centerspeed->value;
		pd.nodrift   = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift( void )
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift  = 1;
	pd.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
void V_DriftPitch( struct ref_params_s *pparams )
{
	float delta, move;

	if ( gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator )
	{
		pd.driftmove = 0;
		pd.pitchvel  = 0;
		return;
	}

	// don't count small mouse motion
	if ( pd.nodrift )
	{
		if ( v_centermove->value > 0 && !( in_mlook.state & 1 ) )
		{
			// this is for lazy players. if they stopped, looked around and then continued
			// to move the view will be centered automatically if they move more than
			// v_centermove units.

			if ( fabs( pparams->cmd->forwardmove ) < cl_forwardspeed->value )
				pd.driftmove = 0;
			else
				pd.driftmove += pparams->frametime;

			if ( pd.driftmove > v_centermove->value )
			{
				V_StartPitchDrift();
			}
			else
			{
				return; // player didn't move enough
			}
		}

		return; // don't drift view
	}

	delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

	if ( !delta )
	{
		pd.pitchvel = 0;
		return;
	}

	move = pparams->frametime * pd.pitchvel;

	pd.pitchvel *= ( 1.0f + ( pparams->frametime * 0.25f ) ); // get faster by time

	if ( delta > 0 )
	{
		if ( move > delta )
		{
			pd.pitchvel = 0;
			move        = delta;
		}
		pparams->cl_viewangles[PITCH] += move;
	}
	else if ( delta < 0 )
	{
		if ( move > -delta )
		{
			pd.pitchvel = 0;
			move        = -delta;
		}
		pparams->cl_viewangles[PITCH] -= move;
	}
}

/*
==============================================================================
                        VIEW RENDERING
==============================================================================
*/

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle( struct ref_params_s *pparams )
{
	cl_entity_t *viewent;

	viewent = gEngfuncs.GetViewModel();
	if ( !viewent )
		return;

	viewent->angles[YAW]   = pparams->viewangles[YAW] + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
	viewent->angles[ROLL] -= v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;

	// don't apply all of the v_ipitch to prevent normally unseen parts of viewmodel from coming into view.
	viewent->angles[PITCH] -= v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * ( v_ipitch_level.value * 0.5 );
	viewent->angles[YAW] -= v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle( struct ref_params_s *pparams )
{
	pparams->viewangles[ROLL] += v_idlescale * sin( pparams->time * v_iroll_cycle.value ) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin( pparams->time * v_ipitch_cycle.value ) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin( pparams->time * v_iyaw_cycle.value ) * v_iyaw_level.value;
}

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll( struct ref_params_s *pparams )
{
	float side;
	cl_entity_t *viewentity;

	viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
	if ( !viewentity )
		return;

	side = V_CalcRoll( viewentity->angles, pparams->simvel, pparams->movevars->rollangle, pparams->movevars->rollspeed );

	pparams->viewangles[ROLL] += side;

	if ( pparams->health <= 0 && ( pparams->viewheight[2] != 0 ) )
	{
		// only roll the view if the player is dead and the viewheight[2] is nonzero
		// this is so deadcam in multiplayer will work.
		pparams->viewangles[ROLL] = 80; // dead view angle
		return;
	}
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle( float frametime, float *ev_punchangle )
{
	float len;

	len = VectorNormalize( ev_punchangle );
	len -= ( 10.0 + len * 0.5 ) * frametime;
	len = max( len, 0.0f );
	VectorScale( ev_punchangle, len, ev_punchangle );
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis( int axis, float punch )
{
	ev_punchangle[axis] = punch;
}
