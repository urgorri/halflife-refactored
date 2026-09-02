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
#include "ai/talkmonster.h"
#include "ai/schedule.h"
#include "ai/defaultai.h"
#include "core/animation.h"
#include "ai/soundent.h"
#include "monsters/scientist.h"

BOOL CScientist::CanHeal( void )
{
	if ( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) || ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5 ) ) )
		return FALSE;

	return TRUE;
}

void CScientist::Heal( void )
{
	if ( !CanHeal() )
		return;

	Vector target = m_hTargetEnt->pev->origin - pev->origin;
	if ( target.Length() > 100 )
		return;

	m_hTargetEnt->TakeHealth( gSkillData.scientistHeal, DMG_GENERIC );
	// Don't heal again for 1 minute
	m_healTime = gpGlobals->time + 60;
}

int CScientist::FriendNumber( int arrayNumber )
{
	static int array[3] = { 1, 2, 0 };
	if ( arrayNumber < 3 )
		return array[arrayNumber];
	return arrayNumber;
}

//=========================================================
// Dead Scientist PROP
//=========================================================
char *CDeadScientist::m_szPoses[] = { "lying_on_back", "lying_on_stomach", "dead_sitting", "dead_hang", "dead_table1", "dead_table2", "dead_table3" };

char *CDeadScientist::GetScientistModel() const
{
	char *pszOverride = (char *)CVAR_GET_STRING( "_sv_override_scientist_mdl" );
	if ( pszOverride && strlen( pszOverride ) > 5 ) // at least requires ".mdl"
	{
		return pszOverride;
	}

	return "models/scientist.mdl";
}

void CDeadScientist::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "pose" ) )
	{
		m_iPose        = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue( pkvd );
}
LINK_ENTITY_TO_CLASS( monster_scientist_dead, CDeadScientist );

//
// ********** DeadScientist SPAWN **********
//
void CDeadScientist ::Spawn()
{
	PRECACHE_MODEL( GetScientistModel() );
	SET_MODEL( ENT( pev ), GetScientistModel() );

	pev->effects  = 0;
	pev->sequence = 0;
	// Corpses have less health
	pev->health = 8; // gSkillData.scientistHealth;

	m_bloodColor = BLOOD_COLOR_RED;

	if ( pev->body == -1 )
	{                                                          // -1 chooses a random head
		pev->body = RANDOM_LONG( 0, NUM_SCIENTIST_HEADS - 1 ); // pick a head, any head
	}
	// Luther is black, make his hands black
	if ( pev->body == HEAD_LUTHER )
		pev->skin = 1;
	else
		pev->skin = 0;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if ( pev->sequence == -1 )
	{
		ALERT( at_console, "Dead scientist with bad pose\n" );
	}

	//	pev->skin += 2; // use bloody skin -- UNDONE: Turn this back on when we have a bloody skin again!
	MonsterInitDead();
}

//=========================================================
// Sitting Scientist PROP
//=========================================================
LINK_ENTITY_TO_CLASS( monster_sitting_scientist, CSittingScientist );
TYPEDESCRIPTION CSittingScientist::m_SaveData[] =
    {
        // Don't need to save/restore m_baseSequence (recalced)
        DEFINE_FIELD( CSittingScientist, m_headTurn, FIELD_INTEGER ),
        DEFINE_FIELD( CSittingScientist, m_flResponseDelay, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CSittingScientist, CScientist );

// animation sequence aliases
typedef enum
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
} SITTING_ANIM;

//
// ********** Scientist SPAWN **********
//
void CSittingScientist ::Spawn()
{
	PRECACHE_MODEL( GetScientistModel() );
	SET_MODEL( ENT( pev ), GetScientistModel() );
	Precache();
	InitBoneControllers();

	UTIL_SetSize( pev, Vector( -14, -14, 0 ), Vector( 14, 14, 36 ) );

	pev->solid    = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects  = 0;
	pev->health   = 50;

	m_bloodColor    = BLOOD_COLOR_RED;
	m_flFieldOfView = VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	SetBits( pev->spawnflags, SF_MONSTER_PREDISASTER ); // predisaster only!

	if ( pev->body == -1 )
	{                                                          // -1 chooses a random head
		pev->body = RANDOM_LONG( 0, NUM_SCIENTIST_HEADS - 1 ); // pick a head, any head
	}
	// Luther is black, make his hands black
	if ( pev->body == HEAD_LUTHER )
		pev->skin = 1;

	m_baseSequence = LookupSequence( "sitlookleft" );
	pev->sequence  = m_baseSequence + RANDOM_LONG( 0, 4 );
	ResetSequenceInfo();

	SetThink( &CSittingScientist::SittingThink );
	pev->nextthink = gpGlobals->time + 0.1;

	DROP_TO_FLOOR( ENT( pev ) );
}

void CSittingScientist ::Precache( void )
{
	m_baseSequence = LookupSequence( "sitlookleft" );
	TalkInit();
}

//=========================================================
// ID as a passive human
//=========================================================
int CSittingScientist ::Classify( void )
{
	return CLASS_HUMAN_PASSIVE;
}

int CSittingScientist::FriendNumber( int arrayNumber )
{
	static int array[3] = { 2, 1, 0 };
	if ( arrayNumber < 3 )
		return array[arrayNumber];
	return arrayNumber;
}

//=========================================================
// sit, do stuff
//=========================================================
void CSittingScientist ::SittingThink( void )
{
	CBaseEntity *pent;

	StudioFrameAdvance();

	// try to greet player
	if ( FIdleHello() )
	{
		pent = FindNearestFriend( TRUE );
		if ( pent )
		{
			float yaw = VecToYaw( pent->pev->origin - pev->origin ) - pev->angles.y;

			if ( yaw > 180 )
				yaw -= 360;
			if ( yaw < -180 )
				yaw += 360;

			if ( yaw > 0 )
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
			else
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

			ResetSequenceInfo();
			pev->frame = 0;
			SetBoneController( 0, 0 );
		}
	}
	else if ( m_fSequenceFinished )
	{
		int i      = RANDOM_LONG( 0, 99 );
		m_headTurn = 0;

		if ( m_flResponseDelay && gpGlobals->time > m_flResponseDelay )
		{
			// respond to question
			IdleRespond();
			pev->sequence     = m_baseSequence + SITTING_ANIM_sitscared;
			m_flResponseDelay = 0;
		}
		else if ( i < 30 )
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;

			// turn towards player or nearest friend and speak

			if ( !FBitSet( m_bitsSaid, bit_saidHelloPlayer ) )
				pent = FindNearestFriend( TRUE );
			else
				pent = FindNearestFriend( FALSE );

			if ( !FIdleSpeak() || !pent )
			{
				m_headTurn    = RANDOM_LONG( 0, 8 ) * 10 - 40;
				pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			}
			else
			{
				// only turn head if we spoke
				float yaw = VecToYaw( pent->pev->origin - pev->origin ) - pev->angles.y;

				if ( yaw > 180 )
					yaw -= 360;
				if ( yaw < -180 )
					yaw += 360;

				if ( yaw > 0 )
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
				else
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

				// ALERT(at_console, "sitting speak\n");
			}
		}
		else if ( i < 60 )
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			m_headTurn    = RANDOM_LONG( 0, 8 ) * 10 - 40;
			if ( RANDOM_LONG( 0, 99 ) < 5 )
			{
				// ALERT(at_console, "sitting speak2\n");
				FIdleSpeak();
			}
		}
		else if ( i < 80 )
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting2;
		}
		else if ( i < 100 )
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
		}

		ResetSequenceInfo();
		pev->frame = 0;
		SetBoneController( 0, m_headTurn );
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

// prepare sitting scientist to answer a question
void CSittingScientist ::SetAnswerQuestion( CTalkMonster *pSpeaker )
{
	m_flResponseDelay = gpGlobals->time + RANDOM_FLOAT( 3, 4 );
	m_hTalkTarget     = (CBaseMonster *)pSpeaker;
}

//=========================================================
// FIdleSpeak
// ask question of nearby friend, or make statement
//=========================================================
int CSittingScientist ::FIdleSpeak( void )
{
	// try to start a conversation, or make statement
	int pitch;

	if ( !FOkToSpeak() )
		return FALSE;

	// set global min delay for next conversation
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT( 4.8, 5.2 );

	pitch = GetVoicePitch();

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return

	// try to talk to any standing or sitting scientists nearby
	CBaseEntity *pentFriend = FindNearestFriend( FALSE );

	if ( pentFriend && RANDOM_LONG( 0, 1 ) )
	{
		CTalkMonster *pTalkMonster = GetClassPtr( (CTalkMonster *)pentFriend->pev );
		pTalkMonster->SetAnswerQuestion( this );

		IdleHeadTurn( pentFriend->pev->origin );
		SENTENCEG_PlayRndSz( ENT( pev ), m_szGrp[TLK_PQUESTION], 1.0, ATTN_IDLE, 0, pitch );
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT( 4.8, 5.2 );
		return TRUE;
	}

	// otherwise, play an idle statement
	if ( RANDOM_LONG( 0, 1 ) )
	{
		SENTENCEG_PlayRndSz( ENT( pev ), m_szGrp[TLK_PIDLE], 1.0, ATTN_IDLE, 0, pitch );
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT( 4.8, 5.2 );
		return TRUE;
	}

	// never spoke
	CTalkMonster::g_talkWaitTime = 0;
	return FALSE;
}
