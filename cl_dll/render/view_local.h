//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Internal view and camera rendering headers and declarations.
//
// $NoKeywords: $
//=============================================================================

#if !defined( VIEW_LOCAL_H )
#define VIEW_LOCAL_H
#pragma once

#include "hud/hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "in_defs.h"
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "bench.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"
#include "Exports.h"
#include "r_studioint.h"
#include "com_model.h"
#include "kbutton.h"
#include "view.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CAM_MODE_RELAX 1
#define CAM_MODE_FOCUS 2

extern engine_studio_api_t IEngineStudio;
extern kbutton_t in_mlook;
extern float in_fov;

extern cvar_t *cl_forwardspeed;
extern cvar_t *chase_active;
extern cvar_t *scr_ofsx, *scr_ofsy, *scr_ofsz;
extern cvar_t *cl_vsmoothing;
extern cvar_t *v_centermove;
extern cvar_t *v_centerspeed;
extern cvar_t *cl_bobcycle;
extern cvar_t *cl_bob;
extern cvar_t *cl_bobup;
extern cvar_t *cl_waterdist;
extern cvar_t *cl_chasedist;

extern cvar_t v_iyaw_cycle;
extern cvar_t v_iroll_cycle;
extern cvar_t v_ipitch_cycle;
extern cvar_t v_iyaw_level;
extern cvar_t v_iroll_level;
extern cvar_t v_ipitch_level;

extern vec3_t v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles;
extern float v_frametime, v_lastDistance;
extern float v_cameraRelaxAngle;
extern float v_cameraFocusAngle;
extern int v_cameraMode;
extern qboolean v_resetCamera;
extern vec3_t ev_punchangle;
extern float v_idlescale;

extern float vJumpOrigin[3];
extern float vJumpAngles[3];

// Camera functions
int CL_IsThirdPerson( void );
void CL_CameraOffset( float *ofs );
void V_NormalizeAngles( float *angles );
void V_InterpolateAngles( float *start, float *end, float *output, float frac );
void V_SmoothInterpolateAngles( float *startAngle, float *endAngle, float *finalAngle, float degreesPerSec );
void V_GetChaseOrigin( float *angles, float *origin, float distance, float *returnvec );
void V_GetSingleTargetCam( cl_entity_t *ent1, float *angle, float *origin );
float MaxAngleBetweenAngles( float *a1, float *a2 );
void V_GetDoubleTargetsCam( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin );
void V_GetDirectedChasePosition( cl_entity_t *ent1, cl_entity_t *ent2, float *angle, float *origin );
void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles );
void V_ResetChaseCam( void );
void V_GetInEyePos( int target, float *origin, float *angles );
void V_GetMapFreePosition( float *cl_angles, float *origin, float *angles );
void V_GetMapChasePosition( int target, float *cl_angles, float *origin, float *angles );
void V_CalcSpectatorRefdef( struct ref_params_s *pparams );

// Bob and view dynamics
float V_CalcBob( struct ref_params_s *pparams );
float V_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed );
void V_StartPitchDrift( void );
void V_StopPitchDrift( void );
void V_DriftPitch( struct ref_params_s *pparams );
void V_CalcGunAngle( struct ref_params_s *pparams );
void V_AddIdle( struct ref_params_s *pparams );
void V_CalcViewRoll( struct ref_params_s *pparams );
void V_DropPunchAngle( float frametime, float *ev_punchangle );
void V_PunchAxis( int axis, float punch );

// Core view
void V_CalcIntermissionRefdef( struct ref_params_s *pparams );
void V_CalcNormalRefdef( struct ref_params_s *pparams );
int V_FindViewModelByWeaponModel( int weaponindex );
extern "C" void CL_DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams );
void V_Init( void );
float CalcFov( float fov_x, float width, float height );
void V_Move( int mx, int my );

void PM_ParticleLine( float *start, float *end, int pcolor, float life, float vert );
int PM_GetVisEntInfo( int ent );
extern "C" int PM_GetPhysEntInfo( int ent );
void InterpolateAngles( float *start, float *end, float *output, float frac );
void NormalizeAngles( float *angles );
extern "C" float Distance( const float *v1, const float *v2 );
float AngleBetweenVectors( const float *v1, const float *v2 );
void VectorAngles( const float *forward, float *angles );

#endif // VIEW_LOCAL_H
