/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/saverestore.h"
#include "world/plats.h"
#include "world/trains.h"
#include "systems/effects.h"

LINK_ENTITY_TO_CLASS( func_train, CFuncTrain );
TYPEDESCRIPTION CFuncTrain::m_SaveData[] =
    {
        DEFINE_FIELD( CFuncTrain, m_sounds, FIELD_INTEGER ),
        DEFINE_FIELD( CFuncTrain, m_pevCurrentTarget, FIELD_EVARS ),
        DEFINE_FIELD( CFuncTrain, m_activated, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CFuncTrain, CBasePlatTrain );

void CFuncTrain ::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "sounds" ) )
	{
		m_sounds       = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBasePlatTrain::KeyValue( pkvd );
}

void CFuncTrain ::Blocked( CBaseEntity *pOther )

{
	if ( gpGlobals->time < m_flActivateFinished )
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH );
}

void CFuncTrain ::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER )
	{
		// Move toward my target
		pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
		Next();
	}
	else
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// Pop back to last target if it's available
		if ( pev->enemy )
			pev->target = pev->enemy->v.targetname;
		pev->nextthink = 0;
		pev->velocity  = g_vecZero;
		if ( pev->noiseStopMoving )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
	}
}

void CFuncTrain ::Wait( void )
{
	// Fire the pass target if there is one
	if ( m_pevCurrentTarget->message )
	{
		FireTargets( STRING( m_pevCurrentTarget->message ), this, this, USE_TOGGLE, 0 );
		if ( FBitSet( m_pevCurrentTarget->spawnflags, SF_CORNER_FIREONCE ) )
			m_pevCurrentTarget->message = 0;
	}

	// need pointer to LAST target.
	if ( FBitSet( m_pevCurrentTarget->spawnflags, SF_TRAIN_WAIT_RETRIGGER ) || ( pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER ) )
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// clear the sound channel.
		if ( pev->noiseMovement )
			STOP_SOUND( edict(), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );
		if ( pev->noiseStopMoving )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
		pev->nextthink = 0;
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if ( m_flWait != 0 )
	{ // -1 wait will wait forever!
		pev->nextthink = pev->ltime + m_flWait;
		if ( pev->noiseMovement )
			STOP_SOUND( edict(), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );
		if ( pev->noiseStopMoving )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
		SetThink( &CFuncTrain::Next );
	}
	else
	{
		Next(); // do it RIGHT now!
	}
}

//
// Train next - path corner needs to change to next target
//
void CFuncTrain ::Next( void )
{
	CBaseEntity *pTarg;

	// now find our next target
	pTarg = GetNextTarget();

	if ( !pTarg )
	{
		if ( pev->noiseMovement )
			STOP_SOUND( edict(), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );
		// Play stop sound
		if ( pev->noiseStopMoving )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, (char *)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
		return;
	}

	// Save last target in case we need to find it again
	pev->message = pev->target;

	pev->target = pTarg->pev->target;
	m_flWait    = pTarg->GetDelay();

	if ( m_pevCurrentTarget && m_pevCurrentTarget->speed != 0 )
	{ // don't copy speed from target if it is 0 (uninitialized)
		pev->speed = m_pevCurrentTarget->speed;
		ALERT( at_aiconsole, "Train %s speed to %4.2f\n", STRING( pev->targetname ), pev->speed );
	}
	m_pevCurrentTarget = pTarg->pev; // keep track of this since path corners change our target for us.

	pev->enemy = pTarg->edict(); // hack

	if ( FBitSet( m_pevCurrentTarget->spawnflags, SF_CORNER_TELEPORT ) )
	{
		// Path corner has indicated a teleport to the next corner.
		SetBits( pev->effects, EF_NOINTERP );
		UTIL_SetOrigin( pev, pTarg->pev->origin - ( pev->mins + pev->maxs ) * 0.5 );
		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from CHAN_VOICE to CHAN_STATIC around OEM beta time because trains should
		// use CHAN_STATIC for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).
		if ( pev->noiseMovement )
			STOP_SOUND( edict(), CHAN_STATIC, (char *)STRING( pev->noiseMovement ) );
		if ( pev->noiseMovement )
			EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noiseMovement ), m_volume, ATTN_NORM );
		ClearBits( pev->effects, EF_NOINTERP );
		SetMoveDone( &CFuncTrain::Wait );
		LinearMove( pTarg->pev->origin - ( pev->mins + pev->maxs ) * 0.5, pev->speed );
	}
}

void CFuncTrain ::Activate( void )
{
	// Not yet active, so teleport to first target
	if ( !m_activated )
	{
		m_activated        = TRUE;
		entvars_t *pevTarg = VARS( FIND_ENTITY_BY_TARGETNAME( NULL, STRING( pev->target ) ) );

		pev->target        = pevTarg->target;
		m_pevCurrentTarget = pevTarg; // keep track of this since path corners change our target for us.

		UTIL_SetOrigin( pev, pevTarg->origin - ( pev->mins + pev->maxs ) * 0.5 );

		if ( FStringNull( pev->targetname ) )
		{ // not triggered, so start immediately
			pev->nextthink = pev->ltime + 0.1;
			SetThink( &CFuncTrain::Next );
		}
		else
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}
}

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/

void CFuncTrain ::Spawn( void )
{
	Precache();
	if ( pev->speed == 0 )
		pev->speed = 100;

	if ( FStringNull( pev->target ) )
		ALERT( at_console, "FuncTrain with no target" );

	if ( pev->dmg == 0 )
		pev->dmg = 2;

	pev->movetype = MOVETYPE_PUSH;

	if ( FBitSet( pev->spawnflags, SF_TRACKTRAIN_PASSABLE ) )
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );

	m_activated = FALSE;

	if ( m_volume == 0 )
		m_volume = 0.85;
}

void CFuncTrain::Precache( void )
{
	CBasePlatTrain::Precache();

#if 0 // obsolete
	// otherwise use preset sound
	switch (m_sounds)
	{
	case 0:
		pev->noise = 0;
		pev->noise1 = 0;
		break;

	case 1:
		PRECACHE_SOUND ("plats/train2.wav");
		PRECACHE_SOUND ("plats/train1.wav");
		pev->noise = MAKE_STRING("plats/train2.wav");
		pev->noise1 = MAKE_STRING("plats/train1.wav");
		break;

	case 2:
		PRECACHE_SOUND ("plats/platmove1.wav");
		PRECACHE_SOUND ("plats/platstop1.wav");
		pev->noise = MAKE_STRING("plats/platstop1.wav");
		pev->noise1 = MAKE_STRING("plats/platmove1.wav");
		break;
	}
#endif
}

void CFuncTrain::OverrideReset( void )
{
	CBaseEntity *pTarg;

	// Are we moving?
	if ( pev->velocity != g_vecZero && pev->nextthink != 0 )
	{
		pev->target = pev->message;
		// now find our next target
		pTarg = GetNextTarget();
		if ( !pTarg )
		{
			pev->nextthink = 0;
			pev->velocity  = g_vecZero;
		}
		else // Keep moving for 0.1 secs, then find path_corner again and restart
		{
			SetThink( &CFuncTrain::Next );
			pev->nextthink = pev->ltime + 0.1;
		}
	}
}

// ---------------------------------------------------------------------
//
// Track Train
//
// ---------------------------------------------------------------------

TYPEDESCRIPTION CFuncTrackTrain::m_SaveData[] =
    {
        DEFINE_FIELD( CFuncTrackTrain, m_ppath, FIELD_CLASSPTR ),
        DEFINE_FIELD( CFuncTrackTrain, m_length, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_height, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_speed, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_dir, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_startSpeed, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_controlMins, FIELD_VECTOR ),
        DEFINE_FIELD( CFuncTrackTrain, m_controlMaxs, FIELD_VECTOR ),
        DEFINE_FIELD( CFuncTrackTrain, m_sounds, FIELD_INTEGER ),
        DEFINE_FIELD( CFuncTrackTrain, m_flVolume, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_flBank, FIELD_FLOAT ),
        DEFINE_FIELD( CFuncTrackTrain, m_oldSpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CFuncTrackTrain, CBaseEntity );
LINK_ENTITY_TO_CLASS( func_tracktrain, CFuncTrackTrain );

void CFuncTrackTrain ::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "wheels" ) )
	{
		m_length       = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "height" ) )
	{
		m_height       = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "startspeed" ) )
	{
		m_startSpeed   = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "sounds" ) )
	{
		m_sounds       = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "volume" ) )
	{
		m_flVolume = (float)( atoi( pkvd->szValue ) );
		m_flVolume *= 0.1;
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "bank" ) )
	{
		m_flBank       = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFuncTrackTrain ::NextThink( float thinkTime, BOOL alwaysThink )
{
	if ( alwaysThink )
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}

void CFuncTrackTrain ::Blocked( CBaseEntity *pOther )
{
	entvars_t *pevOther = pOther->pev;

	// Blocker is on-ground on the train
	if ( FBitSet( pevOther->flags, FL_ONGROUND ) && VARS( pevOther->groundentity ) == pev )
	{
		float deltaSpeed = fabs( pev->speed );
		if ( deltaSpeed > 50 )
			deltaSpeed = 50;
		if ( !pevOther->velocity.z )
			pevOther->velocity.z += deltaSpeed;
		return;
	}
	else
		pevOther->velocity = ( pevOther->origin - pev->origin ).Normalize() * pev->dmg;

	ALERT( at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING( pev->targetname ), STRING( pOther->pev->classname ), pev->dmg );
	if ( pev->dmg <= 0 )
		return;
	// we can't hurt this thing, so we're not concerned with it
	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH );
}

void CFuncTrackTrain ::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( useType != USE_SET )
	{
		if ( !ShouldToggle( useType, ( pev->speed != 0 ) ) )
			return;

		if ( pev->speed == 0 )
		{
			pev->speed = m_speed * m_dir;

			Next();
		}
		else
		{
			pev->speed     = 0;
			pev->velocity  = g_vecZero;
			pev->avelocity = g_vecZero;
			StopSound();
			SetThink( NULL );
		}
	}
	else
	{
		float delta = value;

		delta = ( (int)( pev->speed * 4 ) / (int)m_speed ) * 0.25 + 0.25 * delta;
		if ( delta > 1 )
			delta = 1;
		else if ( delta < -1 )
			delta = -1;
		if ( pev->spawnflags & SF_TRACKTRAIN_FORWARDONLY )
		{
			if ( delta < 0 )
				delta = 0;
		}
		pev->speed = m_speed * delta;
		Next();
		ALERT( at_aiconsole, "TRAIN(%s), speed to %.2f\n", STRING( pev->targetname ), pev->speed );
	}
}

static float Fix( float angle )
{
	while ( angle < 0 )
		angle += 360;
	while ( angle > 360 )
		angle -= 360;

	return angle;
}

static void FixupAngles( Vector &v )
{
	v.x = Fix( v.x );
	v.y = Fix( v.y );
	v.z = Fix( v.z );
}

#define TRAIN_STARTPITCH 60
#define TRAIN_MAXPITCH 200
#define TRAIN_MAXSPEED 1000 // approx max speed for sound pitch calculation

void CFuncTrackTrain ::StopSound( void )
{
	// if sound playing, stop it
	if ( m_soundPlaying && pev->noise )
	{
		unsigned short us_encode;
		unsigned short us_sound = ( (unsigned short)( m_sounds ) & 0x0007 ) << 12;

		us_encode = us_sound;

		PLAYBACK_EVENT_FULL( FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 1, 0 );

		/*
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise));
		*/
		EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "plats/ttrain_brake1.wav", m_flVolume, ATTN_NORM, 0, 100 );
	}

	m_soundPlaying = 0;
}

// update pitch based on speed, start sound if not playing
// NOTE: when train goes through transition, m_soundPlaying should go to 0,
// which will cause the looped sound to restart.

void CFuncTrackTrain ::UpdateSound( void )
{
	float flpitch;

	if ( !pev->noise )
		return;

	flpitch = TRAIN_STARTPITCH + ( abs( pev->speed ) * ( TRAIN_MAXPITCH - TRAIN_STARTPITCH ) / TRAIN_MAXSPEED );

	if ( !m_soundPlaying )
	{
		// play startup sound for train
		EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "plats/ttrain_start1.wav", m_flVolume, ATTN_NORM, 0, 100 );
		EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC, (char *)STRING( pev->noise ), m_flVolume, ATTN_NORM, 0, (int)flpitch );
		m_soundPlaying = 1;
	}
	else
	{
		/*
		        // update pitch
		        EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise), m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, (int) flpitch);
		*/
		// volume 0.0 - 1.0 - 6 bits
		// m_sounds 3 bits
		// flpitch = 6 bits
		// 15 bits total

		unsigned short us_encode;
		unsigned short us_sound  = ( (unsigned short)( m_sounds ) & 0x0007 ) << 12;
		unsigned short us_pitch  = ( (unsigned short)( flpitch / 10.0 ) & 0x003f ) << 6;
		unsigned short us_volume = ( (unsigned short)( m_flVolume * 40.0 ) & 0x003f );

		us_encode = us_sound | us_pitch | us_volume;

		PLAYBACK_EVENT_FULL( FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 0, 0 );
	}
}

void CFuncTrackTrain ::Next( void )
{
	float time = 0.5;

	if ( !pev->speed )
	{
		ALERT( at_aiconsole, "TRAIN(%s): Speed is 0\n", STRING( pev->targetname ) );
		StopSound();
		return;
	}

	//	if ( !m_ppath )
	//		m_ppath = CPathTrack::Instance(FIND_ENTITY_BY_TARGETNAME( NULL, STRING(pev->target) ));
	if ( !m_ppath )
	{
		ALERT( at_aiconsole, "TRAIN(%s): Lost path\n", STRING( pev->targetname ) );
		StopSound();
		return;
	}

	UpdateSound();

	Vector nextPos = pev->origin;

	nextPos.z -= m_height;
	CPathTrack *pnext = m_ppath->LookAhead( &nextPos, pev->speed * 0.1, 1 );
	nextPos.z += m_height;

	pev->velocity    = ( nextPos - pev->origin ) * 10;
	Vector nextFront = pev->origin;

	nextFront.z -= m_height;
	if ( m_length > 0 )
		m_ppath->LookAhead( &nextFront, m_length, 0 );
	else
		m_ppath->LookAhead( &nextFront, 100, 0 );
	nextFront.z += m_height;

	Vector delta  = nextFront - pev->origin;
	Vector angles = UTIL_VecToAngles( delta );
	// The train actually points west
	angles.y += 180;

	// !!!  All of this crap has to be done to make the angles not wrap around, revisit this.
	FixupAngles( angles );
	FixupAngles( pev->angles );

	if ( !pnext || ( delta.x == 0 && delta.y == 0 ) )
		angles = pev->angles;

	float vy, vx;
	if ( !( pev->spawnflags & SF_TRACKTRAIN_NOPITCH ) )
		vx = UTIL_AngleDistance( angles.x, pev->angles.x );
	else
		vx = 0;
	vy = UTIL_AngleDistance( angles.y, pev->angles.y );

	pev->avelocity.y = vy * 10;
	pev->avelocity.x = vx * 10;

	if ( m_flBank != 0 )
	{
		if ( pev->avelocity.y < -5 )
			pev->avelocity.z = UTIL_AngleDistance( UTIL_ApproachAngle( -m_flBank, pev->angles.z, m_flBank * 2 ), pev->angles.z );
		else if ( pev->avelocity.y > 5 )
			pev->avelocity.z = UTIL_AngleDistance( UTIL_ApproachAngle( m_flBank, pev->angles.z, m_flBank * 2 ), pev->angles.z );
		else
			pev->avelocity.z = UTIL_AngleDistance( UTIL_ApproachAngle( 0, pev->angles.z, m_flBank * 4 ), pev->angles.z ) * 4;
	}

	if ( pnext )
	{
		if ( pnext != m_ppath )
		{
			CPathTrack *pFire;
			if ( pev->speed >= 0 )
				pFire = pnext;
			else
				pFire = m_ppath;

			m_ppath = pnext;
			// Fire the pass target if there is one
			if ( pFire->pev->message )
			{
				FireTargets( STRING( pFire->pev->message ), this, this, USE_TOGGLE, 0 );
				if ( FBitSet( pFire->pev->spawnflags, SF_PATH_FIREONCE ) )
					pFire->pev->message = 0;
			}

			if ( pFire->pev->spawnflags & SF_PATH_DISABLE_TRAIN )
				pev->spawnflags |= SF_TRACKTRAIN_NOCONTROL;

			// Don't override speed if under user control
			if ( pev->spawnflags & SF_TRACKTRAIN_NOCONTROL )
			{
				if ( pFire->pev->speed != 0 )
				{ // don't copy speed from target if it is 0 (uninitialized)
					pev->speed = pFire->pev->speed;
					ALERT( at_aiconsole, "TrackTrain %s speed to %4.2f\n", STRING( pev->targetname ), pev->speed );
				}
			}
		}
		SetThink( &CFuncTrackTrain::Next );
		NextThink( pev->ltime + time, TRUE );
	}
	else // end of path, stop
	{
		StopSound();
		pev->velocity  = ( nextPos - pev->origin );
		pev->avelocity = g_vecZero;
		float distance = pev->velocity.Length();
		m_oldSpeed     = pev->speed;

		pev->speed = 0;

		// Move to the dead end

		// Are we there yet?
		if ( distance > 0 )
		{
			// no, how long to get there?
			time          = distance / m_oldSpeed;
			pev->velocity = pev->velocity * ( m_oldSpeed / distance );
			SetThink( &CFuncTrackTrain::DeadEnd );
			NextThink( pev->ltime + time, FALSE );
		}
		else
		{
			DeadEnd();
		}
	}
}

void CFuncTrackTrain::DeadEnd( void )
{
	// Fire the dead-end target if there is one
	CPathTrack *pTrack, *pNext;

	pTrack = m_ppath;

	ALERT( at_aiconsole, "TRAIN(%s): Dead end ", STRING( pev->targetname ) );
	// Find the dead end path node
	// HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on it's speed
	// so we have to traverse the list to it's end.
	if ( pTrack )
	{
		if ( m_oldSpeed < 0 )
		{
			do
			{
				pNext = pTrack->ValidPath( pTrack->GetPrevious(), TRUE );
				if ( pNext )
					pTrack = pNext;
			} while ( pNext );
		}
		else
		{
			do
			{
				pNext = pTrack->ValidPath( pTrack->GetNext(), TRUE );
				if ( pNext )
					pTrack = pNext;
			} while ( pNext );
		}
	}

	pev->velocity  = g_vecZero;
	pev->avelocity = g_vecZero;
	if ( pTrack )
	{
		ALERT( at_aiconsole, "at %s\n", STRING( pTrack->pev->targetname ) );
		if ( pTrack->pev->netname )
			FireTargets( STRING( pTrack->pev->netname ), this, this, USE_TOGGLE, 0 );
	}
	else
		ALERT( at_aiconsole, "\n" );
}

void CFuncTrackTrain ::SetControls( entvars_t *pevControls )
{
	Vector offset = pevControls->origin - pev->oldorigin;

	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}

BOOL CFuncTrackTrain ::OnControls( entvars_t *pevTest )
{
	Vector offset = pevTest->origin - pev->origin;

	if ( pev->spawnflags & SF_TRACKTRAIN_NOCONTROL )
		return FALSE;

	// Transform offset into local coordinates
	UTIL_MakeVectors( pev->angles );
	Vector local;
	local.x = DotProduct( offset, gpGlobals->v_forward );
	local.y = -DotProduct( offset, gpGlobals->v_right );
	local.z = DotProduct( offset, gpGlobals->v_up );

	if ( local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z &&
	     local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z )
		return TRUE;

	return FALSE;
}

void CFuncTrackTrain ::Find( void )
{
	m_ppath = CPathTrack::Instance( FIND_ENTITY_BY_TARGETNAME( NULL, STRING( pev->target ) ) );
	if ( !m_ppath )
		return;

	entvars_t *pevTarget = m_ppath->pev;
	if ( !FClassnameIs( pevTarget, "path_track" ) )
	{
		ALERT( at_error, "func_track_train must be on a path of path_track\n" );
		m_ppath = NULL;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead( &look, m_length, 0 );
	look.z += m_height;

	pev->angles = UTIL_VecToAngles( look - nextPos );
	// The train actually points west
	pev->angles.y += 180;

	if ( pev->spawnflags & SF_TRACKTRAIN_NOPITCH )
		pev->angles.x = 0;
	UTIL_SetOrigin( pev, nextPos );
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncTrackTrain::Next );
	pev->speed = m_startSpeed;

	UpdateSound();
}

void CFuncTrackTrain ::NearestPath( void )
{
	CBaseEntity *pTrack   = NULL;
	CBaseEntity *pNearest = NULL;
	float dist, closest;

	closest = 1024;

	while ( ( pTrack = UTIL_FindEntityInSphere( pTrack, pev->origin, 1024 ) ) != NULL )
	{
		// filter out non-tracks
		if ( !( pTrack->pev->flags & ( FL_CLIENT | FL_MONSTER ) ) && FClassnameIs( pTrack->pev, "path_track" ) )
		{
			dist = ( pev->origin - pTrack->pev->origin ).Length();
			if ( dist < closest )
			{
				closest  = dist;
				pNearest = pTrack;
			}
		}
	}

	if ( !pNearest )
	{
		ALERT( at_console, "Can't find a nearby track !!!\n" );
		SetThink( NULL );
		return;
	}

	ALERT( at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING( pev->targetname ), STRING( pNearest->pev->targetname ) );
	// If I'm closer to the next path_track on this path, then it's my real path
	pTrack = ( (CPathTrack *)pNearest )->GetNext();
	if ( pTrack )
	{
		if ( ( pev->origin - pTrack->pev->origin ).Length() < ( pev->origin - pNearest->pev->origin ).Length() )
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack *)pNearest;

	if ( pev->speed != 0 )
	{
		NextThink( pev->ltime + 0.1, FALSE );
		SetThink( &CFuncTrackTrain::Next );
	}
}

void CFuncTrackTrain::OverrideReset( void )
{
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncTrackTrain::NearestPath );
}

CFuncTrackTrain *CFuncTrackTrain::Instance( edict_t *pent )
{
	if ( FClassnameIs( pent, "func_tracktrain" ) )
		return (CFuncTrackTrain *)GET_PRIVATE( pent );
	return NULL;
}

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/

void CFuncTrackTrain ::Spawn( void )
{
	if ( pev->speed == 0 )
		m_speed = 100;
	else
		m_speed = pev->speed;

	pev->speed     = 0;
	pev->velocity  = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse   = m_speed;

	m_dir = 1;

	if ( FStringNull( pev->target ) )
		ALERT( at_console, "FuncTrain with no target" );

	if ( pev->spawnflags & SF_TRACKTRAIN_PASSABLE )
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );

	// Cache off placed origin for train controls
	pev->oldorigin = pev->origin;

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;
	// start trains on the next frame, to make sure their targets have had
	// a chance to spawn/activate
	NextThink( pev->ltime + 0.1, FALSE );
	SetThink( &CFuncTrackTrain::Find );
	Precache();
}

void CFuncTrackTrain ::Precache( void )
{
	if ( m_flVolume == 0.0 )
		m_flVolume = 1.0;

	switch ( m_sounds )
	{
	default:
		// no sound
		pev->noise = 0;
		break;
	case 1:
		PRECACHE_SOUND( "plats/ttrain1.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain1.wav" );
		break;
	case 2:
		PRECACHE_SOUND( "plats/ttrain2.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain2.wav" );
		break;
	case 3:
		PRECACHE_SOUND( "plats/ttrain3.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain3.wav" );
		break;
	case 4:
		PRECACHE_SOUND( "plats/ttrain4.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain4.wav" );
		break;
	case 5:
		PRECACHE_SOUND( "plats/ttrain6.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain6.wav" );
		break;
	case 6:
		PRECACHE_SOUND( "plats/ttrain7.wav" );
		pev->noise = MAKE_STRING( "plats/ttrain7.wav" );
		break;
	}

	PRECACHE_SOUND( "plats/ttrain_brake1.wav" );
	PRECACHE_SOUND( "plats/ttrain_start1.wav" );

	m_usAdjustPitch = PRECACHE_EVENT( 1, "events/train.sc" );
}

// This class defines the volume of space that the player must stand in to control the train
LINK_ENTITY_TO_CLASS( func_traincontrols, CFuncTrainControls );

void CFuncTrainControls ::Find( void )
{
	edict_t *pTarget = NULL;

	do
	{
		pTarget = FIND_ENTITY_BY_TARGETNAME( pTarget, STRING( pev->target ) );
	} while ( !FNullEnt( pTarget ) && !FClassnameIs( pTarget, "func_tracktrain" ) );

	if ( FNullEnt( pTarget ) )
	{
		ALERT( at_console, "No train %s\n", STRING( pev->target ) );
		return;
	}

	CFuncTrackTrain *ptrain = CFuncTrackTrain::Instance( pTarget );
	ptrain->SetControls( pev );
	UTIL_Remove( this );
}

void CFuncTrainControls ::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( &CFuncTrainControls::Find );
	pev->nextthink = gpGlobals->time;
}

// ----------------------------------------------------------------------------
//
// Track changer / Train elevator
//
// ----------------------------------------------------------------------------

#define SF_TRACK_ACTIVATETRAIN 0x00000001
#define SF_TRACK_RELINK 0x00000002
#define SF_TRACK_ROTMOVE 0x00000004
#define SF_TRACK_STARTBOTTOM 0x00000008
#define SF_TRACK_DONT_MOVE 0x00000010

//
// This entity is a rotating/moving platform that will carry a train to a new track.
// It must be larger in X-Y planar area than the train, since it must contain the
// train within these dimensions in order to operate when the train is near it.
//

typedef enum
{
	TRAIN_SAFE,
	TRAIN_BLOCKING,
	TRAIN_FOLLOWING
} TRAIN_CODE;

