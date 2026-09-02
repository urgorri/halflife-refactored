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
#include "ai/monsters.h"
#include "ai/schedule.h"
#include "ai/talkmonster.h"
#include "ai/defaultai.h"
#include "ai/soundent.h"
#include "core/animation.h"

extern Schedule_t slIdleResponse[];
CBaseEntity *CTalkMonster::EnumFriends( CBaseEntity *pPrevious, int listNumber, BOOL bTrace )
{
	CBaseEntity *pFriend = pPrevious;
	char *pszFriend;
	TraceResult tr;
	Vector vecCheck;

	pszFriend = m_szFriends[FriendNumber( listNumber )];
	while ( pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ) )
	{
		if ( pFriend == this || !pFriend->IsAlive() )
			// don't talk to self or dead people
			continue;
		if ( bTrace )
		{
			vecCheck   = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			UTIL_TraceLine( pev->origin, vecCheck, ignore_monsters, ENT( pev ), &tr );
		}
		else
			tr.flFraction = 1.0;

		if ( tr.flFraction == 1.0 )
		{
			return pFriend;
		}
	}

	return NULL;
}

void CTalkMonster::AlertFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while ( pFriend = EnumFriends( pFriend, i, TRUE ) )
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster->IsAlive() )
			{
				// don't provoke a friend that's playing a death animation. They're a goner
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
			}
		}
	}
}

void CTalkMonster::ShutUpFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while ( pFriend = EnumFriends( pFriend, i, TRUE ) )
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				pMonster->SentenceStop();
			}
		}
	}
}

// UNDONE: Keep a follow time in each follower, make a list of followers in this function and do LRU
// UNDONE: Check this in Restore to keep restored monsters from joining a full list of followers
void CTalkMonster::LimitFollowers( CBaseEntity *pPlayer, int maxFollowers )
{
	CBaseEntity *pFriend = NULL;
	int i, count;

	count = 0;
	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while ( pFriend = EnumFriends( pFriend, i, FALSE ) )
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				if ( pMonster->m_hTargetEnt == pPlayer )
				{
					count++;
					if ( count > maxFollowers )
						pMonster->StopFollowing( TRUE );
				}
			}
		}
	}
}

float CTalkMonster::TargetDistance( void )
{
	// If we lose the player, or he dies, return a really large distance
	if ( m_hTargetEnt == NULL || !m_hTargetEnt->IsAlive() )
		return 1e6;

	return ( m_hTargetEnt->pev->origin - pev->origin ).Length();
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================

CBaseEntity *CTalkMonster ::FindNearestFriend( BOOL fPlayer )
{
	CBaseEntity *pFriend  = NULL;
	CBaseEntity *pNearest = NULL;
	float range           = 10000000.0;
	TraceResult tr;
	Vector vecStart = pev->origin;
	Vector vecCheck;
	int i;
	char *pszFriend;
	int cfriends;

	vecStart.z = pev->absmax.z;

	if ( fPlayer )
		cfriends = 1;
	else
		cfriends = TLK_CFRIENDS;

	// for each type of friend...

	for ( i = cfriends - 1; i > -1; i-- )
	{
		if ( fPlayer )
			pszFriend = "player";
		else
			pszFriend = m_szFriends[FriendNumber( i )];

		if ( !pszFriend )
			continue;

		// for each friend in this bsp...
		while ( pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ) )
		{
			if ( pFriend == this || !pFriend->IsAlive() )
				// don't talk to self or dead people
				continue;

			CBaseMonster *pMonster = pFriend->MyMonsterPointer();

			// If not a monster for some reason, or in a script, or prone
			if ( !pMonster || pMonster->m_MonsterState == MONSTERSTATE_SCRIPT || pMonster->m_MonsterState == MONSTERSTATE_PRONE )
				continue;

			vecCheck   = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			// if closer than previous friend, and in range, see if he's visible

			if ( range > ( vecStart - vecCheck ).Length() )
			{
				UTIL_TraceLine( vecStart, vecCheck, ignore_monsters, ENT( pev ), &tr );

				if ( tr.flFraction == 1.0 )
				{
					// visible and in range, this is the new nearest scientist
					if ( ( vecStart - vecCheck ).Length() < TALKRANGE_MIN )
					{
						pNearest = pFriend;
						range    = ( vecStart - vecCheck ).Length();
					}
				}
			}
		}
	}
	return pNearest;
}

int CTalkMonster ::GetVoicePitch( void )
{
	return m_voicePitch + RANDOM_LONG( 0, 3 );
}

void CTalkMonster ::Touch( CBaseEntity *pOther )
{
	// Did the player touch me?
	if ( pOther->IsPlayer() )
	{
		// Ignore if pissed at player
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return;

		// Stay put during speech
		if ( IsTalking() )
			return;

		// Heuristic for determining if the player is pushing me away
		float speed = fabs( pOther->pev->velocity.x ) + fabs( pOther->pev->velocity.y );
		if ( speed > 50 )
		{
			SetConditions( bits_COND_CLIENT_PUSH );
			MakeIdealYaw( pOther->pev->origin );
		}
	}
}

//=========================================================
// IdleRespond
// Respond to a previous question
//=========================================================
void CTalkMonster ::IdleRespond( void )
{
	int pitch = GetVoicePitch();

	// play response
	PlaySentence( m_szGrp[TLK_ANSWER], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
}

int CTalkMonster ::FOkToSpeak( void )
{
	// if in the grip of a barnacle, don't speak
	if ( m_MonsterState == MONSTERSTATE_PRONE || m_IdealMonsterState == MONSTERSTATE_PRONE )
	{
		return FALSE;
	}

	// if not alive, certainly don't speak
	if ( pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	// if someone else is talking, don't speak
	if ( gpGlobals->time <= CTalkMonster::g_talkWaitTime )
		return FALSE;

	if ( pev->spawnflags & SF_MONSTER_GAG )
		return FALSE;

	if ( m_MonsterState == MONSTERSTATE_PRONE )
		return FALSE;

	// if player is not in pvs, don't speak
	if ( !IsAlive() || FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) )
		return FALSE;

	// don't talk if you're in combat
	if ( m_hEnemy != NULL && FVisible( m_hEnemy ) )
		return FALSE;

	return TRUE;
}

int CTalkMonster::CanPlaySentence( BOOL fDisregardState )
{
	if ( fDisregardState )
		return CBaseMonster::CanPlaySentence( fDisregardState );
	return FOkToSpeak();
}

//=========================================================
// FIdleStare
//=========================================================
int CTalkMonster ::FIdleStare( void )
{
	if ( !FOkToSpeak() )
		return FALSE;

	PlaySentence( m_szGrp[TLK_STARE], RANDOM_FLOAT( 5, 7.5 ), VOL_NORM, ATTN_IDLE );

	m_hTalkTarget = FindNearestFriend( TRUE );
	return TRUE;
}

//=========================================================
// IdleHello
// Try to greet player first time he's seen
//=========================================================
int CTalkMonster ::FIdleHello( void )
{
	if ( !FOkToSpeak() )
		return FALSE;

	// if this is first time scientist has seen player, greet him
	if ( !FBitSet( m_bitsSaid, bit_saidHelloPlayer ) )
	{
		// get a player
		CBaseEntity *pPlayer = FindNearestFriend( TRUE );

		if ( pPlayer )
		{
			if ( FInViewCone( pPlayer ) && FVisible( pPlayer ) )
			{
				m_hTalkTarget = pPlayer;

				if ( FBitSet( pev->spawnflags, SF_MONSTER_PREDISASTER ) )
					PlaySentence( m_szGrp[TLK_PHELLO], RANDOM_FLOAT( 3, 3.5 ), VOL_NORM, ATTN_IDLE );
				else
					PlaySentence( m_szGrp[TLK_HELLO], RANDOM_FLOAT( 3, 3.5 ), VOL_NORM, ATTN_IDLE );

				SetBits( m_bitsSaid, bit_saidHelloPlayer );

				return TRUE;
			}
		}
	}
	return FALSE;
}

// turn head towards supplied origin
void CTalkMonster ::IdleHeadTurn( Vector &vecFriend )
{
	// turn head in desired direction only if ent has a turnable head
	if ( m_afCapability & bits_CAP_TURN_HEAD )
	{
		float yaw = VecToYaw( vecFriend - pev->origin ) - pev->angles.y;

		if ( yaw > 180 )
			yaw -= 360;
		if ( yaw < -180 )
			yaw += 360;

		// turn towards vector
		SetBoneController( 0, yaw );
	}
}

//=========================================================
// FIdleSpeak
// ask question of nearby friend, or make statement
//=========================================================
int CTalkMonster ::FIdleSpeak( void )
{
	// try to start a conversation, or make statement
	int pitch;
	const char *szIdleGroup;
	const char *szQuestionGroup;
	float duration;

	if ( !FOkToSpeak() )
		return FALSE;

	// set idle groups based on pre/post disaster
	if ( FBitSet( pev->spawnflags, SF_MONSTER_PREDISASTER ) )
	{
		szIdleGroup     = m_szGrp[TLK_PIDLE];
		szQuestionGroup = m_szGrp[TLK_PQUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT( 4.8, 5.2 );
	}
	else
	{
		szIdleGroup     = m_szGrp[TLK_IDLE];
		szQuestionGroup = m_szGrp[TLK_QUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT( 2.8, 3.2 );
	}

	pitch = GetVoicePitch();

	// player using this entity is alive and wounded?
	CBaseEntity *pTarget = m_hTargetEnt;

	if ( pTarget != NULL )
	{
		if ( pTarget->IsPlayer() )
		{
			if ( pTarget->IsAlive() )
			{
				m_hTalkTarget = m_hTargetEnt;
				if ( !FBitSet( m_bitsSaid, bit_saidDamageHeavy ) &&
				     ( m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 8 ) )
				{
					// EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT3], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT3], duration, VOL_NORM, ATTN_IDLE );
					SetBits( m_bitsSaid, bit_saidDamageHeavy );
					return TRUE;
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidDamageMedium ) &&
				          ( m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 4 ) )
				{
					// EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT2], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT2], duration, VOL_NORM, ATTN_IDLE );
					SetBits( m_bitsSaid, bit_saidDamageMedium );
					return TRUE;
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidDamageLight ) &&
				          ( m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 2 ) )
				{
					// EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT1], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT1], duration, VOL_NORM, ATTN_IDLE );
					SetBits( m_bitsSaid, bit_saidDamageLight );
					return TRUE;
				}
			}
			else
			{
				//!!!KELLY - here's a cool spot to have the talkmonster talk about the dead player if we want.
				// "Oh dear, Gordon Freeman is dead!" -Scientist
				// "Damn, I can't do this without you." -Barney
			}
		}
	}

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	CBaseEntity *pFriend = FindNearestFriend( FALSE );

	if ( pFriend && !( pFriend->IsMoving() ) && ( RANDOM_LONG( 0, 99 ) < 75 ) )
	{
		PlaySentence( szQuestionGroup, duration, VOL_NORM, ATTN_IDLE );
		// SENTENCEG_PlayRndSz( ENT(pev), szQuestionGroup, 1.0, ATTN_IDLE, 0, pitch );

		// force friend to answer
		CTalkMonster *pTalkMonster = (CTalkMonster *)pFriend;
		m_hTalkTarget              = pFriend;
		pTalkMonster->SetAnswerQuestion( this ); // UNDONE: This is EVIL!!!
		pTalkMonster->m_flStopTalkTime = m_flStopTalkTime;

		m_nSpeak++;
		return TRUE;
	}

	// otherwise, play an idle statement, try to face client when making a statement.
	if ( RANDOM_LONG( 0, 1 ) )
	{
		// SENTENCEG_PlayRndSz( ENT(pev), szIdleGroup, 1.0, ATTN_IDLE, 0, pitch );
		CBaseEntity *pFriend = FindNearestFriend( TRUE );

		if ( pFriend )
		{
			m_hTalkTarget = pFriend;
			PlaySentence( szIdleGroup, duration, VOL_NORM, ATTN_IDLE );
			m_nSpeak++;
			return TRUE;
		}
	}

	// didn't speak
	Talk( 0 );
	CTalkMonster::g_talkWaitTime = 0;
	return FALSE;
}

void CTalkMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener )
{
	if ( !bConcurrent )
		ShutUpFriends();

	ClearConditions( bits_COND_CLIENT_PUSH ); // Forget about moving!  I've got something to say!
	m_useTime = gpGlobals->time + duration;
	PlaySentence( pszSentence, duration, volume, attenuation );

	m_hTalkTarget = pListener;
}

void CTalkMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation )
{
	if ( !pszSentence )
		return;

	Talk( duration );

	CTalkMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;
	if ( pszSentence[0] == '!' )
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, GetVoicePitch() );
	else
		SENTENCEG_PlayRndSz( edict(), pszSentence, volume, attenuation, 0, GetVoicePitch() );

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits( m_bitsSaid, bit_saidHelloPlayer );
}

//=========================================================
// Talk - set a timer that tells us when the monster is done
// talking.
//=========================================================
void CTalkMonster ::Talk( float flDuration )
{
	if ( flDuration <= 0 )
	{
		// no duration :(
		m_flStopTalkTime = gpGlobals->time + 3;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->time + flDuration;
	}
}

// Prepare this talking monster to answer question
void CTalkMonster ::SetAnswerQuestion( CTalkMonster *pSpeaker )
{
	if ( !m_pCine )
		ChangeSchedule( slIdleResponse );
	m_hTalkTarget = (CBaseMonster *)pSpeaker;
}


BOOL CTalkMonster ::IsTalking( void )
{
	if ( m_flStopTalkTime > gpGlobals->time )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// If there's a player around, watch him.
//=========================================================
void CTalkMonster ::PrescheduleThink( void )
{
	if ( !HasConditions( bits_COND_SEE_CLIENT ) )
	{
		SetConditions( bits_COND_CLIENT_UNSEEN );
	}
}

// try to smell something
void CTalkMonster ::TrySmellTalk( void )
{
	if ( !FOkToSpeak() )
		return;

	// clear smell bits periodically
	if ( gpGlobals->time > m_flLastSaidSmelled )
	{
		//		ALERT ( at_aiconsole, "Clear smell bits\n" );
		ClearBits( m_bitsSaid, bit_saidSmelled );
	}
	// smelled something?
	if ( !FBitSet( m_bitsSaid, bit_saidSmelled ) && HasConditions( bits_COND_SMELL ) )
	{
		PlaySentence( m_szGrp[TLK_SMELL], RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_IDLE );
		m_flLastSaidSmelled = gpGlobals->time + 60; // don't talk about the stinky for a while.
		SetBits( m_bitsSaid, bit_saidSmelled );
	}
}

