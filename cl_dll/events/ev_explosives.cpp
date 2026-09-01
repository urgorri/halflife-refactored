//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Explosive, projectile, and melee weapon events (Crowbar, Crossbow, RPG, Tripmine, Snark).
//
// $NoKeywords: $
//=============================================================================

#include "hud/hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"

#include "eventscripts.h"
#include "ev_hldm.h"

#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio/studio_util.h"
#include "r_studioint.h"

extern void V_PunchAxis( int axis, float punch );

static int g_iSwing = 0;

#define VEC_HULL_MIN Vector( -16, -16, -36 )
#define VEC_DUCK_HULL_MIN Vector( -16, -16, -18 )

//======================
//	   CROWBAR START
//======================
void EV_Crowbar( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, PITCH_NORM );

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( CROWBAR_ATTACK1MISS, 1 );

		switch ( ( g_iSwing++ ) % 3 )
		{
		case 0:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROWBAR_ATTACK1MISS, 1 );
			break;
		case 1:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROWBAR_ATTACK2MISS, 1 );
			break;
		case 2:
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROWBAR_ATTACK3MISS, 1 );
			break;
		}
	}
}
//======================
//	   CROWBAR END
//======================

//======================
//	  CROSSBOW START
//======================
void EV_BoltCallback( struct tempent_s *ent, float frametime, float currenttime )
{
	ent->entity.origin = ent->entity.baseline.vuser1;
	ent->entity.angles = ent->entity.baseline.vuser2;
}

void EV_FireCrossbow2( event_args_t *args )
{
	vec3_t vecSrc, vecEnd;
	vec3_t up, right, forward;
	pmtrace_t tr;

	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );

	VectorCopy( args->velocity, velocity );

	AngleVectors( angles, forward, right, up );

	EV_GetGunPosition( args, vecSrc, origin );

	VectorMA( vecSrc, 8192, forward, vecEnd );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0xF ) );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat( 0.95, 1.0 ), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0xF ) );

	if ( EV_IsLocal( idx ) )
	{
		if ( args->iparam1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );
		else if ( args->iparam2 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE3, 1 );
	}

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_NORMAL, -1, &tr );

	if ( tr.fraction < 1.0 )
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );

		if ( pe->solid != SOLID_BSP )
		{
			switch ( gEngfuncs.pfnRandomLong( 0, 1 ) )
			{
			case 0:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			case 1:
				gEngfuncs.pEventAPI->EV_PlaySound( idx, tr.endpos, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				break;
			}
		}
		else if ( pe->rendermode == kRenderNormal )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( 0, tr.endpos, CHAN_BODY, "weapons/xbow_hit1.wav", gEngfuncs.pfnRandomFloat( 0.95, 1.0 ), ATTN_NORM, 0, PITCH_NORM );

			if ( gEngfuncs.PM_PointContents( tr.endpos, NULL ) != CONTENTS_WATER )
				gEngfuncs.pEfxAPI->R_SparkShower( tr.endpos );

			vec3_t vBoltAngles;
			int iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/crossbow_bolt.mdl" );

			VectorAngles( forward, vBoltAngles );

			TEMPENTITY *bolt = gEngfuncs.pEfxAPI->R_TempModel( tr.endpos - forward * 10, Vector( 0, 0, 0 ), vBoltAngles, 5, iModelIndex, TE_BOUNCE_NULL );

			if ( bolt )
			{
				bolt->flags |= ( FTENT_CLIENTCUSTOM );
				bolt->entity.baseline.vuser1 = tr.endpos - forward * 10;
				bolt->entity.baseline.vuser2 = vBoltAngles;
				bolt->callback               = EV_BoltCallback;
			}
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

void EV_FireCrossbow( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/xbow_fire1.wav", 1, ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0xF ) );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/xbow_reload1.wav", gEngfuncs.pfnRandomFloat( 0.95, 1.0 ), ATTN_NORM, 0, 93 + gEngfuncs.pfnRandomLong( 0, 0xF ) );

	if ( EV_IsLocal( idx ) )
	{
		if ( args->iparam1 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE1, 1 );
		else if ( args->iparam2 )
			gEngfuncs.pEventAPI->EV_WeaponAnimation( CROSSBOW_FIRE3, 1 );

		V_PunchAxis( 0, -2.0 );
	}
}
//======================
//	   CROSSBOW END
//======================

//======================
//	    RPG START
//======================
void EV_FireRpg( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/rocketfire1.wav", 0.9, ATTN_NORM, 0, PITCH_NORM );
	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_ITEM, "weapons/glauncher.wav", 0.7, ATTN_NORM, 0, PITCH_NORM );

	if ( EV_IsLocal( idx ) )
	{
		gEngfuncs.pEventAPI->EV_WeaponAnimation( RPG_FIRE2, 1 );

		V_PunchAxis( 0, -5.0 );
	}
}
//======================
//	     RPG END
//======================

//======================
//	   TRIPMINE START
//======================
void EV_TripmineFire( event_args_t *args )
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy( args->origin, vecSrc );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, NULL, NULL );

	if ( !EV_IsLocal( idx ) )
		return;

	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight( view_ofs );

	vecSrc = vecSrc + view_ofs;

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecSrc + forward * 128, PM_NORMAL, -1, &tr );

	if ( tr.fraction < 1.0 )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( TRIPMINE_DRAW, 0 );

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   TRIPMINE END
//======================

//======================
//	   SQUEAK START
//======================
void EV_SnarkFire( event_args_t *args )
{
	int idx;
	vec3_t vecSrc, angles, view_ofs, forward;
	pmtrace_t tr;

	idx = args->entindex;
	VectorCopy( args->origin, vecSrc );
	VectorCopy( args->angles, angles );

	AngleVectors( angles, forward, NULL, NULL );

	if ( !EV_IsLocal( idx ) )
		return;

	if ( args->ducking )
		vecSrc = vecSrc - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );

	gEngfuncs.pEventAPI->EV_PushPMStates();

	gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc + forward * 20, vecSrc + forward * 64, PM_NORMAL, -1, &tr );

	if ( tr.allsolid == 0 && tr.startsolid == 0 && tr.fraction > 0.25 )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( SQUEAK_THROW, 0 );

	gEngfuncs.pEventAPI->EV_PopPMStates();
}
//======================
//	   SQUEAK END
//======================
