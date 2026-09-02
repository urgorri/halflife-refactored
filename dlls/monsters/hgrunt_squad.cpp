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
#include "core/animation.h"
#include "weapons/weapon_base.h"
#include "ai/talkmonster.h"
#include "ai/soundent.h"
#include "ai/squadmonster.h"
#include "weapons/weapon_mp5.h"
#include "weapons/projectile_grenade.h"
#include "weapons/weapon_shotgun.h"
#include "systems/effects.h"
#include "monsters/hgrunt.h"

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t tlGruntFail[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_WAIT, (float)2 },
        { TASK_WAIT_PVS, (float)0 },
};

Schedule_t slGruntFail[] =
    {
        { tlGruntFail,
          ARRAYSIZE( tlGruntFail ),
          bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2 |
              bits_COND_CAN_MELEE_ATTACK1 |
              bits_COND_CAN_MELEE_ATTACK2,
          0,
          "Grunt Fail" },
};

//=========================================================
// Grunt Combat Fail
//=========================================================
Task_t tlGruntCombatFail[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_WAIT_FACE_ENEMY, (float)2 },
        { TASK_WAIT_PVS, (float)0 },
};

Schedule_t slGruntCombatFail[] =
    {
        { tlGruntCombatFail,
          ARRAYSIZE( tlGruntCombatFail ),
          bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2,
          0,
          "Grunt Combat Fail" },
};

//=========================================================
// Victory dance!
//=========================================================
Task_t tlGruntVictoryDance[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_WAIT, (float)1.5 },
        { TASK_GET_PATH_TO_ENEMY_CORPSE, (float)0 },
        { TASK_WALK_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
};

Schedule_t slGruntVictoryDance[] =
    {
        { tlGruntVictoryDance,
          ARRAYSIZE( tlGruntVictoryDance ),
          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE,
          0,
          "GruntVictoryDance" },
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlGruntEstablishLineOfFire[] =
    {
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_GRUNT_ELOF_FAIL },
        { TASK_GET_PATH_TO_ENEMY, (float)0 },
        { TASK_GRUNT_SPEAK_SENTENCE, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
};

Schedule_t slGruntEstablishLineOfFire[] =
    {
        { tlGruntEstablishLineOfFire,
          ARRAYSIZE( tlGruntEstablishLineOfFire ),
          bits_COND_NEW_ENEMY |
              bits_COND_ENEMY_DEAD |
              bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_MELEE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2 |
              bits_COND_CAN_MELEE_ATTACK2 |
              bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER,
          "GruntEstablishLineOfFire" },
};

//=========================================================
// GruntFoundEnemy - grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t tlGruntFoundEnemy[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_SIGNAL1 },
};

Schedule_t slGruntFoundEnemy[] =
    {
        { tlGruntFoundEnemy,
          ARRAYSIZE( tlGruntFoundEnemy ),
          bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER,
          "GruntFoundEnemy" },
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t tlGruntCombatFace1[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_WAIT, (float)1.5 },
        { TASK_SET_SCHEDULE, (float)SCHED_GRUNT_SWEEP },
};

Schedule_t slGruntCombatFace[] =
    {
        { tlGruntCombatFace1,
          ARRAYSIZE( tlGruntCombatFace1 ),
          bits_COND_NEW_ENEMY |
              bits_COND_ENEMY_DEAD |
              bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2,
          0,
          "Combat Face" },
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
Task_t tlGruntSignalSuppress[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_SIGNAL2 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
};

Schedule_t slGruntSignalSuppress[] =
    {
        { tlGruntSignalSuppress,
          ARRAYSIZE( tlGruntSignalSuppress ),
          bits_COND_ENEMY_DEAD |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND |
              bits_COND_GRUNT_NOFIRE |
              bits_COND_NO_AMMO_LOADED,

          bits_SOUND_DANGER,
          "SignalSuppress" },
};

Task_t tlGruntSuppress[] =
    {
        { TASK_STOP_MOVING, 0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
};

Schedule_t slGruntSuppress[] =
    {
        { tlGruntSuppress,
          ARRAYSIZE( tlGruntSuppress ),
          bits_COND_ENEMY_DEAD |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND |
              bits_COND_GRUNT_NOFIRE |
              bits_COND_NO_AMMO_LOADED,

          bits_SOUND_DANGER,
          "Suppress" },
};

//=========================================================
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t tlGruntWaitInCover[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_SET_ACTIVITY, (float)ACT_IDLE },
        { TASK_WAIT_FACE_ENEMY, (float)1 },
};

Schedule_t slGruntWaitInCover[] =
    {
        { tlGruntWaitInCover,
          ARRAYSIZE( tlGruntWaitInCover ),
          bits_COND_NEW_ENEMY |
              bits_COND_HEAR_SOUND |
              bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2 |
              bits_COND_CAN_MELEE_ATTACK1 |
              bits_COND_CAN_MELEE_ATTACK2,

          bits_SOUND_DANGER,
          "GruntWaitInCover" },
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t tlGruntTakeCover1[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_GRUNT_TAKECOVER_FAILED },
        { TASK_WAIT, (float)0.2 },
        { TASK_FIND_COVER_FROM_ENEMY, (float)0 },
        { TASK_GRUNT_SPEAK_SENTENCE, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_REMEMBER, (float)bits_MEMORY_INCOVER },
        { TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY },
};

Schedule_t slGruntTakeCover[] =
    {
        { tlGruntTakeCover1,
          ARRAYSIZE( tlGruntTakeCover1 ),
          0,
          0,
          "TakeCover" },
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlGruntGrenadeCover1[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FIND_COVER_FROM_ENEMY, (float)99 },
        { TASK_FIND_FAR_NODE_COVER_FROM_ENEMY, (float)384 },
        { TASK_PLAY_SEQUENCE, (float)ACT_SPECIAL_ATTACK1 },
        { TASK_CLEAR_MOVE_WAIT, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY },
};

Schedule_t slGruntGrenadeCover[] =
    {
        { tlGruntGrenadeCover1,
          ARRAYSIZE( tlGruntGrenadeCover1 ),
          0,
          0,
          "GrenadeCover" },
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlGruntTossGrenadeCover1[] =
    {
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_RANGE_ATTACK2, (float)0 },
        { TASK_SET_SCHEDULE, (float)SCHED_TAKE_COVER_FROM_ENEMY },
};

Schedule_t slGruntTossGrenadeCover[] =
    {
        { tlGruntTossGrenadeCover1,
          ARRAYSIZE( tlGruntTossGrenadeCover1 ),
          0,
          0,
          "TossGrenadeCover" },
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t tlGruntTakeCoverFromBestSound[] =
    {
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_COWER }, // duck and cover if cannot move from explosion
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FIND_COVER_FROM_BEST_SOUND, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_REMEMBER, (float)bits_MEMORY_INCOVER },
        { TASK_TURN_LEFT, (float)179 },
};

Schedule_t slGruntTakeCoverFromBestSound[] =
    {
        { tlGruntTakeCoverFromBestSound,
          ARRAYSIZE( tlGruntTakeCoverFromBestSound ),
          0,
          0,
          "GruntTakeCoverFromBestSound" },
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t tlGruntHideReload[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_SET_FAIL_SCHEDULE, (float)SCHED_RELOAD },
        { TASK_FIND_COVER_FROM_ENEMY, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_REMEMBER, (float)bits_MEMORY_INCOVER },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_RELOAD },
};

Schedule_t slGruntHideReload[] =
    {
        { tlGruntHideReload,
          ARRAYSIZE( tlGruntHideReload ),
          bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER,
          "GruntHideReload" } };

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t tlGruntSweep[] =
    {
        { TASK_TURN_LEFT, (float)179 },
        { TASK_WAIT, (float)1 },
        { TASK_TURN_LEFT, (float)179 },
        { TASK_WAIT, (float)1 },
};

Schedule_t slGruntSweep[] =
    {
        { tlGruntSweep,
          ARRAYSIZE( tlGruntSweep ),

          bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_CAN_RANGE_ATTACK1 |
              bits_COND_CAN_RANGE_ATTACK2 |
              bits_COND_HEAR_SOUND,

          bits_SOUND_WORLD | // sound flags
              bits_SOUND_DANGER |
              bits_SOUND_PLAYER,

          "Grunt Sweep" },
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack1A[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_CROUCH },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
};

Schedule_t slGruntRangeAttack1A[] =
    {
        { tlGruntRangeAttack1A,
          ARRAYSIZE( tlGruntRangeAttack1A ),
          bits_COND_NEW_ENEMY |
              bits_COND_ENEMY_DEAD |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_ENEMY_OCCLUDED |
              bits_COND_HEAR_SOUND |
              bits_COND_GRUNT_NOFIRE |
              bits_COND_NO_AMMO_LOADED,

          bits_SOUND_DANGER,
          "Range Attack1A" },
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack1B[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_IDLE_ANGRY },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_GRUNT_CHECK_FIRE, (float)0 },
        { TASK_RANGE_ATTACK1, (float)0 },
};

Schedule_t slGruntRangeAttack1B[] =
    {
        { tlGruntRangeAttack1B,
          ARRAYSIZE( tlGruntRangeAttack1B ),
          bits_COND_NEW_ENEMY |
              bits_COND_ENEMY_DEAD |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_ENEMY_OCCLUDED |
              bits_COND_NO_AMMO_LOADED |
              bits_COND_GRUNT_NOFIRE |
              bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER,
          "Range Attack1B" },
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack2[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_GRUNT_FACE_TOSS_DIR, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_RANGE_ATTACK2 },
        { TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY }, // don't run immediately after throwing grenade.
};

Schedule_t slGruntRangeAttack2[] =
    {
        { tlGruntRangeAttack2,
          ARRAYSIZE( tlGruntRangeAttack2 ),
          0,
          0,
          "RangeAttack2" },
};

//=========================================================
// repel
//=========================================================
Task_t tlGruntRepel[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FACE_IDEAL, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_GLIDE },
};

Schedule_t slGruntRepel[] =
    {
        { tlGruntRepel,
          ARRAYSIZE( tlGruntRepel ),
          bits_COND_SEE_ENEMY |
              bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER |
              bits_SOUND_COMBAT |
              bits_SOUND_PLAYER,
          "Repel" },
};

//=========================================================
// repel
//=========================================================
Task_t tlGruntRepelAttack[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_FACE_ENEMY, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_FLY },
};

Schedule_t slGruntRepelAttack[] =
    {
        { tlGruntRepelAttack,
          ARRAYSIZE( tlGruntRepelAttack ),
          bits_COND_ENEMY_OCCLUDED,
          0,
          "Repel Attack" },
};

//=========================================================
// repel land
//=========================================================
Task_t tlGruntRepelLand[] =
    {
        { TASK_STOP_MOVING, (float)0 },
        { TASK_PLAY_SEQUENCE, (float)ACT_LAND },
        { TASK_GET_PATH_TO_LASTPOSITION, (float)0 },
        { TASK_RUN_PATH, (float)0 },
        { TASK_WAIT_FOR_MOVEMENT, (float)0 },
        { TASK_CLEAR_LASTPOSITION, (float)0 },
};

Schedule_t slGruntRepelLand[] =
    {
        { tlGruntRepelLand,
          ARRAYSIZE( tlGruntRepelLand ),
          bits_COND_SEE_ENEMY |
              bits_COND_NEW_ENEMY |
              bits_COND_LIGHT_DAMAGE |
              bits_COND_HEAVY_DAMAGE |
              bits_COND_HEAR_SOUND,

          bits_SOUND_DANGER |
              bits_SOUND_COMBAT |
              bits_SOUND_PLAYER,
          "Repel Land" },
};

DEFINE_CUSTOM_SCHEDULES( CHGrunt ){
    slGruntFail,
    slGruntCombatFail,
    slGruntVictoryDance,
    slGruntEstablishLineOfFire,
    slGruntFoundEnemy,
    slGruntCombatFace,
    slGruntSignalSuppress,
    slGruntSuppress,
    slGruntWaitInCover,
    slGruntTakeCover,
    slGruntGrenadeCover,
    slGruntTossGrenadeCover,
    slGruntTakeCoverFromBestSound,
    slGruntHideReload,
    slGruntSweep,
    slGruntRangeAttack1A,
    slGruntRangeAttack1B,
    slGruntRangeAttack2,
    slGruntRepel,
    slGruntRepelAttack,
    slGruntRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHGrunt, CSquadMonster );

//=========================================================
// SetActivity
//=========================================================
void CHGrunt ::SetActivity( Activity NewActivity )
{
	int iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel  = GET_MODEL_PTR( ENT( pev ) );

	switch ( NewActivity )
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if ( FBitSet( pev->weapons, HGRUNT_9MMAR ) )
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_mp5" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_mp5" );
			}
		}
		else
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_shotgun" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_shotgun" );
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown
		// grenade or fired grenade, we must determine which and pick proper sequence
		if ( pev->weapons & HGRUNT_HANDGRENADE )
		{
			// get toss anim
			iSequence = LookupSequence( "throwgrenade" );
		}
		else
		{
			// get launch anim
			iSequence = LookupSequence( "launchgrenade" );
		}
		break;
	case ACT_RUN:
		if ( pev->health <= HGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity( ACT_RUN_HURT );
		}
		else
		{
			iSequence = LookupActivity( NewActivity );
		}
		break;
	case ACT_WALK:
		if ( pev->health <= HGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity( ACT_WALK_HURT );
		}
		else
		{
			iSequence = LookupActivity( NewActivity );
		}
		break;
	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity( NewActivity );
		break;
	default:
		iSequence = LookupActivity( NewActivity );
		break;
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT( at_console, "%s has no sequence for act:%d\n", STRING( pev->classname ), NewActivity );
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}
}

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t *CHGrunt ::GetSchedule( void )
{

	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling.
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if ( pev->flags & FL_ONGROUND )
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType( SCHED_GRUNT_REPEL_LAND );
		}
		else
		{
			// repel down a rope,
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType( SCHED_GRUNT_REPEL_ATTACK );
			else
				return GetScheduleOfType( SCHED_GRUNT_REPEL );
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound )
		{
			if ( pSound->m_iType & bits_SOUND_DANGER )
			{
				// dangerous sound nearby!

				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause
				// this may only affect a single individual in a squad.

				if ( FOkToSpeak() )
				{
					SENTENCEG_PlayRndSz( ENT( pev ), "HG_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
			/*
			if (!HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & (bits_SOUND_PLAYER | bits_SOUND_COMBAT) ))
			{
			    MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if ( HasConditions( bits_COND_ENEMY_DEAD ) )
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster ::GetSchedule();
		}

		// new enemy
		if ( HasConditions( bits_COND_NEW_ENEMY ) )
		{
			if ( InSquad() )
			{
				MySquadLeader()->m_fEnemyEluded = FALSE;

				if ( !IsLeader() )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
					//!!!KELLY - the leader of a squad of grunts has just seen the player or a
					// monster and has made it the squad's enemy. You
					// can check pev->flags for FL_CLIENT to determine whether this is the player
					// or a monster. He's going to immediately start
					// firing, though. If you'd like, we can make an alternate "first sight"
					// schedule where the leader plays a handsign anim
					// that gives us enough time to hear a short sentence or spoken command
					// before he starts pluggin away.
					if ( FOkToSpeak() ) // && RANDOM_LONG(0,1))
					{
						if ( ( m_hEnemy != NULL ) && m_hEnemy->IsPlayer() )
							// player
							SENTENCEG_PlayRndSz( ENT( pev ), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
						else if ( ( m_hEnemy != NULL ) &&
						          ( m_hEnemy->Classify() != CLASS_PLAYER_ALLY ) &&
						          ( m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE ) &&
						          ( m_hEnemy->Classify() != CLASS_MACHINE ) )
							// monster
							SENTENCEG_PlayRndSz( ENT( pev ), "HG_MONST", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );

						JustSpoke();
					}

					if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
					{
						return GetScheduleOfType( SCHED_GRUNT_SUPPRESS );
					}
					else
					{
						return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
					}
				}
			}
		}
		// no ammo
		else if ( HasConditions( bits_COND_NO_AMMO_LOADED ) )
		{
			//!!!KELLY - this individual just realized he's out of bullet ammo.
			// He's going to try to find cover to run to and reload, but rarely, if
			// none is available, he'll drop and reload in the open here.
			return GetScheduleOfType( SCHED_GRUNT_COVER_AND_RELOAD );
		}

		// damaged just a little
		else if ( HasConditions( bits_COND_LIGHT_DAMAGE ) )
		{
			// if hurt:
			// 90% chance of taking cover
			// 10% chance of flinch.
			int iPercent = RANDOM_LONG( 0, 99 );

			if ( iPercent <= 90 && m_hEnemy != NULL )
			{
				// only try to take cover if we actually have an enemy!

				//!!!KELLY - this grunt was hit and is going to run to cover.
				if ( FOkToSpeak() ) // && RANDOM_LONG(0,1))
				{
					// SENTENCEG_PlayRndSz( ENT(pev), "HG_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = HGRUNT_SENT_COVER;
					// JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
			}
			else
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
		}
		// can kick
		else if ( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
		{
			return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
		}
		// can grenade launch

		else if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER ) && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
		{
			// shoot a grenade if you can
			return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
		}
		// can shoot
		else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
		{
			if ( InSquad() )
			{
				// if the enemy has eluded the squad and a squad member has just located the enemy
				// and the enemy does not see the squad member, issue a call to the squad to waste a
				// little time and give the player a chance to turn.
				if ( MySquadLeader()->m_fEnemyEluded && !HasConditions( bits_COND_ENEMY_FACING_ME ) )
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;
					return GetScheduleOfType( SCHED_GRUNT_FOUND_ENEMY );
				}
			}

			if ( OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
			{
				// try to take an available ENGAGE slot
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}
			else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
			{
				// throw a grenade if can and no engage slots are available
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
			else
			{
				// hide!
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
			}
		}
		// can't see enemy
		else if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
			{
				//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
				if ( FOkToSpeak() )
				{
					SENTENCEG_PlayRndSz( ENT( pev ), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
			else if ( OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
			{
				//!!!KELLY - grunt cannot see the enemy and has just decided to
				// charge the enemy's position.
				if ( FOkToSpeak() ) // && RANDOM_LONG(0,1))
				{
					// SENTENCEG_PlayRndSz( ENT(pev), "HG_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = HGRUNT_SENT_CHARGE;
					// JustSpoke();
				}

				return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
			}
			else
			{
				//!!!KELLY - grunt is going to stay put for a couple seconds to see if
				// the enemy wanders back out into the open, or approaches the
				// grunt's covered position. Good place for a taunt, I guess?
				if ( FOkToSpeak() && RANDOM_LONG( 0, 1 ) )
				{
					SENTENCEG_PlayRndSz( ENT( pev ), "HG_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_STANDOFF );
			}
		}

		if ( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
		{
			return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
		}
	}
	}

	// no special cases here, call the base class
	return CSquadMonster ::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t *CHGrunt ::GetScheduleOfType( int Type )
{
	switch ( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		if ( InSquad() )
		{
			if ( g_iSkillLevel == SKILL_HARD && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
			{
				if ( FOkToSpeak() )
				{
					SENTENCEG_PlayRndSz( ENT( pev ), "HG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch );
					JustSpoke();
				}
				return slGruntTossGrenadeCover;
			}
			else
			{
				return &slGruntTakeCover[0];
			}
		}
		else
		{
			if ( RANDOM_LONG( 0, 1 ) )
			{
				return &slGruntTakeCover[0];
			}
			else
			{
				return &slGruntGrenadeCover[0];
			}
		}
	}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
	{
		return &slGruntTakeCoverFromBestSound[0];
	}
	case SCHED_GRUNT_TAKECOVER_FAILED:
	{
		if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
		{
			return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
		}

		return GetScheduleOfType( SCHED_FAIL );
	}
	break;
	case SCHED_GRUNT_ELOF_FAIL:
	{
		// human grunt is unable to move to a position that allows him to attack the enemy.
		return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
	}
	break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
	{
		return &slGruntEstablishLineOfFire[0];
	}
	break;
	case SCHED_RANGE_ATTACK1:
	{
		// randomly stand or crouch
		if ( RANDOM_LONG( 0, 9 ) == 0 )
			m_fStanding = RANDOM_LONG( 0, 1 );

		if ( m_fStanding )
			return &slGruntRangeAttack1B[0];
		else
			return &slGruntRangeAttack1A[0];
	}
	case SCHED_RANGE_ATTACK2:
	{
		return &slGruntRangeAttack2[0];
	}
	case SCHED_COMBAT_FACE:
	{
		return &slGruntCombatFace[0];
	}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
	{
		return &slGruntWaitInCover[0];
	}
	case SCHED_GRUNT_SWEEP:
	{
		return &slGruntSweep[0];
	}
	case SCHED_GRUNT_COVER_AND_RELOAD:
	{
		return &slGruntHideReload[0];
	}
	case SCHED_GRUNT_FOUND_ENEMY:
	{
		return &slGruntFoundEnemy[0];
	}
	case SCHED_VICTORY_DANCE:
	{
		if ( InSquad() )
		{
			if ( !IsLeader() )
			{
				return &slGruntFail[0];
			}
		}

		return &slGruntVictoryDance[0];
	}
	case SCHED_GRUNT_SUPPRESS:
	{
		if ( m_hEnemy->IsPlayer() && m_fFirstEncounter )
		{
			m_fFirstEncounter = FALSE; // after first encounter, leader won't issue handsigns anymore when he has a new enemy
			return &slGruntSignalSuppress[0];
		}
		else
		{
			return &slGruntSuppress[0];
		}
	}
	case SCHED_FAIL:
	{
		if ( m_hEnemy != NULL )
		{
			// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
			return &slGruntCombatFail[0];
		}

		return &slGruntFail[0];
	}
	case SCHED_GRUNT_REPEL:
	{
		if ( pev->velocity.z > -128 )
			pev->velocity.z -= 32;
		return &slGruntRepel[0];
	}
	case SCHED_GRUNT_REPEL_ATTACK:
	{
		if ( pev->velocity.z > -128 )
			pev->velocity.z -= 32;
		return &slGruntRepelAttack[0];
	}
	case SCHED_GRUNT_REPEL_LAND:
	{
		return &slGruntRepelLand[0];
	}
	default:
	{
		return CSquadMonster ::GetScheduleOfType( Type );
	}
	}
}

