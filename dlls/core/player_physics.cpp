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
/*

===== player.cpp ========================================================

  functions dealing with the player

*/

#include "core/extdll.h"
#include "core/util.h"

#include "core/cbase.h"
#include "core/player.h"
#include "weapons/weapon_spraycan.h"
#include "world/trains.h"
#include "ai/nodes.h"
#include "weapons.h"
#include "ai/soundent.h"
#include "ai/monsters.h"
#include "shake.h"
#include "core/decals.h"
#include "gameplay/gamerules.h"
#include "core/game.h"
#include "pm_shared.h"
#include "hltv.h"

// #define DUCKFIX

extern DLL_GLOBAL ULONG g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL g_fGameOver;
#define AIRTIME 12
#include "core/player_physics.h"

void CBasePlayer::WaterMove()
{
	int air;

	if ( pev->movetype == MOVETYPE_NOCLIP )
		return;

	if ( pev->health < 0 )
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	if ( pev->waterlevel != 3 )
	{
		// not underwater

		// play 'up for air' sound
		if ( pev->air_finished < gpGlobals->time )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM );
		else if ( pev->air_finished < gpGlobals->time + 9 )
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM );

		pev->air_finished = gpGlobals->time + AIRTIME;
		pev->dmg          = 2;

		// if we took drowning damage, give it back slowly
		if ( m_idrowndmg > m_idrownrestored )
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.

			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		}
	}
	else
	{ // fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;

		if ( pev->air_finished < gpGlobals->time ) // drown!
		{
			if ( pev->pain_finished < gpGlobals->time )
			{
				// take drowning damage
				pev->dmg += 1;
				if ( pev->dmg > 5 )
					pev->dmg = 5;
				TakeDamage( VARS( eoNullEntity ), VARS( eoNullEntity ), pev->dmg, DMG_DROWN );
				pev->pain_finished = gpGlobals->time + 1;

				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += pev->dmg;
			}
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
		}
	}

	if ( !pev->waterlevel )
	{
		if ( FBitSet( pev->flags, FL_INWATER ) )
		{
			ClearBits( pev->flags, FL_INWATER );
		}
		return;
	}

	// make bubbles

	air = (int)( pev->air_finished - gpGlobals->time );
	if ( !RANDOM_LONG( 0, 0x1f ) && RANDOM_LONG( 0, AIRTIME - 1 ) >= air )
	{
		switch ( RANDOM_LONG( 0, 3 ) )
		{
		case 0:
			EMIT_SOUND( ENT( pev ), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM );
			break;
		case 1:
			EMIT_SOUND( ENT( pev ), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM );
			break;
		case 2:
			EMIT_SOUND( ENT( pev ), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM );
			break;
		case 3:
			EMIT_SOUND( ENT( pev ), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM );
			break;
		}
	}

	if ( pev->watertype == CONTENT_LAVA ) // do damage
	{
		if ( pev->dmgtime < gpGlobals->time )
			TakeDamage( VARS( eoNullEntity ), VARS( eoNullEntity ), 10 * pev->waterlevel, DMG_BURN );
	}
	else if ( pev->watertype == CONTENT_SLIME ) // do damage
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage( VARS( eoNullEntity ), VARS( eoNullEntity ), 4 * pev->waterlevel, DMG_ACID );
	}

	if ( !FBitSet( pev->flags, FL_INWATER ) )
	{
		SetBits( pev->flags, FL_INWATER );
		pev->dmgtime = 0;
	}
}

void CBasePlayer::Jump()
{
	Vector vecWallCheckDir; // direction we're tracing a line to find a wall when walljumping
	Vector vecAdjustedVelocity;
	Vector vecSpot;
	TraceResult tr;

	if ( FBitSet( pev->flags, FL_WATERJUMP ) )
		return;

	if ( pev->waterlevel >= 2 )
	{
		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if ( !FBitSet( m_afButtonPressed, IN_JUMP ) )
		return; // don't pogo stick

	if ( !( pev->flags & FL_ONGROUND ) || !pev->groundentity )
	{
		return;
	}

	// many features in this function use v_forward, so makevectors now.
	UTIL_MakeVectors( pev->angles );

	// ClearBits(pev->flags, FL_ONGROUND);		// don't stairwalk

	SetAnimation( PLAYER_JUMP );

	if ( m_fLongJump &&
	     ( pev->button & IN_DUCK ) &&
	     ( pev->flDuckTime > 0 ) &&
	     pev->velocity.Length() > 50 )
	{
		SetAnimation( PLAYER_SUPERJUMP );
	}

	// If you're standing on a conveyor, add it's velocity to yours (for momentum)
	entvars_t *pevGround = VARS( pev->groundentity );
	if ( pevGround && ( pevGround->flags & FL_CONVEYOR ) )
	{
		pev->velocity = pev->velocity + pev->basevelocity;
	}

	// JoshA: CS behaviour does this for tracktrain + train as well,
	// but let's just do this for func_vehicle to avoid breaking existing content.
	//
	// If you're standing on a moving train... then add the velocity of the train to yours.
	if ( pevGround && ( /*(!strcmp( "func_tracktrain", STRING(pevGround->classname))) ||
	                     (!strcmp( "func_train", STRING(pevGround->classname))) ) ||*/
	                    ( !strcmp( "func_vehicle", STRING( pevGround->classname ) ) ) ) )
		pev->velocity = pev->velocity + pevGround->velocity;
}

// This is a glorious hack to find free space when you've crouched into some solid space
// Our crouching collisions do not work correctly for some reason and this is easier
// than fixing the problem :(
void FixPlayerCrouchStuck( edict_t *pPlayer )
{
	TraceResult trace;

	// Move up as many as 18 pixels if the player is stuck.
	for ( int i = 0; i < 18; i++ )
	{
		UTIL_TraceHull( pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace );
		if ( trace.fStartSolid )
			pPlayer->v.origin.z++;
		else
			break;
	}
}

void CBasePlayer::Duck()
{
	if ( pev->button & IN_DUCK )
	{
		if ( m_IdealActivity != ACT_LEAP )
		{
			SetAnimation( PLAYER_WALK );
		}
	}
}

void CBasePlayer::PreThink( void )
{
	int buttonsChanged = ( m_afButtonLast ^ pev->button ); // These buttons have changed this frame

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed  = buttonsChanged & pev->button;      // The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & ( ~pev->button ); // The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	if ( g_fGameOver )
		return; // intermission or finale

	UTIL_MakeVectors( pev->v_angle ); // is this still used?

	ItemPreFrame();
	WaterMove();

	if ( g_pGameRules && g_pGameRules->FAllowFlashlight() )
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();

	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// Observer Button Handling
	if ( IsObserver() )
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		pev->impulse = 0;
		return;
	}

	if ( pev->deadflag >= DEAD_DYING )
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
		pev->flags |= FL_ONTRAIN;
	else
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
	{
		CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );
		float vel;

		if ( !pTrain )
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -38 ), ignore_monsters, ENT( pev ), &trainTrace );

			// HACKHACK - Just look for the func_tracktrain classname
			if ( trainTrace.flFraction != 1.0 && trainTrace.pHit )
				pTrain = CBaseEntity::Instance( trainTrace.pHit );

			if ( !pTrain || !( pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE ) || !pTrain->OnControls( pev ) )
			{
				// ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				if ( pTrain->Classify() == CLASS_VEHICLE )
					( (CFuncVehicle *)pTrain )->m_pDriver = NULL;
				return;
			}
		}
		else if ( !FBitSet( pev->flags, FL_ONGROUND ) || FBitSet( pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL ) || ( ( pev->button & ( IN_MOVELEFT | IN_MOVERIGHT ) ) && pTrain->Classify() != CLASS_VEHICLE ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			// and it isn't a func_vehicle.
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			if ( pTrain->Classify() == CLASS_VEHICLE )
				( (CFuncVehicle *)pTrain )->m_pDriver = NULL;
			return;
		}

		pev->velocity = g_vecZero;
		vel           = 0;
		if ( pTrain->Classify() == CLASS_VEHICLE )
		{
			if ( pev->button & IN_FORWARD )
			{
				vel = 1;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
			if ( pev->button & IN_BACK )
			{
				vel = -1;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
			if ( pev->button & IN_MOVELEFT )
			{
				vel = 20;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
			if ( pev->button & IN_MOVERIGHT )
			{
				vel = 30;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
		}
		else
		{
			if ( m_afButtonPressed & IN_FORWARD )
			{
				vel = 1;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
			else if ( m_afButtonPressed & IN_BACK )
			{
				vel = -1;
				pTrain->Use( this, this, USE_SET, (float)vel );
			}
		}

		if ( vel )
		{
			m_iTrain = TrainSpeed( pTrain->pev->speed, pTrain->pev->impulse );
			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
		}
	}
	else if ( m_iTrain & TRAIN_ACTIVE )
		m_iTrain = TRAIN_NEW; // turn off train

	if ( pev->button & IN_JUMP )
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}

	// If trying to duck, already ducked, or in the process of ducking
	if ( ( pev->button & IN_DUCK ) || FBitSet( pev->flags, FL_DUCKING ) || ( m_afPhysicsFlags & PFLAG_DUCKING ) )
		Duck();

	if ( !FBitSet( pev->flags, FL_ONGROUND ) )
	{
		m_flFallVelocity = -pev->velocity.z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		pev->velocity = g_vecZero;
	}
}

void CBasePlayer::PostThink()
{
	if ( g_fGameOver )
		goto pt_end; // intermission or finale

	if ( !IsAlive() )
		goto pt_end;

	// Handle Tank controlling
	if ( m_pTank != NULL )
	{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if ( m_pTank->OnControls( pev ) && !pev->weaponmodel )
		{
			m_pTank->Use( this, this, USE_SET, 2 ); // try fire the gun
		}
		else
		{ // they've moved off the platform
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
		}
	}

	// do weapon stuff
	ItemPostFrame();

	// check to see if player landed hard enough to make a sound
	// falling farther than half of the maximum safe distance, but not as far a max safe distance will
	// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
	// of maximum safe distance will make no sound. Falling farther than max safe distance will play a
	// fallpain sound, and damage will be inflicted based on how far the player fell

	if ( ( FBitSet( pev->flags, FL_ONGROUND ) ) && ( pev->health > 0 ) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		if ( pev->watertype == CONTENT_WATER )
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when
			// BUG - water has current force
			// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
			// EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if ( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{ // after this point, we start doing damage

			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this );

			if ( flFallDamage > pev->health )
			{ // splat
				// note: play on item channel because we play footstep landing on body channel
				EMIT_SOUND( ENT( pev ), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM );
			}

			if ( flFallDamage > 0 )
			{
				TakeDamage( VARS( eoNullEntity ), VARS( eoNullEntity ), flFallDamage, DMG_FALL );
				pev->punchangle.x = 0;
			}
		}

		if ( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
	}

	if ( FBitSet( pev->flags, FL_ONGROUND ) )
	{
		if ( m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer() )
		{
			CSoundEnt::InsertSound( bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2 );
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character
	if ( IsAlive() )
	{
		if ( !pev->velocity.x && !pev->velocity.y )
			SetAnimation( PLAYER_IDLE );
		else if ( ( pev->velocity.x || pev->velocity.y ) && ( FBitSet( pev->flags, FL_ONGROUND ) ) )
			SetAnimation( PLAYER_WALK );
		else if ( pev->waterlevel > 1 )
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance();
	CheckPowerups( pev );

	UpdatePlayerSound();

pt_end:
#if defined( CLIENT_WEAPONS )
	// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if ( m_rgpPlayerItems[i] )
		{
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[i];

			while ( pPlayerItem )
			{
				CBasePlayerWeapon *gun;

				gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();

				if ( gun && gun->UseDecrement() )
				{
					gun->m_flNextPrimaryAttack   = max( gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0f );
					gun->m_flNextSecondaryAttack = max( gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001f );

					if ( gun->m_flTimeWeaponIdle != 1000 )
					{
						gun->m_flTimeWeaponIdle = max( gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001f );
					}

					if ( gun->pev->fuser1 != 1000 )
					{
						gun->pev->fuser1 = max( gun->pev->fuser1 - gpGlobals->frametime, -0.001f );
					}

					// Only decrement if not flagged as NO_DECREMENT
					//					if ( gun->m_flPumpTime != 1000 )
					//	{
					//		gun->m_flPumpTime	= max( gun->m_flPumpTime - gpGlobals->frametime, -0.001f );
					//	}
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if ( m_flNextAttack < -0.001 )
		m_flNextAttack = -0.001;

	if ( m_flNextAmmoBurn != 1000 )
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;

		if ( m_flNextAmmoBurn < -0.001 )
			m_flNextAmmoBurn = -0.001;
	}

	if ( m_flAmmoStartCharge != 1000 )
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;

		if ( m_flAmmoStartCharge < -0.001 )
			m_flAmmoStartCharge = -0.001;
	}
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
}
