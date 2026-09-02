/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   This source code contains proprietary and confidential information of
 *   Valve LLC and its suppliers.  Access to this code is restricted to
 *   persons who have executed a written SDK license with Valve.  Any access,
 *   use or distribution of this code by or to any unlicensed person is illegal.
 *
 ****/
#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "ai/monsters.h"
#include "ai/schedule.h"
#include "ai/talkmonster.h"

#include "gameplay/scripted.h"
#include "ai/defaultai.h"

#include "ai/soundent.h"
#include "core/animation.h"

//=========================================================
// Talking monster base class
// Used for scientists and barneys
//=========================================================
float CTalkMonster::g_talkWaitTime = 0; // time delay until it's ok to speak: used so that two NPCs don't talk at once

// NOTE: m_voicePitch & m_szGrp should be fixed up by precache each save/restore

TYPEDESCRIPTION CTalkMonster::m_SaveData[] =
    {
        DEFINE_FIELD( CTalkMonster, m_bitsSaid, FIELD_INTEGER ),
        DEFINE_FIELD( CTalkMonster, m_nSpeak, FIELD_INTEGER ),

        // Recalc'ed in Precache()
        //	DEFINE_FIELD( CTalkMonster, m_voicePitch, FIELD_INTEGER ),
        //	DEFINE_FIELD( CTalkMonster, m_szGrp, FIELD_??? ),
        DEFINE_FIELD( CTalkMonster, m_useTime, FIELD_TIME ),
        DEFINE_FIELD( CTalkMonster, m_iszUse, FIELD_STRING ),
        DEFINE_FIELD( CTalkMonster, m_iszUnUse, FIELD_STRING ),
        DEFINE_FIELD( CTalkMonster, m_flLastSaidSmelled, FIELD_TIME ),
        DEFINE_FIELD( CTalkMonster, m_flStopTalkTime, FIELD_TIME ),
        DEFINE_FIELD( CTalkMonster, m_hTalkTarget, FIELD_EHANDLE ),
};

IMPLEMENT_SAVERESTORE( CTalkMonster, CBaseMonster );

// array of friend names
char *CTalkMonster::m_szFriends[TLK_CFRIENDS] =
    {
        "monster_barney",
        "monster_scientist",
        "monster_sitting_scientist",
};

//=========================================================
// AI Schedules Specific to talking monsters
//=========================================================

Task_t tlIdleResponse[] =
    {
        { TASK_SET_ACTIVITY, (float)ACT_IDLE }, // Stop and listen
        { TASK_WAIT, (float)0.5 },              // Wait until sure it's me they are talking to
        { TASK_TLK_EYECONTACT, (float)0 },      // Wait until speaker is done
        { TASK_TLK_RESPOND, (float)0 },         // Wait and then say my response
        { TASK_TLK_IDEALYAW, (float)0 },        // look at who I'm talking to
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 },
        { TASK_TLK_EYECONTACT, (float)0 }, // Wait until speaker is done
};

Schedule_t slIdleResponse[] =
    {
        { tlIdleResponse,
          ARRAYSIZE( tlIdleResponse ),
          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE,
          0,
          "Idle Response"

        },
};

Task_t tlIdleSpeak[] =
    {
        { TASK_TLK_SPEAK, (float)0 },    // question or remark
        { TASK_TLK_IDEALYAW, (float)0 }, // look at who I'm talking to
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 },
        { TASK_TLK_EYECONTACT, (float)0 },
        { TASK_WAIT_RANDOM, (float)0.5 },
};

Schedule_t slIdleSpeak[] =
    {
        { tlIdleSpeak,
          ARRAYSIZE( tlIdleSpeak ),
          bits_COND_NEW_ENEMY |
              bits_COND_CLIENT_PUSH |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE,
          0,
          "Idle Speak" },
};

Task_t tlIdleSpeakWait[] =
    {
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 }, // Stop and talk
        { TASK_TLK_SPEAK, (float)0 },              // question or remark
        { TASK_TLK_EYECONTACT, (float)0 },         //
        { TASK_WAIT, (float)2 },                   // wait - used when sci is in 'use' mode to keep head turned
};

Schedule_t slIdleSpeakWait[] =
    {
        { tlIdleSpeakWait,
          ARRAYSIZE( tlIdleSpeakWait ),
          bits_COND_NEW_ENEMY |
              bits_COND_CLIENT_PUSH |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE,
          0,
          "Idle Speak Wait" },
};

Task_t tlIdleHello[] =
    {
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 }, // Stop and talk
        { TASK_TLK_HELLO, (float)0 },              // Try to say hello to player
        { TASK_TLK_EYECONTACT, (float)0 },
        { TASK_WAIT, (float)0.5 },    // wait a bit
        { TASK_TLK_HELLO, (float)0 }, // Try to say hello to player
        { TASK_TLK_EYECONTACT, (float)0 },
        { TASK_WAIT, (float)0.5 },    // wait a bit
        { TASK_TLK_HELLO, (float)0 }, // Try to say hello to player
        { TASK_TLK_EYECONTACT, (float)0 },
        { TASK_WAIT, (float)0.5 },    // wait a bit
        { TASK_TLK_HELLO, (float)0 }, // Try to say hello to player
        { TASK_TLK_EYECONTACT, (float)0 },
        { TASK_WAIT, (float)0.5 }, // wait a bit

};

Schedule_t slIdleHello[] =
    {
        { tlIdleHello,
          ARRAYSIZE( tlIdleHello ),
          bits_COND_NEW_ENEMY |
              bits_COND_CLIENT_PUSH |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND |
              bits_COND_PROVOKED,

          bits_SOUND_COMBAT,
          "Idle Hello" },
};

Task_t tlIdleStopShooting[] =
    {
        { TASK_TLK_STOPSHOOTING, (float)0 }, // tell player to stop shooting friend
                                             // { TASK_TLK_EYECONTACT,		(float)0		},// look at the player
};

Schedule_t slIdleStopShooting[] =
    {
        { tlIdleStopShooting,
          ARRAYSIZE( tlIdleStopShooting ),
          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND,
          0,
          "Idle Stop Shooting" },
};

Task_t tlMoveAway[] =
    {
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_MOVE_AWAY_FAIL },
        { TASK_STORE_LASTPOSITION, (float)0 },
        { TASK_MOVE_AWAY_PATH, (float)100 },
        { TASK_WALK_PATH_FOR_UNITS, (float)100 },
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FACE_PLAYER, (float)0.5 },
};

Schedule_t slMoveAway[] =
    {
        { tlMoveAway,
          ARRAYSIZE( tlMoveAway ),
          0,
          0,
          "MoveAway" },
};

Task_t tlMoveAwayFail[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FACE_PLAYER, (float)0.5 },
};

Schedule_t slMoveAwayFail[] =
    {
        { tlMoveAwayFail,
          ARRAYSIZE( tlMoveAwayFail ),
          0,
          0,
          "MoveAwayFail" },
};

Task_t tlMoveAwayFollow[] =
    {
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_TARGET_FACE },
        { TASK_STORE_LASTPOSITION, (float)0 },
        { TASK_MOVE_AWAY_PATH, (float)100 },
        { TASK_WALK_PATH_FOR_UNITS, (float)100 },
        { TASK_STOP_MOVING, (float)0 },
        { TASK_SET_SCHEDULE, (float)SCHED_TARGET_FACE },
};

Schedule_t slMoveAwayFollow[] =
    {
        { tlMoveAwayFollow,
          ARRAYSIZE( tlMoveAwayFollow ),
          0,
          0,
          "MoveAwayFollow" },
};

Task_t tlTlkIdleWatchClient[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_TLK_LOOK_AT_CLIENT, (float)6 },
};

Task_t tlTlkIdleWatchClientStare[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_TLK_CLIENT_STARE, (float)6 },
        { TASK_TLK_STARE, (float)0 },
        { TASK_TLK_IDEALYAW, (float)0 }, // look at who I'm talking to
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 },
        { TASK_TLK_EYECONTACT, (float)0 },
};

Schedule_t slTlkIdleWatchClient[] =
    {
        { tlTlkIdleWatchClient,
          ARRAYSIZE( tlTlkIdleWatchClient ),
          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND |
              bits_COND_SMELL |
              bits_COND_CLIENT_PUSH |
              bits_COND_CLIENT_UNSEEN |
              bits_COND_PROVOKED,

          bits_SOUND_COMBAT | // sound flags - change these, and you'll break the talking code.
                              // bits_SOUND_PLAYER		|
                              // bits_SOUND_WORLD		|

              bits_SOUND_DANGER |
              bits_SOUND_MEAT | // scents
              bits_SOUND_CARCASS |
              bits_SOUND_GARBAGE,
          "TlkIdleWatchClient" },

        { tlTlkIdleWatchClientStare,
          ARRAYSIZE( tlTlkIdleWatchClientStare ),
          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND |
              bits_COND_SMELL |
              bits_COND_CLIENT_PUSH |
              bits_COND_CLIENT_UNSEEN |
              bits_COND_PROVOKED,

          bits_SOUND_COMBAT | // sound flags - change these, and you'll break the talking code.
                              // bits_SOUND_PLAYER		|
                              // bits_SOUND_WORLD		|

              bits_SOUND_DANGER |
              bits_SOUND_MEAT | // scents
              bits_SOUND_CARCASS |
              bits_SOUND_GARBAGE,
          "TlkIdleWatchClientStare" },
};

Task_t tlTlkIdleEyecontact[] =
    {
        { TASK_TLK_IDEALYAW, (float)0 }, // look at who I'm talking to
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_SET_ACTIVITY, (float)ACT_SIGNAL3 },
        { TASK_TLK_EYECONTACT, (float)0 }, // Wait until speaker is done
};

Schedule_t slTlkIdleEyecontact[] =
    {
        { tlTlkIdleEyecontact,
          ARRAYSIZE( tlTlkIdleEyecontact ),
          bits_COND_NEW_ENEMY |
              bits_COND_CLIENT_PUSH |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE,
          0,
          "TlkIdleEyecontact" },
};

DEFINE_CUSTOM_SCHEDULES( CTalkMonster ){
    slIdleResponse,
    slIdleSpeak,
    slIdleHello,
    slIdleSpeakWait,
    slIdleStopShooting,
    slMoveAway,
    slMoveAwayFollow,
    slMoveAwayFail,
    slTlkIdleWatchClient,
    &slTlkIdleWatchClient[1],
    slTlkIdleEyecontact,
};

IMPLEMENT_CUSTOM_SCHEDULES( CTalkMonster, CBaseMonster );

void CTalkMonster ::SetActivity( Activity newActivity )
{
	if ( newActivity == ACT_IDLE && IsTalking() )
		newActivity = ACT_SIGNAL3;

	if ( newActivity == ACT_SIGNAL3 && ( LookupActivity( ACT_SIGNAL3 ) == ACTIVITY_NOT_AVAILABLE ) )
		newActivity = ACT_IDLE;

	CBaseMonster::SetActivity( newActivity );
}

void CTalkMonster ::StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TLK_SPEAK:
		// ask question or make statement
		FIdleSpeak();
		TaskComplete();
		break;

	case TASK_TLK_RESPOND:
		// respond to question
		IdleRespond();
		TaskComplete();
		break;

	case TASK_TLK_HELLO:
		// greet player
		FIdleHello();
		TaskComplete();
		break;

	case TASK_TLK_STARE:
		// let the player know I know he's staring at me.
		FIdleStare();
		TaskComplete();
		break;

	case TASK_FACE_PLAYER:
	case TASK_TLK_LOOK_AT_CLIENT:
	case TASK_TLK_CLIENT_STARE:
		// track head to the client for a while.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;

	case TASK_TLK_EYECONTACT:
		break;

	case TASK_TLK_IDEALYAW:
		if ( m_hTalkTarget != NULL )
		{
			pev->yaw_speed = 60;
			float yaw      = VecToYaw( m_hTalkTarget->pev->origin - pev->origin ) - pev->angles.y;

			if ( yaw > 180 )
				yaw -= 360;
			if ( yaw < -180 )
				yaw += 360;

			if ( yaw < 0 )
			{
				pev->ideal_yaw = min( yaw + 45.0f, 0.0f ) + pev->angles.y;
			}
			else
			{
				pev->ideal_yaw = max( yaw - 45.0f, 0.0f ) + pev->angles.y;
			}
		}
		TaskComplete();
		break;

	case TASK_TLK_HEADRESET:
		// reset head position after looking at something
		m_hTalkTarget = NULL;
		TaskComplete();
		break;

	case TASK_TLK_STOPSHOOTING:
		// tell player to stop shooting
		PlaySentence( m_szGrp[TLK_NOSHOOT], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_NORM );
		TaskComplete();
		break;

	case TASK_CANT_FOLLOW:
		StopFollowing( FALSE );
		PlaySentence( m_szGrp[TLK_STOP], RANDOM_FLOAT( 2, 2.5 ), VOL_NORM, ATTN_NORM );
		TaskComplete();
		break;

	case TASK_WALK_PATH_FOR_UNITS:
		m_movementActivity = ACT_WALK;
		break;

	case TASK_MOVE_AWAY_PATH:
	{
		Vector dir = pev->angles;
		dir.y      = pev->ideal_yaw + 180;
		Vector move;

		UTIL_MakeVectorsPrivate( dir, move, NULL, NULL );
		dir = pev->origin + move * pTask->flData;
		if ( MoveToLocation( ACT_WALK, 2, dir ) )
		{
			TaskComplete();
		}
		else if ( FindCover( pev->origin, pev->view_ofs, 0, CoverRadius() ) )
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + 2;
			TaskComplete();
		}
		else
		{
			// nowhere to go?
			TaskFail();
		}
	}
	break;

	case TASK_PLAY_SCRIPT:
		m_hTalkTarget = NULL;
		CBaseMonster::StartTask( pTask );
		break;

	default:
		CBaseMonster::StartTask( pTask );
	}
}

void CTalkMonster ::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TLK_CLIENT_STARE:
	case TASK_TLK_LOOK_AT_CLIENT:

		edict_t *pPlayer;

		// track head to the client for a while.
		if ( m_MonsterState == MONSTERSTATE_IDLE &&
		     !IsMoving() &&
		     !IsTalking() )
		{
			// Get edict for one player
			pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

			if ( pPlayer )
			{
				IdleHeadTurn( pPlayer->v.origin );
			}
		}
		else
		{
			// started moving or talking
			TaskFail();
			return;
		}

		if ( pTask->iTask == TASK_TLK_CLIENT_STARE )
		{
			// fail out if the player looks away or moves away.
			if ( ( pPlayer->v.origin - pev->origin ).Length2D() > TLK_STARE_DIST )
			{
				// player moved away.
				TaskFail();
			}

			UTIL_MakeVectors( pPlayer->v.angles );
			if ( UTIL_DotPoints( pPlayer->v.origin, pev->origin, gpGlobals->v_forward ) < m_flFieldOfView )
			{
				// player looked away
				TaskFail();
			}
		}

		if ( gpGlobals->time > m_flWaitFinished )
		{
			TaskComplete();
		}
		break;

	case TASK_FACE_PLAYER:
	{
		// Get edict for one player
		edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

		if ( pPlayer )
		{
			MakeIdealYaw( pPlayer->v.origin );
			ChangeYaw( pev->yaw_speed );
			IdleHeadTurn( pPlayer->v.origin );
			if ( gpGlobals->time > m_flWaitFinished && FlYawDiff() < 10 )
			{
				TaskComplete();
			}
		}
		else
		{
			TaskFail();
		}
	}
	break;

	case TASK_TLK_EYECONTACT:
		if ( !IsMoving() && IsTalking() && m_hTalkTarget != NULL )
		{
			// ALERT( at_console, "waiting %f\n", m_flStopTalkTime - gpGlobals->time );
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			TaskComplete();
		}
		break;

	case TASK_WALK_PATH_FOR_UNITS:
	{
		float distance;

		distance = ( m_vecLastPosition - pev->origin ).Length2D();

		// Walk path until far enough away
		if ( distance > pTask->flData || MovementIsComplete() )
		{
			TaskComplete();
			RouteClear(); // Stop moving
		}
	}
	break;
	case TASK_WAIT_FOR_MOVEMENT:
		if ( IsTalking() && m_hTalkTarget != NULL )
		{
			// ALERT(at_console, "walking, talking\n");
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			IdleHeadTurn( pev->origin );
			// override so that during walk, a scientist may talk and greet player
			FIdleHello();
			if ( RANDOM_LONG( 0, m_nSpeak * 20 ) == 0 )
			{
				FIdleSpeak();
			}
		}

		CBaseMonster::RunTask( pTask );
		if ( TaskIsComplete() )
			IdleHeadTurn( pev->origin );
		break;

	default:
		if ( IsTalking() && m_hTalkTarget != NULL )
		{
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			SetBoneController( 0, 0 );
		}
		CBaseMonster::RunTask( pTask );
	}
}

void CTalkMonster ::Killed( entvars_t *pevAttacker, int iGib )
{
	// If a client killed me (unless I was already Barnacle'd), make everyone else mad/afraid of him
	if ( ( pevAttacker->flags & FL_CLIENT ) && m_MonsterState != MONSTERSTATE_PRONE )
	{
		AlertFriends();
		LimitFollowers( CBaseEntity::Instance( pevAttacker ), 0 );
	}

	m_hTargetEnt = NULL;
	// Don't finish that sentence
	StopTalking();
	SetUse( NULL );
	CBaseMonster::Killed( pevAttacker, iGib );
}


void CTalkMonster ::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
	case SCRIPT_EVENT_SENTENCE_RND1: // Play a named sentence group 25% of the time
		if ( RANDOM_LONG( 0, 99 ) < 75 )
			break;
		// fall through...
	case SCRIPT_EVENT_SENTENCE: // Play a named sentence group
		ShutUpFriends();
		PlaySentence( pEvent->options, RANDOM_FLOAT( 2.8, 3.4 ), VOL_NORM, ATTN_IDLE );
		// ALERT(at_console, "script event speak\n");
		break;

	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

// monsters derived from ctalkmonster should call this in precache()

void CTalkMonster ::TalkInit( void )
{
	// every new talking monster must reset this global, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)

	CTalkMonster::g_talkWaitTime = 0;

	m_voicePitch = 100;
}
//=========================================================
// FindNearestFriend
// Scan for nearest, visible friend. If fPlayer is true, look for
// nearest player
//=========================================================

int CTalkMonster ::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( IsAlive() )
	{
		// if player damaged this entity, have other friends talk about it
		if ( pevAttacker && m_MonsterState != MONSTERSTATE_PRONE && FBitSet( pevAttacker->flags, FL_CLIENT ) )
		{
			CBaseEntity *pFriend = FindNearestFriend( FALSE );

			if ( pFriend && pFriend->IsAlive() )
			{
				// only if not dead or dying!
				CTalkMonster *pTalkMonster = (CTalkMonster *)pFriend;
				pTalkMonster->ChangeSchedule( slIdleStopShooting );
			}
		}
	}
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

Schedule_t *CTalkMonster ::GetScheduleOfType( int Type )
{
	switch ( Type )
	{
	case SCHED_MOVE_AWAY:
		return slMoveAway;

	case SCHED_MOVE_AWAY_FOLLOW:
		return slMoveAwayFollow;

	case SCHED_MOVE_AWAY_FAIL:
		return slMoveAwayFail;

	case SCHED_TARGET_FACE:
		// speak during 'use'
		if ( RANDOM_LONG( 0, 99 ) < 2 )
			// ALERT ( at_console, "target chase speak\n" );
			return slIdleSpeakWait;
		else
			return slIdleStand;

	case SCHED_IDLE_STAND:
	{
		// if never seen player, try to greet him
		if ( !FBitSet( m_bitsSaid, bit_saidHelloPlayer ) )
		{
			return slIdleHello;
		}

		// sustained light wounds?
		if ( !FBitSet( m_bitsSaid, bit_saidWoundLight ) && ( pev->health <= ( pev->max_health * 0.75 ) ) )
		{
			// SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_WOUND], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
			// CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
			PlaySentence( m_szGrp[TLK_WOUND], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
			SetBits( m_bitsSaid, bit_saidWoundLight );
			return slIdleStand;
		}
		// sustained heavy wounds?
		else if ( !FBitSet( m_bitsSaid, bit_saidWoundHeavy ) && ( pev->health <= ( pev->max_health * 0.5 ) ) )
		{
			// SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_MORTAL], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
			// CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
			PlaySentence( m_szGrp[TLK_MORTAL], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
			SetBits( m_bitsSaid, bit_saidWoundHeavy );
			return slIdleStand;
		}

		// talk about world
		if ( FOkToSpeak() && RANDOM_LONG( 0, m_nSpeak * 2 ) == 0 )
		{
			// ALERT ( at_console, "standing idle speak\n" );
			return slIdleSpeak;
		}

		if ( !IsTalking() && HasConditions( bits_COND_SEE_CLIENT ) && RANDOM_LONG( 0, 6 ) == 0 )
		{
			edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

			if ( pPlayer )
			{
				// watch the client.
				UTIL_MakeVectors( pPlayer->v.angles );
				if ( ( pPlayer->v.origin - pev->origin ).Length2D() < TLK_STARE_DIST &&
				     UTIL_DotPoints( pPlayer->v.origin, pev->origin, gpGlobals->v_forward ) >= m_flFieldOfView )
				{
					// go into the special STARE schedule if the player is close, and looking at me too.
					return &slTlkIdleWatchClient[1];
				}

				return slTlkIdleWatchClient;
			}
		}
		else
		{
			if ( IsTalking() )
				// look at who we're talking to
				return slTlkIdleEyecontact;
			else
				// regular standing idle
				return slIdleStand;
		}

		// NOTE - caller must first CTalkMonster::GetScheduleOfType,
		// then check result and decide what to return ie: if sci gets back
		// slIdleStand, return slIdleSciStand
	}
	break;
	}

	return CBaseMonster::GetScheduleOfType( Type );
}

//=========================================================
// IsTalking - am I saying a sentence right now?
//=========================================================

int CTalkMonster::IRelationship( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() )
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return R_HT;
	return CBaseMonster::IRelationship( pTarget );
}

void CTalkMonster::StopFollowing( BOOL clearSchedule )
{
	if ( IsFollowing() )
	{
		if ( !( m_afMemory & bits_MEMORY_PROVOKED ) )
		{
			PlaySentence( m_szGrp[TLK_UNUSE], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
			m_hTalkTarget = m_hTargetEnt;
		}

		if ( m_movementGoal == MOVEGOAL_TARGETENT )
			RouteClear(); // Stop him from walking toward the player
		m_hTargetEnt = NULL;
		if ( clearSchedule )
			ClearSchedule();
		if ( m_hEnemy != NULL )
			m_IdealMonsterState = MONSTERSTATE_COMBAT;
	}
}

void CTalkMonster::StartFollowing( CBaseEntity *pLeader )
{
	if ( m_pCine )
		m_pCine->CancelScript();

	if ( m_hEnemy != NULL )
		m_IdealMonsterState = MONSTERSTATE_ALERT;

	m_hTargetEnt = pLeader;
	PlaySentence( m_szGrp[TLK_USE], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
	m_hTalkTarget = m_hTargetEnt;
	ClearConditions( bits_COND_CLIENT_PUSH );
	ClearSchedule();
}

BOOL CTalkMonster::CanFollow( void )
{
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		if ( !m_pCine || !m_pCine->CanInterrupt() )
			return FALSE;
	}

	if ( !IsAlive() )
		return FALSE;

	return !IsFollowing();
}

void CTalkMonster ::FollowerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Don't allow use during a scripted_sentence
	if ( m_useTime > gpGlobals->time )
		return;

	if ( pCaller != NULL && pCaller->IsPlayer() )
	{
		// Pre-disaster followers can't be used
		if ( pev->spawnflags & SF_MONSTER_PREDISASTER )
		{
			DeclineFollowing();
		}
		else if ( CanFollow() )
		{
			LimitFollowers( pCaller, 1 );

			if ( m_afMemory & bits_MEMORY_PROVOKED )
				ALERT( at_console, "I'm not following you, you evil person!\n" );
			else
			{
				StartFollowing( pCaller );
				SetBits( m_bitsSaid, bit_saidHelloPlayer ); // Don't say hi after you've started following
			}
		}
		else
		{
			StopFollowing( TRUE );
		}
	}
}

void CTalkMonster::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "UseSentence" ) )
	{
		m_iszUse       = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "UnUseSentence" ) )
	{
		m_iszUnUse     = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue( pkvd );
}

void CTalkMonster::Precache( void )
{
	if ( m_iszUse )
		m_szGrp[TLK_USE] = STRING( m_iszUse );
	if ( m_iszUnUse )
		m_szGrp[TLK_UNUSE] = STRING( m_iszUnUse );
}
