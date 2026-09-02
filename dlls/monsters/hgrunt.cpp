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
//=========================================================
// hgrunt
//=========================================================

//=========================================================
// Hit groups!
//=========================================================
/*

  1 - Head
  2 - Stomach
  3 - Gun

*/

#include "core/extdll.h"
#include "core/plane.h"
#include "core/util.h"
#include "core/cbase.h"
#include "ai/monsters.h"
#include "ai/schedule.h"
#include "core/animation.h"
#include "ai/squadmonster.h"
#include "weapons/weapon_base.h"
#include "ai/soundent.h"
#include "ai/talkmonster.h"
#include "systems/effects.h"
#include "customentity.h"

int g_fGruntQuestion; // true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int g_iSkillLevel;

#include "monsters/hgrunt.h"

LINK_ENTITY_TO_CLASS( monster_human_grunt, CHGrunt );

TYPEDESCRIPTION CHGrunt::m_SaveData[] =
    {
        DEFINE_FIELD( CHGrunt, m_flNextGrenadeCheck, FIELD_TIME ),
        DEFINE_FIELD( CHGrunt, m_flNextPainTime, FIELD_TIME ),
        //	DEFINE_FIELD( CHGrunt, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
        DEFINE_FIELD( CHGrunt, m_vecTossVelocity, FIELD_VECTOR ),
        DEFINE_FIELD( CHGrunt, m_fThrowGrenade, FIELD_BOOLEAN ),
        DEFINE_FIELD( CHGrunt, m_fStanding, FIELD_BOOLEAN ),
        DEFINE_FIELD( CHGrunt, m_fFirstEncounter, FIELD_BOOLEAN ),
        DEFINE_FIELD( CHGrunt, m_cClipSize, FIELD_INTEGER ),
        DEFINE_FIELD( CHGrunt, m_voicePitch, FIELD_INTEGER ),
        //  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
        //  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
        DEFINE_FIELD( CHGrunt, m_iSentence, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CHGrunt, CSquadMonster );

const char *CHGrunt::pGruntSentences[] =
    {
        "HG_GREN",    // grenade scared grunt
        "HG_ALERT",   // sees player
        "HG_MONSTER", // sees monster
        "HG_COVER",   // running to cover
        "HG_THROW",   // about to throw grenade
        "HG_CHARGE",  // running out to get the enemy
        "HG_TAUNT",   // say rude things
};

//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has
// started moving.
//=========================================================
void CHGrunt ::SpeakSentence( void )
{
	if ( m_iSentence == HGRUNT_SENT_NONE )
	{
		// no sentence cued up.
		return;
	}

	if ( FOkToSpeak() )
	{
		SENTENCEG_PlayRndSz( ENT( pev ), pGruntSentences[m_iSentence], HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are
// Human Grunt's nemesis.
//=========================================================
int CHGrunt::IRelationship( CBaseEntity *pTarget )
{
	if ( FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev, "monster_gargantua" ) ) )
	{
		return R_NM;
	}

	return CSquadMonster::IRelationship( pTarget );
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CHGrunt ::GibMonster( void )
{
	Vector vecGunPos;
	Vector vecGunAngles;

	if ( GetBodygroup( 2 ) != 2 )
	{ // throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );

		CBaseEntity *pGun;
		if ( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
		{
			pGun = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
		}
		else
		{
			pGun = DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
		}
		if ( pGun )
		{
			pGun->pev->velocity  = Vector( RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( 200, 300 ) );
			pGun->pev->avelocity = Vector( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}

		if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) )
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, vecGunAngles );
			if ( pGun )
			{
				pGun->pev->velocity  = Vector( RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( 200, 300 ) );
				pGun->pev->avelocity = Vector( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			}
		}
	}

	CBaseMonster ::GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHGrunt ::ISoundMask( void )
{
	return bits_SOUND_WORLD |
	       bits_SOUND_COMBAT |
	       bits_SOUND_PLAYER |
	       bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CHGrunt ::FOkToSpeak( void )
{
	// if someone else is talking, don't speak
	if ( gpGlobals->time <= CTalkMonster::g_talkWaitTime )
		return FALSE;

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		if ( m_MonsterState != MONSTERSTATE_COMBAT )
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	// if player is not in pvs, don't speak
	//	if (FNullEnt(FIND_CLIENT_IN_PVS(edict())))
	//		return FALSE;

	return TRUE;
}

//=========================================================
//=========================================================
void CHGrunt ::JustSpoke( void )
{
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT( 1.5, 2.0 );
	m_iSentence                  = HGRUNT_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHGrunt ::PrescheduleThink( void )
{
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}
}

//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds.
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHGrunt ::SetYawSpeed( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:
		ys = 150;
		break;
	case ACT_RUN:
		ys = 150;
		break;
	case ACT_WALK:
		ys = 180;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 120;
		break;
	case ACT_RANGE_ATTACK2:
		ys = 120;
		break;
	case ACT_MELEE_ATTACK1:
		ys = 120;
		break;
	case ACT_MELEE_ATTACK2:
		ys = 120;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

void CHGrunt ::IdleSound( void )
{
	if ( FOkToSpeak() && ( g_fGruntQuestion || RANDOM_LONG( 0, 1 ) ) )
	{
		if ( !g_fGruntQuestion )
		{
			// ask question or make statement
			switch ( RANDOM_LONG( 0, 2 ) )
			{
			case 0: // check in
				SENTENCEG_PlayRndSz( ENT( pev ), "HG_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz( ENT( pev ), "HG_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz( ENT( pev ), "HG_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			}
		}
		else
		{
			switch ( g_fGruntQuestion )
			{
			case 1: // check in
				SENTENCEG_PlayRndSz( ENT( pev ), "HG_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			case 2: // question
				SENTENCEG_PlayRndSz( ENT( pev ), "HG_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================

int CHGrunt ::Classify( void )
{
	return CLASS_HUMAN_MILITARY;
}

//=========================================================
//=========================================================

void CHGrunt ::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector vecShootDir;
	Vector vecShootOrigin;

	switch ( pEvent->event )
	{
	case HGRUNT_AE_DROP_GUN:
	{
		Vector vecGunPos;
		Vector vecGunAngles;

		GetAttachment( 0, vecGunPos, vecGunAngles );

		// switch to body group with no gun.
		SetBodygroup( GUN_GROUP, GUN_NONE );

		// now spawn a gun.
		if ( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
		{
			DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
		}
		else
		{
			DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
		}
		if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) )
		{
			DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
		}
	}
	break;

	case HGRUNT_AE_RELOAD:
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions( bits_COND_NO_AMMO_LOADED );
		break;

	case HGRUNT_AE_GREN_TOSS:
	{
		UTIL_MakeVectors( pev->angles );
		// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
		CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

		m_fThrowGrenade      = FALSE;
		m_flNextGrenadeCheck = gpGlobals->time + 6; // wait six seconds before even looking again to see if a grenade can be thrown.
		                                            // !!!LATER - when in a group, only try to throw grenade if ordered.
	}
	break;

	case HGRUNT_AE_GREN_LAUNCH:
	{
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM );
		CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
		m_fThrowGrenade = FALSE;
		if ( g_iSkillLevel == SKILL_HARD )
			m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 ); // wait a random amount of time before shooting again
		else
			m_flNextGrenadeCheck = gpGlobals->time + 6; // wait six seconds before even looking again to see if a grenade can be thrown.
	}
	break;

	case HGRUNT_AE_GREN_DROP:
	{
		UTIL_MakeVectors( pev->angles );
		CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
	}
	break;

	case HGRUNT_AE_BURST1:
	{
		if ( FBitSet( pev->weapons, HGRUNT_9MMAR ) )
		{
			Shoot();

			// the first round of the three round burst plays the sound and puts a sound in the world sound list.
			if ( RANDOM_LONG( 0, 1 ) )
			{
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_mgun1.wav", 1, ATTN_NORM );
			}
			else
			{
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_mgun2.wav", 1, ATTN_NORM );
			}
		}
		else
		{
			Shotgun();

			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, ATTN_NORM );
		}

		CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
	}
	break;

	case HGRUNT_AE_BURST2:
	case HGRUNT_AE_BURST3:
		Shoot();
		break;

	case HGRUNT_AE_KICK:
	{
		CBaseEntity *pHurt = Kick();

		if ( pHurt )
		{
			// SOUND HERE!
			UTIL_MakeVectors( pev->angles );
			pHurt->pev->punchangle.x = 15;
			pHurt->pev->velocity     = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
			pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );
		}
	}
	break;

	case HGRUNT_AE_CAUGHT_ENEMY:
	{
		if ( FOkToSpeak() )
		{
			SENTENCEG_PlayRndSz( ENT( pev ), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
			JustSpoke();
		}
	}

	default:
		CSquadMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHGrunt ::Spawn()
{
	Precache();

	SET_MODEL( ENT( pev ), "models/hgrunt.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid           = SOLID_SLIDEBOX;
	pev->movetype        = MOVETYPE_STEP;
	m_bloodColor         = BLOOD_COLOR_RED;
	pev->effects         = 0;
	pev->health          = gSkillData.hgruntHealth;
	m_flFieldOfView      = 0.2; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState       = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime     = gpGlobals->time;
	m_iSentence          = HGRUNT_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded    = FALSE;
	m_fFirstEncounter = TRUE; // this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector( 0, 0, 55 );

	if ( pev->weapons == 0 )
	{
		// initialize to original values
		pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;
		// pev->weapons = HGRUNT_SHOTGUN;
		// pev->weapons = HGRUNT_9MMAR | HGRUNT_GRENADELAUNCHER;
	}

	if ( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
	{
		SetBodygroup( GUN_GROUP, GUN_SHOTGUN );
		m_cClipSize = 8;
	}
	else
	{
		m_cClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_cClipSize;

	if ( RANDOM_LONG( 0, 99 ) < 80 )
		pev->skin = 0; // light skin
	else
		pev->skin = 1; // dark skin

	if ( FBitSet( pev->weapons, HGRUNT_SHOTGUN ) )
	{
		SetBodygroup( HEAD_GROUP, HEAD_SHOTGUN );
	}
	else if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) )
	{
		SetBodygroup( HEAD_GROUP, HEAD_M203 );
		pev->skin = 1; // alway dark skin
	}

	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHGrunt ::Precache()
{
	PRECACHE_MODEL( "models/hgrunt.mdl" );

	PRECACHE_SOUND( "hgrunt/gr_mgun1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_mgun2.wav" );

	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	PRECACHE_SOUND( "hgrunt/gr_pain1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain3.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain4.wav" );
	PRECACHE_SOUND( "hgrunt/gr_pain5.wav" );

	PRECACHE_SOUND( "hgrunt/gr_reload1.wav" );

	PRECACHE_SOUND( "weapons/glauncher.wav" );

	PRECACHE_SOUND( "weapons/sbarrel1.wav" );

	PRECACHE_SOUND( "zombie/claw_miss2.wav" ); // because we use the basemonster SWIPE animation event

	// get voice pitch
	if ( RANDOM_LONG( 0, 1 ) )
		m_voicePitch = 109 + RANDOM_LONG( 0, 7 );
	else
		m_voicePitch = 100;

	m_iBrassShell   = PRECACHE_MODEL( "models/shell.mdl" ); // brass shell
	m_iShotgunShell = PRECACHE_MODEL( "models/shotgunshell.mdl" );
}

//=========================================================
// start task
//=========================================================
void CHGrunt ::StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_GRUNT_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_GRUNT_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_GRUNT_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget( bits_MEMORY_INCOVER );
		CSquadMonster ::StartTask( pTask );
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		CSquadMonster ::StartTask( pTask );
		if ( pev->movetype == MOVETYPE_FLY )
		{
			m_IdealActivity = ACT_GLIDE;
		}
		break;

	default:
		CSquadMonster ::StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CHGrunt ::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
	{
		// project a point along the toss vector and turn to face that point.
		MakeIdealYaw( pev->origin + m_vecTossVelocity * 64 );
		ChangeYaw( pev->yaw_speed );

		if ( FacingIdeal() )
		{
			m_iTaskStatus = TASKSTATUS_COMPLETE;
		}
		break;
	}
	default:
	{
		CSquadMonster ::RunTask( pTask );
		break;
	}
	}
}

//=========================================================
// PainSound
//=========================================================
void CHGrunt ::PainSound( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "HG_PAIN", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif
		switch ( RANDOM_LONG( 0, 6 ) )
		{
		case 0:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_pain3.wav", 1, ATTN_NORM );
			break;
		case 1:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_pain4.wav", 1, ATTN_NORM );
			break;
		case 2:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_pain5.wav", 1, ATTN_NORM );
			break;
		case 3:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_pain1.wav", 1, ATTN_NORM );
			break;
		case 4:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_pain2.wav", 1, ATTN_NORM );
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound
//=========================================================
void CHGrunt ::DeathSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_IDLE );
		break;
	case 1:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_IDLE );
		break;
	case 2:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_IDLE );
		break;
	}
}
