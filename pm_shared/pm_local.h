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

#if !defined( PM_LOCAL_H )
#define PM_LOCAL_H
#pragma once

#include <assert.h>
#include "mathlib.h"
#include "const.h"
#include "minmax.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_shared.h"
#include "pm_movevars.h"
#include "pm_debug.h"
#include <stdio.h>  // NULL
#include <math.h>   // sqrt
#include <string.h> // strcpy
#include <stdlib.h> // atoi
#include <ctype.h>  // isspace

#pragma warning( disable : 4305 )

#ifdef CLIENT_DLL
extern int iJumpSpectator;
#ifndef DISABLE_JUMP_ORIGIN
extern float vJumpOrigin[3];
extern float vJumpAngles[3];
#else
extern float vJumpOrigin[3];
extern float vJumpAngles[3];
#endif
#endif

typedef enum
{
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_studio
} modtype_t;

extern playermove_t *pmove;
extern int g_onladder;

typedef struct
{
	int planenum;
	short children[2]; // negative numbers are contents
} dclipnode_t;

typedef struct mplane_s
{
	vec3_t normal; // surface normal
	float dist;    // closest appoach to origin
	byte type;     // for texture axis selection and fast side tests
	byte signbits; // signx + signy<<1 + signz<<1
	byte pad[2];
} mplane_t;

typedef struct hull_s
{
	dclipnode_t *clipnodes;
	mplane_t *planes;
	int firstclipnode;
	int lastclipnode;
	vec3_t clip_mins;
	vec3_t clip_maxs;
} hull_t;

// Ducking time
#define TIME_TO_DUCK 0.4
#define VEC_DUCK_HULL_MIN -18
#define VEC_DUCK_HULL_MAX 18
#define VEC_DUCK_VIEW 12
#define PM_DEAD_VIEWHEIGHT -8
#define MAX_CLIMB_SPEED 200
#define STUCK_MOVEUP 1
#define STUCK_MOVEDOWN -1
#define VEC_HULL_MIN -36
#define VEC_HULL_MAX 36
#define VEC_VIEW 28
#define STOP_EPSILON 0.1

#define CTEXTURESMAX 512    // max number of textures loaded
#define CBTEXTURENAMEMAX 13 // only load first n chars of name

#define CHAR_TEX_CONCRETE 'C' // texture types
#define CHAR_TEX_METAL 'M'
#define CHAR_TEX_DIRT 'D'
#define CHAR_TEX_VENT 'V'
#define CHAR_TEX_GRATE 'G'
#define CHAR_TEX_TILE 'T'
#define CHAR_TEX_SLOSH 'S'
#define CHAR_TEX_WOOD 'W'
#define CHAR_TEX_COMPUTER 'P'
#define CHAR_TEX_GLASS 'Y'
#define CHAR_TEX_FLESH 'F'

#define STEP_CONCRETE 0 // default step sound
#define STEP_METAL 1    // metal floor
#define STEP_DIRT 2     // dirt, sand, rock
#define STEP_VENT 3     // ventillation duct
#define STEP_GRATE 4    // metal grating
#define STEP_TILE 5     // floor tiles
#define STEP_SLOSH 6    // shallow liquid puddle
#define STEP_WADE 7     // wading in liquid
#define STEP_LADDER 8   // climbing ladder

#define PLAYER_FATAL_FALL_SPEED 1024
#define PLAYER_MAX_SAFE_FALL_SPEED 580
#define DAMAGE_FOR_FALL_SPEED (float)100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )
#define PLAYER_MIN_BOUNCE_SPEED 200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350
#define PLAYER_LONGJUMP_SPEED 350
#define PLAYER_DUCKING_MULTIPLIER 0.333

#ifndef PITCH
#define PITCH 0
#endif
#ifndef YAW
#define YAW 1
#endif
#ifndef ROLL
#define ROLL 2
#endif

#ifndef MAX_CLIENTS
#define MAX_CLIENTS 32
#endif

#define CONTENTS_CURRENT_0 -9
#define CONTENTS_CURRENT_90 -10
#define CONTENTS_CURRENT_180 -11
#define CONTENTS_CURRENT_270 -12
#define CONTENTS_CURRENT_UP -13
#define CONTENTS_CURRENT_DOWN -14
#define CONTENTS_TRANSLUCENT -15
#define PM_CHECKSTUCK_MINTIME 0.05
#define BUNNYJUMP_MAX_SPEED_FACTOR 1.7f
#define WJ_HEIGHT 8

void PM_InitTextureTypes( void );
void PM_PlayStepSound( int step, float fvol );
int PM_MapTextureTypeStepType( char chTextureType );
void PM_CatagorizeTextureType( void );
void PM_UpdateStepSound( void );
void PM_PlayWaterSounds( void );

void PM_CheckVelocity( void );
int PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddCorrectGravity( void );
void PM_FixupGravityVelocity( void );
void PM_AddGravity( void );
int PM_FlyMove( void );
void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel );
void PM_WalkMove( void );
void PM_Friction( void );

void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel );
void PM_AirMove( void );
void PM_Jump( void );
void PM_PreventMegaBunnyJumping( void );
void PM_CheckFalling( void );
void PM_Physics_Toss( void );
void PM_NoClip( void );
void PM_SpectatorMove( void );

void PM_WaterMove( void );
qboolean PM_InWater( void );
qboolean PM_CheckWater( void );
void PM_WaterJump( void );
void PM_CheckWaterJump( void );

physent_t *PM_Ladder( void );
void PM_LadderMove( physent_t *pLadder );

float PM_SplineFraction( float value, float scale );
void PM_FixPlayerCrouchStuck( int direction );
void PM_UnDuck( void );
void PM_Duck( void );

qboolean PM_AddToTouched( pmtrace_t tr, vec3_t impactvelocity );
void PM_CatagorizePosition( void );
int PM_GetRandomStuckOffsets( int nIndex, int server, vec3_t offset );
void PM_ResetStuckOffsets( int nIndex, int server );
int PM_CheckStuck( void );
void PM_CreateStuckTable( void );
pmtrace_t PM_PushEntity( vec3_t push );

float PM_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed );
void PM_DropPunchAngle( vec3_t punchangle );
void PM_CheckParamters( void );
void PM_ReduceTimers( void );

#endif // PM_LOCAL_H
