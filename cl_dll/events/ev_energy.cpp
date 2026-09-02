//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Energy weapon events (Gauss / Tau cannon, Egon / Gluon gun, Hornet gun).
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
extern engine_studio_api_t IEngineStudio;
extern cvar_t *cl_lw;
extern float g_flApplyVel;

#define SND_CHANGE_PITCH ( 1 << 7 )

//======================
//	   GAUSS START
//======================
void EV_SpinGauss( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	int iSoundState = 0;

	int pitch;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	pitch = args->iparam1;

	iSoundState = args->bparam1 ? SND_CHANGE_PITCH : 0;

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "ambience/pulsemachine.wav", 1.0, ATTN_NORM, iSoundState, pitch );
}

void EV_StopPreviousGauss( int idx )
{
	gEngfuncs.pEventAPI->EV_KillEvents( idx, "events/gauss.sc" );
	gEngfuncs.pEventAPI->EV_KillEvents( idx, "events/gaussspin.sc" );
	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_WEAPON, "ambience/pulsemachine.wav" );
}

void EV_FireGauss( event_args_t *args )
{
	int idx;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	float flDamage  = args->fparam1;
	int m_fPrimaryFire  = args->bparam1;
	int m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	vec3_t vecSrc;
	vec3_t vecDest;
	edict_t *pentIgnore;
	pmtrace_t tr, beam_tr;
	float flMaxFrac = 1.0;
	int nTotal      = 0;
	int fHasPunched = 0;
	int fFirstBeam  = 1;
	int nMaxHits    = 10;
	physent_t *pEntity;
	int m_iBeam, m_iGlow, m_iBalls;
	vec3_t up, right, forward;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	VectorCopy( args->velocity, velocity );

	if ( args->bparam2 )
	{
		EV_StopPreviousGauss( idx );
		return;
	}

	EV_GetGunPosition( args, vecSrc, origin );

	m_iBeam  = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/smoke.spr" );
	m_iBalls = m_iGlow = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/hotglow.spr" );

	AngleVectors( angles, forward, right, up );

	VectorMA( vecSrc, 8192, forward, vecDest );

	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, -2.0 );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( GAUSS_FIRE2, 2 );

		if ( m_fPrimaryFire == false )
			g_flApplyVel = flDamage;
	}

	gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "weapons/gauss2.wav", 0.5 + flDamage * ( 1.0 / 400.0 ), ATTN_NORM, 0, 85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );

	while ( flDamage > 10 && nMaxHits > 0 )
	{
		nMaxHits--;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

		gEngfuncs.pEventAPI->EV_PushPMStates();

		gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecDest, PM_NORMAL, -1, &tr );

		gEngfuncs.pEventAPI->EV_PopPMStates();

		if ( tr.allsolid )
			break;

		if ( fFirstBeam )
		{
			if ( EV_IsLocal( idx ) )
			{
				EV_MuzzleFlash();
			}
			fFirstBeam = 0;

			gEngfuncs.pEfxAPI->R_BeamEntPoint(
			    idx | 0x1000,
			    tr.endpos,
			    m_iBeam,
			    0.1,
			    m_fPrimaryFire ? 1.0 : 2.5,
			    0.0,
			    m_fPrimaryFire ? 128.0 : flDamage,
			    0,
			    0,
			    0,
			    m_fPrimaryFire ? 255 : 255,
			    m_fPrimaryFire ? 128 : 255,
			    m_fPrimaryFire ? 0 : 255 );
		}
		else
		{
			gEngfuncs.pEfxAPI->R_BeamPoints( vecSrc,
			                                 tr.endpos,
			                                 m_iBeam,
			                                 0.1,
			                                 m_fPrimaryFire ? 1.0 : 2.5,
			                                 0.0,
			                                 m_fPrimaryFire ? 128.0 : flDamage,
			                                 0,
			                                 0,
			                                 0,
			                                 m_fPrimaryFire ? 255 : 255,
			                                 m_fPrimaryFire ? 128 : 255,
			                                 m_fPrimaryFire ? 0 : 255 );
		}

		pEntity = gEngfuncs.pEventAPI->EV_GetPhysent( tr.ent );
		if ( pEntity == NULL )
			break;

		if ( pEntity->solid == SOLID_BSP )
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct( tr.plane.normal, forward );

			if ( n < 0.5 )
			{
				vec3_t r;

				VectorMA( forward, 2.0 * n, tr.plane.normal, r );

				flMaxFrac = flMaxFrac - tr.fraction;

				VectorCopy( r, forward );

				VectorMA( tr.endpos, 8.0, forward, vecSrc );
				VectorMA( vecSrc, 8192.0, forward, vecDest );

				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage * n / 255.0, flDamage * n * 0.5 * 0.1, FTENT_FADEOUT );

				vec3_t fwd;
				VectorAdd( tr.endpos, tr.plane.normal, fwd );

				gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 100 );

				if ( n == 0 )
				{
					n = 0.1;
				}

				flDamage = flDamage * ( 1 - n );
			}
			else
			{
				EV_HLDM_DecalGunshot( &tr, BULLET_MONSTER_12MM );

				gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 1.0, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );

				if ( fHasPunched )
				{
					break;
				}
				fHasPunched = 1;

				if ( !m_fPrimaryFire )
				{
					vec3_t start;

					VectorMA( tr.endpos, 8.0, forward, start );

					gEngfuncs.pEventAPI->EV_PushPMStates();

					gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );

					gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
					gEngfuncs.pEventAPI->EV_PlayerTrace( start, vecDest, PM_NORMAL, -1, &beam_tr );

					if ( !beam_tr.allsolid )
					{
						vec3_t delta;
						float n;

						gEngfuncs.pEventAPI->EV_PlayerTrace( beam_tr.endpos, tr.endpos, PM_NORMAL, -1, &beam_tr );

						VectorSubtract( beam_tr.endpos, tr.endpos, delta );

						n = Length( delta );

						if ( n < flDamage )
						{
							if ( n == 0 )
								n = 1;
							flDamage -= n;

							{
								vec3_t fwd;
								VectorSubtract( tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 3, 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 100 );
							}

							EV_HLDM_DecalGunshot( &beam_tr, BULLET_MONSTER_12MM );

							gEngfuncs.pEfxAPI->R_TempSprite( beam_tr.endpos, vec3_origin, 0.1, m_iGlow, kRenderGlow, kRenderFxNoDissipation, flDamage / 255.0, 6.0, FTENT_FADEOUT );

							{
								vec3_t fwd;
								VectorSubtract( beam_tr.endpos, forward, fwd );
								gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, beam_tr.endpos, fwd, m_iBalls, (int)( flDamage * 0.3 ), 0.1, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 200, 255, 40 );
							}

							VectorAdd( beam_tr.endpos, forward, vecSrc );
						}
					}
					else
					{
						flDamage = 0;
					}

					gEngfuncs.pEventAPI->EV_PopPMStates();
				}
				else
				{
					if ( m_fPrimaryFire )
					{
						gEngfuncs.pEfxAPI->R_TempSprite( tr.endpos, vec3_origin, 0.2, m_iGlow, kRenderGlow, kRenderFxNoDissipation, 200.0 / 255.0, 0.3, FTENT_FADEOUT );

						{
							vec3_t fwd;
							VectorAdd( tr.endpos, tr.plane.normal, fwd );
							gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100, 255, 200 );
						}
					}

					flDamage = 0;
				}
			}
		}
		else
		{
			VectorAdd( tr.endpos, forward, vecSrc );
		}
	}
}
//======================
//	   GAUSS END
//======================

//======================
//	    EGON START
//======================
static int g_fireAnims1[] = { EGON_FIRE1, EGON_FIRE2, EGON_FIRE3, EGON_FIRE4 };
static int g_fireAnims2[] = { EGON_ALTFIRECYCLE };

enum EGON_FIRESTATE
{
	FIRE_OFF,
	FIRE_CHARGE
};
enum EGON_FIREMODE
{
	FIRE_NARROW,
	FIRE_WIDE
};

#define EGON_PRIMARY_VOLUME 450
#define EGON_BEAM_SPRITE "sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE "sprites/XSpark1.spr"
#define EGON_SOUND_OFF "weapons/egon_off1.wav"
#define EGON_SOUND_RUN "weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP "weapons/egon_windup2.wav"

BEAM *pBeam  = NULL;
BEAM *pBeam2 = NULL;

void EV_EgonFire( event_args_t *args )
{
	int idx, iFireState, iFireMode;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	iFireState   = args->iparam1;
	iFireMode    = args->iparam2;
	int iStartup = args->bparam1;

	if ( iStartup )
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_STARTUP, 0.9, ATTN_NORM, 0, 100 );
	}
	else
	{
		if ( iFireMode == FIRE_WIDE )
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.98, ATTN_NORM, 0, 125 );
		else
			gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_STATIC, EGON_SOUND_RUN, 0.9, ATTN_NORM, 0, 100 );
	}

	if ( EV_IsLocal( idx ) )
		gEngfuncs.pEventAPI->EV_WeaponAnimation( g_fireAnims1[gEngfuncs.pfnRandomLong( 0, 3 )], 1 );

	if ( iStartup == 1 && EV_IsLocal( idx ) && !pBeam && !pBeam2 && cl_lw->value )
	{
		vec3_t vecSrc, vecEnd, origin, angles, forward, right, up;
		pmtrace_t tr;

		cl_entity_t *pl = gEngfuncs.GetEntityByIndex( idx );

		if ( pl )
		{
			VectorCopy( gHUD.m_vecAngles, angles );

			AngleVectors( angles, forward, right, up );

			EV_GetGunPosition( args, vecSrc, pl->origin );

			VectorMA( vecSrc, 2048, forward, vecEnd );

			gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

			gEngfuncs.pEventAPI->EV_PushPMStates();

			gEngfuncs.pEventAPI->EV_SetSolidPlayers( idx - 1 );

			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_NORMAL, -1, &tr );

			gEngfuncs.pEventAPI->EV_PopPMStates();

			int iBeamModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( EGON_BEAM_SPRITE );

			float r = 50.0f;
			float g = 50.0f;
			float b = 125.0f;

			if ( IEngineStudio.IsHardware() )
			{
				r /= 100.0f;
				g /= 100.0f;
			}

			pBeam = gEngfuncs.pEfxAPI->R_BeamEntPoint( idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 3.5, 0.2, 0.7, 55, 0, 0, r, g, b );

			if ( pBeam )
				pBeam->flags |= ( FBEAM_SINENOISE );

			pBeam2 = gEngfuncs.pEfxAPI->R_BeamEntPoint( idx | 0x1000, tr.endpos, iBeamModelIndex, 99999, 5.0, 0.08, 0.7, 25, 0, 0, r, g, b );
		}
	}
}

void EV_EgonStop( event_args_t *args )
{
	int idx;
	vec3_t origin;

	idx = args->entindex;
	VectorCopy( args->origin, origin );

	gEngfuncs.pEventAPI->EV_StopSound( idx, CHAN_STATIC, EGON_SOUND_RUN );

	if ( args->iparam1 )
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, EGON_SOUND_OFF, 0.98, ATTN_NORM, 0, 100 );

	if ( EV_IsLocal( idx ) )
	{
		if ( pBeam )
		{
			pBeam->die = 0.0;
			pBeam      = NULL;
		}

		if ( pBeam2 )
		{
			pBeam2->die = 0.0;
			pBeam2      = NULL;
		}
	}
}
//======================
//	    EGON END
//======================

//======================
//	   HORNET START
//======================
void EV_HornetGunFire( event_args_t *args )
{
	int idx, iFireMode;
	vec3_t origin, angles, vecSrc, forward, right, up;

	idx = args->entindex;
	VectorCopy( args->origin, origin );
	VectorCopy( args->angles, angles );
	iFireMode = args->iparam1;

	if ( EV_IsLocal( idx ) )
	{
		V_PunchAxis( 0, gEngfuncs.pfnRandomLong( 0, 2 ) );
		gEngfuncs.pEventAPI->EV_WeaponAnimation( HGUN_SHOOT, 1 );
	}

	switch ( gEngfuncs.pfnRandomLong( 0, 2 ) )
	{
	case 0:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire1.wav", 1, ATTN_NORM, 0, 100 );
		break;
	case 1:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire2.wav", 1, ATTN_NORM, 0, 100 );
		break;
	case 2:
		gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON, "agrunt/ag_fire3.wav", 1, ATTN_NORM, 0, 100 );
		break;
	}
}
//======================
//	   HORNET END
//======================
