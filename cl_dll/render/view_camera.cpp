#include "view_local.h"

/*
void V_NormalizeAngles( float *angles )
{
    int i;
    // Normalize angles
    for ( i = 0; i < 3; i++ )
    {
        if ( angles[i] > 180.0 )
        {
            angles[i] -= 360.0;
        }
        else if ( angles[i] < -180.0 )
        {
            angles[i] += 360.0;
        }
    }
}

/*
===================
V_InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================

void V_InterpolateAngles( float *start, float *end, float *output, float frac )
{
    int i;
    float ang1, ang2;
    float d;

    V_NormalizeAngles( start );
    V_NormalizeAngles( end );

    for ( i = 0 ; i < 3 ; i++ )
    {
        ang1 = start[i];
        ang2 = end[i];

        d = ang2 - ang1;
        if ( d > 180 )
        {
            d -= 360;
        }
        else if ( d < -180 )
        {
            d += 360;
        }

        output[i] = ang1 + d * frac;
    }

    V_NormalizeAngles( output );
} */


void V_SmoothInterpolateAngles( float *startAngle, float *endAngle, float *finalAngle, float degreesPerSec )
{
	float absd, frac, d, threshhold;

	NormalizeAngles( startAngle );
	NormalizeAngles( endAngle );

	for ( int i = 0; i < 3; i++ )
	{
		d = endAngle[i] - startAngle[i];

		if ( d > 180.0f )
		{
			d -= 360.0f;
		}
		else if ( d < -180.0f )
		{
			d += 360.0f;
		}

		absd = fabs( d );

		if ( absd > 0.01f )
		{
			frac = degreesPerSec * v_frametime;

			threshhold = degreesPerSec / 4;

			if ( absd < threshhold )
			{
				float h = absd / threshhold;
				h *= h;
				frac *= h; // slow down last degrees
			}

			if ( frac > absd )
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if ( d > 0 )
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}
	}

	NormalizeAngles( finalAngle );
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin( float *angles, float *origin, float distance, float *returnvec )
{
	vec3_t vecEnd;
	vec3_t forward;
	vec3_t vecStart;
	pmtrace_t *trace;
	int maxLoops = 8;

	int ignoreent = -1; // first, ignore no entity

	cl_entity_t *ent = NULL;

	// Trace back from the target using the player's view angles
	AngleVectors( angles, forward, NULL, NULL );

	VectorScale( forward, -1, forward );

	VectorCopy( origin, vecStart );

	VectorMA( vecStart, distance, forward, vecEnd );

	while ( maxLoops > 0 )
	{
		trace = gEngfuncs.PM_TraceLine( vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent );

		// WARNING! trace->ent is is the number in physent list not the normal entity number

		if ( trace->ent <= 0 )
			break; // we hit the world or nothing, stop trace

		ent = gEngfuncs.GetEntityByIndex( PM_GetPhysEntInfo( trace->ent ) );

		if ( ent == NULL )
			break;

		// hit non-player solid BSP , stop here
		if ( ent->curstate.solid == SOLID_BSP && !ent->player )
			break;

		// if close enought to end pos, stop, otherwise continue trace
		if ( Distance( trace->endpos, vecEnd ) < 1.0f )
		{
			break;
		}
		else
		{
			ignoreent = trace->ent; // ignore last hit entity
			VectorCopy( trace->endpos, vecStart );
		}

		maxLoops--;
	}

	/*	if ( ent )
	    {
	        gEngfuncs.Con_Printf("Trace loops %i , entity %i, model %s, solid %i\n",(8-maxLoops),ent->curstate.number, ent->model->name , ent->curstate.solid );
	    } */

	VectorMA( trace->endpos, 4, trace->plane.normal, returnvec );

	v_lastDistance = Distance( trace->endpos, origin ); // real distance without offset
}

/*void V_GetDeathCam(cl_entity_t * ent1, cl_entity_t * ent2, float * angle, float * origin)
{
    float newAngle[3]; float newOrigin[3];

    float distance = 168.0f;

    v_lastDistance+= v_frametime * 96.0f;	// move unit per seconds back

    if ( v_resetCamera )
        v_lastDistance = 64.0f;

    if ( distance > v_lastDistance )
        distance = v_lastDistance;

    VectorCopy(ent1->origin, newOrigin);

    if ( ent1->player )
        newOrigin[2]+= 17; // head level of living player

    // get new angle towards second target
    if ( ent2 )
    {
        VectorSubtract( ent2->origin, ent1->origin, newAngle );
        VectorAngles( newAngle, newAngle );
        newAngle[0] = -newAngle[0];
    }
    else
    {
        // if no second target is given, look down to dead player
        newAngle[0] = 90.0f;
        newAngle[1] = 0.0f;
        newAngle[2] = 0;
    }

    // and smooth view
    V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

    V_GetChaseOrigin( angle, newOrigin, distance, origin );

    VectorCopy(angle, v_lastAngles);
}*/

void V_GetSingleTargetCam( cl_entity_t *ent1, float *angle, float *origin )
{
	float newAngle[3];
	float newOrigin[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;

	// see is target is a dead player
	qboolean deadPlayer = ent1->player && ( ent1->curstate.solid == SOLID_NOT );

	float dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

	// go away in final scenes or if player just died
	if ( flags & DRC_FLAG_FINAL )
		distance *= 2.0f;
	else if ( deadPlayer )
		distance *= 1.5f;

	// let v_lastDistance float smoothly away
	v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy( ent1->origin, newOrigin );

	if ( ent1->player )
	{
		if ( deadPlayer )
			newOrigin[2] += 2; // laying on ground
		else
			newOrigin[2] += 17; // head level of living player
	}
	else
		newOrigin[2] += 8; // object, tricky, must be above bomb in CS

	// we have no second target, choose view direction based on
	// show front of primary target
	VectorCopy( ent1->angles, newAngle );

	// show dead players from front, normal players back
	if ( flags & DRC_FLAG_FACEPLAYER )
		newAngle[1] += 180.0f;

	newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

	// if final scene (bomb), show from real high pos
	if ( flags & DRC_FLAG_FINAL )
		newAngle[0] = 22.5f;

	// choose side of object/player
	if ( flags & DRC_FLAG_SIDE )
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

	// HACK, if player is dead don't clip against his dead body, can't check this
	V_GetChaseOrigin( angle, newOrigin, distance, origin );
}

float MaxAngleBetweenAngles( float *a1, float *a2 )
{
	float d, maxd = 0.0f;

	NormalizeAngles( a1 );
	NormalizeAngles( a2 );

	for ( int i = 0; i < 3; i++ )
	{
		d = a2[i] - a1[i];
		if ( d > 180 )
		{
			d -= 360;
		}
		else if ( d < -180 )
		{
			d += 360;
		}

		d = fabs( d );

		if ( d > maxd )
			maxd = d;
	}

	return maxd;
}

void V_GetDoubleTargetsCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin )
{
	float newAngle[3];
	float newOrigin[3];
	float tempVec[3];

	int flags = gHUD.m_Spectator.m_iObserverFlags;

	float dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

	// go away in final scenes or if player just died
	if ( flags & DRC_FLAG_FINAL )
		distance *= 2.0f;

	// let v_lastDistance float smoothly away
	v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy( ent1->origin, newOrigin );

	if ( ent1->player )
		newOrigin[2] += 17; // head level of living player
	else
		newOrigin[2] += 8; // object, tricky, must be above bomb in CS

	// get new angle towards second target
	VectorSubtract( ent2->origin, ent1->origin, newAngle );

	VectorAngles( newAngle, newAngle );
	newAngle[0] = -newAngle[0];

	// set angle diffrent in Dramtaic scenes
	newAngle[0] += 12.5f * dfactor; // lower angle if dramatic

	if ( flags & DRC_FLAG_SIDE )
		newAngle[1] += 22.5f;
	else
		newAngle[1] -= 22.5f;

	float d = MaxAngleBetweenAngles( v_lastAngles, newAngle );

	if ( ( d < v_cameraFocusAngle ) && ( v_cameraMode == CAM_MODE_RELAX ) )
	{
		// difference is to small and we are in relax camera mode, keep viewangles
		VectorCopy( v_lastAngles, newAngle );
	}
	else if ( ( d < v_cameraRelaxAngle ) && ( v_cameraMode == CAM_MODE_FOCUS ) )
	{
		// we catched up with our target, relax again
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		// target move too far away, focus camera again
		v_cameraMode = CAM_MODE_FOCUS;
	}

	// and smooth view, if not a scene cut
	if ( v_resetCamera || ( v_cameraMode == CAM_MODE_RELAX ) )
	{
		VectorCopy( newAngle, angle );
	}
	else
	{
		V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 180.0f );
	}

	V_GetChaseOrigin( newAngle, newOrigin, distance, origin );

	// move position up, if very close at target
	if ( v_lastDistance < 64.0f )
		origin[2] += 16.0f * ( 1.0f - ( v_lastDistance / 64.0f ) );

	// calculate angle to second target
	VectorSubtract( ent2->origin, origin, tempVec );
	VectorAngles( tempVec, tempVec );
	tempVec[0] = -tempVec[0];

	/* take middle between two viewangles
	InterpolateAngles( newAngle, tempVec, newAngle, 0.5f); */
}

void V_GetDirectedChasePosition( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin )
{

	if ( v_resetCamera )
	{
		v_lastDistance = 4096.0f;
		// v_cameraMode = CAM_MODE_FOCUS;
	}

	if ( ( ent2 == (cl_entity_t *)0xFFFFFFFF ) || ( ent1->player && ( ent1->curstate.solid == SOLID_NOT ) ) )
	{
		// we have no second target or player just died
		V_GetSingleTargetCam( ent1, angle, origin );
	}
	else if ( ent2 )
	{
		// keep both target in view
		V_GetDoubleTargetsCam( ent1, ent2, angle, origin );
	}
	else
	{
		// second target disappeard somehow (dead)

		// keep last good viewangle
		float newOrigin[3];

		int flags = gHUD.m_Spectator.m_iObserverFlags;

		float dfactor = ( flags & DRC_FLAG_DRAMATIC ) ? -1.0f : 1.0f;

		float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;

		// go away in final scenes or if player just died
		if ( flags & DRC_FLAG_FINAL )
			distance *= 2.0f;

		// let v_lastDistance float smoothly away
		v_lastDistance += v_frametime * 32.0f; // move unit per seconds back

		if ( distance > v_lastDistance )
			distance = v_lastDistance;

		VectorCopy( ent1->origin, newOrigin );

		if ( ent1->player )
			newOrigin[2] += 17; // head level of living player
		else
			newOrigin[2] += 8; // object, tricky, must be above bomb in CS

		V_GetChaseOrigin( angle, newOrigin, distance, origin );
	}

	VectorCopy( angle, v_lastAngles );
}

void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles )
{
	cl_entity_t *ent = NULL;

	if ( target )
	{
		ent = gEngfuncs.GetEntityByIndex( target );
	};

	if ( !ent )
	{
		// just copy a save in-map position
		VectorCopy( vJumpAngles, angles );
		VectorCopy( vJumpOrigin, origin );
		return;
	}

	if ( gHUD.m_Spectator.m_autoDirector->value )
	{
		if ( g_iUser3 )
			V_GetDirectedChasePosition( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ), angles, origin );
		else
			V_GetDirectedChasePosition( ent, (cl_entity_t *)0xFFFFFFFF, angles, origin );
	}
	else
	{
		if ( cl_angles == NULL ) // no mouse angles given, use entity angles ( locked mode )
		{
			VectorCopy( ent->angles, angles );
			angles[0] *= -1;
		}
		else
			VectorCopy( cl_angles, angles );

		VectorCopy( ent->origin, origin );

		origin[2] += 28; // DEFAULT_VIEWHEIGHT - some offset

		V_GetChaseOrigin( angles, origin, cl_chasedist->value, origin );
	}

	v_resetCamera = false;
}

void V_ResetChaseCam()
{
	v_resetCamera = true;
}

void V_GetInEyePos( int target, float *origin, float *angles )
{
	if ( !target )
	{
		// just copy a save in-map position
		VectorCopy( vJumpAngles, angles );
		VectorCopy( vJumpOrigin, origin );
		return;
	};

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( target );

	if ( !ent )
		return;

	VectorCopy( ent->origin, origin );
	VectorCopy( ent->angles, angles );

	angles[PITCH] *= -3.0f; // see CL_ProcessEntityUpdate()

	if ( ent->curstate.solid == SOLID_NOT )
	{
		angles[ROLL] = 80; // dead view angle
		origin[2] += -8;   // PM_DEAD_VIEWHEIGHT
	}
	else if ( ent->curstate.usehull == 1 )
		origin[2] += 12; // VEC_DUCK_VIEW;
	else
		// exacty eye position can't be caluculated since it depends on
		// client values like cl_bobcycle, this offset matches the default values
		origin[2] += 28; // DEFAULT_VIEWHEIGHT
}

void V_GetMapFreePosition( float *cl_angles, float *origin, float *angles )
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy( cl_angles, angles );

	// modify angles since we don't wanna see map's bottom
	angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * ( ( 90.0f - angles[0] ) / 90.0f );

	AngleVectors( angles, forward, NULL, NULL );

	VectorNormalize( forward );

	VectorMA( zScaledTarget, -( 4096.0f / gHUD.m_Spectator.m_mapZoom ), forward, origin );
}

void V_GetMapChasePosition( int target, float *cl_angles, float *origin, float *angles )
{
	vec3_t forward;

	if ( target )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( target );

		if ( gHUD.m_Spectator.m_autoDirector->value )
		{
			// this is done to get the angles made by director mode
			V_GetChasePos( target, cl_angles, origin, angles );
			VectorCopy( ent->origin, origin );

			// keep fix chase angle horizontal
			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy( cl_angles, angles );
			VectorCopy( ent->origin, origin );

			// modify angles since we don't wanna see map's bottom
			angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
		}
	}
	else
	{
		// keep out roaming position, but modify angles
		VectorCopy( cl_angles, angles );
		angles[0] = 51.25f + 38.75f * ( angles[0] / 90.0f );
	}

	origin[2] *= ( ( 90.0f - angles[0] ) / 90.0f );
	angles[2] = 0.0f; // don't roll angle (if chased player is dead)

	AngleVectors( angles, forward, NULL, NULL );

	VectorNormalize( forward );

	VectorMA( origin, -1536, forward, origin );
}


/*
==================
V_CalcSpectatorRefdef

==================
*/
void V_CalcSpectatorRefdef( struct ref_params_s *pparams )
{
	static vec3_t velocity( 0.0f, 0.0f, 0.0f );

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex   = 0;

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex( g_iUser2 );

	pparams->onlyClientDraw = false;

	// refresh position
	VectorCopy( pparams->simorg, v_sim_org );

	// get old values
	VectorCopy( pparams->cl_viewangles, v_cl_angles );
	VectorCopy( pparams->viewangles, v_angles );
	VectorCopy( pparams->vieworg, v_origin );

	if ( ( g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE ) && ent )
	{
		// calculate player velocity
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if ( timeDiff > 0 )
		{
			vec3_t distance;
			VectorSubtract( ent->prevstate.origin, ent->curstate.origin, distance );
			VectorScale( distance, 1 / timeDiff, distance );

			velocity[0] = velocity[0] * 0.9f + distance[0] * 0.1f;
			velocity[1] = velocity[1] * 0.9f + distance[1] * 0.1f;
			velocity[2] = velocity[2] * 0.9f + distance[2] * 0.1f;

			VectorCopy( velocity, pparams->simvel );
		}

		// predict missing client data and set weapon model ( in HLTV mode or inset in eye mode )
#ifdef _TFC
		if ( gEngfuncs.IsSpectateOnly() || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE )
#else
		if ( gEngfuncs.IsSpectateOnly() )
#endif
		{
			V_GetInEyePos( g_iUser2, pparams->simorg, pparams->cl_viewangles );

			pparams->health = 1;

			cl_entity_t *gunModel = gEngfuncs.GetViewModel();

			if ( lastWeaponModelIndex != ent->curstate.weaponmodel )
			{
				// weapon model changed

				lastWeaponModelIndex = ent->curstate.weaponmodel;
				lastViewModelIndex   = V_FindViewModelByWeaponModel( lastWeaponModelIndex );
				if ( lastViewModelIndex )
				{
					gEngfuncs.pfnWeaponAnim( 0, 0 ); // reset weapon animation
				}
				else
				{
					// model not found
					gunModel->model      = NULL; // disable weapon model
					lastWeaponModelIndex = lastViewModelIndex = 0;
				}
			}

			if ( lastViewModelIndex )
			{
				gunModel->model               = IEngineStudio.GetModelByIndex( lastViewModelIndex );
				gunModel->curstate.modelindex = lastViewModelIndex;
				gunModel->curstate.frame      = 0;
				gunModel->curstate.colormap   = 0;
				gunModel->index               = g_iUser2;
			}
			else
			{
				gunModel->model = NULL; // disable weaopn model
			}
		}
		else
		{
			// only get viewangles from entity
			VectorCopy( ent->angles, pparams->cl_viewangles );
			pparams->cl_viewangles[PITCH] *= -3.0f; // see CL_ProcessEntityUpdate()
		}
	}

	v_frametime = pparams->frametime;

	if ( pparams->nextView == 0 )
	{
		// first renderer cycle, full screen

		switch ( g_iUser1 )
		{
		case OBS_CHASE_LOCKED:
			V_GetChasePos( g_iUser2, NULL, v_origin, v_angles );
			break;

		case OBS_CHASE_FREE:
			V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
			break;

		case OBS_ROAMING:
			VectorCopy( v_cl_angles, v_angles );
			VectorCopy( v_sim_org, v_origin );

			// override values if director is active
			gHUD.m_Spectator.GetDirectorCamera( v_origin, v_angles );
			break;

		case OBS_IN_EYE:
			V_CalcNormalRefdef( pparams );
			break;

		case OBS_MAP_FREE:
			pparams->onlyClientDraw = true;
			V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
			break;

		case OBS_MAP_CHASE:
			pparams->onlyClientDraw = true;
			V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );
			break;
		}

		if ( gHUD.m_Spectator.m_pip->value )
			pparams->nextView = 1; // force a second renderer view

		gHUD.m_Spectator.m_iDrawCycle = 0;
	}
	else
	{
		// second renderer cycle, inset window

		// set inset parameters
		pparams->viewport[0] = XRES_HD( gHUD.m_Spectator.m_OverviewData.insetWindowX ); // change viewport to inset window
		pparams->viewport[1] = YRES_HD( gHUD.m_Spectator.m_OverviewData.insetWindowY );
		pparams->viewport[2] = XRES_HD( gHUD.m_Spectator.m_OverviewData.insetWindowWidth );
		pparams->viewport[3] = YRES_HD( gHUD.m_Spectator.m_OverviewData.insetWindowHeight );
		pparams->nextView    = 0; // on further view

		// override some settings in certain modes
		switch ( (int)gHUD.m_Spectator.m_pip->value )
		{
		case INSET_CHASE_FREE:
			V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
			break;

		case INSET_IN_EYE:
			V_CalcNormalRefdef( pparams );
			break;

		case INSET_MAP_FREE:
			pparams->onlyClientDraw = true;
			V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
			break;

		case INSET_MAP_CHASE:
			pparams->onlyClientDraw = true;

			if ( g_iUser1 == OBS_ROAMING )
				V_GetMapChasePosition( 0, v_cl_angles, v_origin, v_angles );
			else
				V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );

			break;
		}

		gHUD.m_Spectator.m_iDrawCycle = 1;
	}

	// write back new values into pparams
	VectorCopy( v_cl_angles, pparams->cl_viewangles );
	VectorCopy( v_angles, pparams->viewangles )
	    VectorCopy( v_origin, pparams->vieworg );
}

#if defined( TRACE_TEST )

extern float in_fov;
/*
====================
CalcFov
====================
*/
float CalcFov( float fov_x, float width, float height )
{
	float a;
	float x;

	if ( fov_x < 1 || fov_x > 179 )
		fov_x = 90; // error, set to 90

	x = width / tan( fov_x / 360 * M_PI );

	a = atan( height / x );

	a = a * 360 / M_PI;

	return a;
}

int hitent = -1;

void V_Move( int mx, int my )
{
	float fov;
	float fx, fy;
	float dx, dy;
	float c_x, c_y;
	float dX, dY;
	vec3_t forward, up, right;
	vec3_t newangles;

	vec3_t farpoint;
	pmtrace_t tr;

	fov = CalcFov( in_fov, (float)ScreenWidth, (float)ScreenHeight );

	c_x = (float)ScreenWidth / 2.0;
	c_y = (float)ScreenHeight / 2.0;

	dx = (float)mx - c_x;
	dy = (float)my - c_y;

	// Proportion we moved in each direction
	fx = dx / c_x;
	fy = dy / c_y;

	dX = fx * in_fov / 2.0;
	dY = fy * fov / 2.0;

	newangles = v_angles;

	newangles[YAW] -= dX;
	newangles[PITCH] += dY;

	// Now rotate v_forward around that point
	AngleVectors( newangles, forward, right, up );

	farpoint = v_origin + 8192 * forward;

	// Trace
	tr = *( gEngfuncs.PM_TraceLine( (float *)&v_origin, (float *)&farpoint, PM_TRACELINE_PHYSENTSONLY, 2 /*point sized hull*/, -1 ) );

	if ( tr.fraction != 1.0 && tr.ent != 0 )
	{
		hitent = PM_GetPhysEntInfo( tr.ent );
		PM_ParticleLine( (float *)&v_origin, (float *)&tr.endpos, 5, 1.0, 0.0 );
	}
	else
	{
		hitent = -1;
	}
}

#endif
