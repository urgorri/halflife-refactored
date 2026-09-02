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

void PM_LadderMove( physent_t *pLadder )
{
	vec3_t ladderCenter;
	trace_t trace;
	qboolean onFloor;
	vec3_t floor;
	vec3_t modelmins, modelmaxs;

	if ( pmove->movetype == MOVETYPE_NOCLIP )
		return;

#if defined( _TFC )
	// this is how TFC freezes players, so we don't want them climbing ladders
	if ( pmove->maxspeed <= 1.0 )
		return;
#endif

	pmove->PM_GetModelBounds( pLadder->model, modelmins, modelmaxs );

	VectorAdd( modelmins, modelmaxs, ladderCenter );
	VectorScale( ladderCenter, 0.5, ladderCenter );

	pmove->movetype = MOVETYPE_FLY;

	// On ladder, convert movement to be relative to the ladder

	VectorCopy( pmove->origin, floor );
	floor[2] += pmove->player_mins[pmove->usehull][2] - 1;

	if ( pmove->PM_PointContents( floor, NULL ) == CONTENTS_SOLID )
		onFloor = true;
	else
		onFloor = false;

	pmove->gravity = 0;
	pmove->PM_TraceModel( pLadder, pmove->origin, ladderCenter, &trace );
	if ( trace.fraction != 1.0 )
	{
		float forward = 0, right = 0;
		vec3_t vpn, v_right;
		float flSpeed = MAX_CLIMB_SPEED;

		// they shouldn't be able to move faster than their maxspeed
		if ( flSpeed > pmove->maxspeed )
		{
			flSpeed = pmove->maxspeed;
		}

		AngleVectors( pmove->angles, vpn, v_right, NULL );

		if ( pmove->flags & FL_DUCKING )
		{
			flSpeed *= PLAYER_DUCKING_MULTIPLIER;
		}

		if ( pmove->cmd.buttons & IN_BACK )
		{
			forward -= flSpeed;
		}
		if ( pmove->cmd.buttons & IN_FORWARD )
		{
			forward += flSpeed;
		}
		if ( pmove->cmd.buttons & IN_MOVELEFT )
		{
			right -= flSpeed;
		}
		if ( pmove->cmd.buttons & IN_MOVERIGHT )
		{
			right += flSpeed;
		}

		if ( pmove->cmd.buttons & IN_JUMP )
		{
			pmove->movetype = MOVETYPE_WALK;
			VectorScale( trace.plane.normal, 270, pmove->velocity );
		}
		else
		{
			if ( forward != 0 || right != 0 )
			{
				vec3_t velocity, perp, cross, lateral, tmp;
				float normal;

				// ALERT(at_console, "pev %.2f %.2f %.2f - ",
				//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
				//  Calculate player's intended velocity
				// Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
				VectorScale( vpn, forward, velocity );
				VectorMA( velocity, right, v_right, velocity );

				// Perpendicular in the ladder plane
				//					Vector perp = CrossProduct( Vector(0,0,1), trace.vecPlaneNormal );
				//					perp = perp.Normalize();
				VectorClear( tmp );
				tmp[2] = 1;
				CrossProduct( tmp, trace.plane.normal, perp );
				VectorNormalize( perp );

				// decompose velocity into ladder plane
				normal = DotProduct( velocity, trace.plane.normal );
				// This is the velocity into the face of the ladder
				VectorScale( trace.plane.normal, normal, cross );

				// This is the player's additional velocity
				VectorSubtract( velocity, cross, lateral );

				// This turns the velocity into the face of the ladder into velocity that
				// is roughly vertically perpendicular to the face of the ladder.
				// NOTE: It IS possible to face up and move down or face down and move up
				// because the velocity is a sum of the directional velocity and the converted
				// velocity through the face of the ladder -- by design.
				CrossProduct( trace.plane.normal, perp, tmp );
				VectorMA( lateral, -normal, tmp, pmove->velocity );
				if ( onFloor && normal > 0 ) // On ground moving away from the ladder
				{
					VectorMA( pmove->velocity, MAX_CLIMB_SPEED, trace.plane.normal, pmove->velocity );
				}
				// pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
			}
			else
			{
				VectorClear( pmove->velocity );
			}
		}
	}
}

physent_t *PM_Ladder( void )
{
	int i;
	physent_t *pe;
	hull_t *hull;
	int num;
	vec3_t test;

	for ( i = 0; i < pmove->nummoveent; i++ )
	{
		pe = &pmove->moveents[i];

		if ( pe->model && (modtype_t)pmove->PM_GetModelType( pe->model ) == mod_brush && pe->skin == CONTENTS_LADDER )
		{

			hull = (hull_t *)pmove->PM_HullForBsp( pe, test );
			num  = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			VectorSubtract( pmove->origin, test, test );

			// Test the player's hull for intersection with this model
			if ( pmove->PM_HullPointContents( hull, num, test ) == CONTENTS_EMPTY )
				continue;

			return pe;
		}
	}

	return NULL;
}
