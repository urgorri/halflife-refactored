/*** 
 * monster_sensors.cpp - Monster Sensing, Vision & Hearing routines
 ***/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "ai/nodes.h"
#include "ai/monsters.h"
#include "ai/monster_sensors.h"
#include "core/animation.h"
#include "core/saverestore.h"
#include "weapons/weapons.h"
#include "gameplay/scripted.h"
#include "ai/squadmonster.h"
#include "core/decals.h"
#include "ai/soundent.h"
#include "gameplay/gamerules.h"
#include "ai/defaultai.h"
#include "ai/schedule.h"

void CBaseMonster ::Listen( void )
{
	int iSound;
	int iMySounds;
	float hearingSensitivity;
	CSound *pCurrentSound;

	m_iAudibleList = SOUNDLIST_EMPTY;
	ClearConditions( bits_COND_HEAR_SOUND | bits_COND_SMELL | bits_COND_SMELL_FOOD );
	m_afSoundTypes = 0;

	iMySounds = ISoundMask();

	if ( m_pSchedule )
	{
		//!!!WATCH THIS SPOT IF YOU ARE HAVING SOUND RELATED BUGS!
		// Make sure your schedule AND personal sound masks agree!
		iMySounds &= m_pSchedule->iSoundMask;
	}

float CBaseMonster ::FLSoundVolume( CSound *pSound )
{
	return ( pSound->m_iVolume - ( ( pSound->m_vecOrigin - pev->origin ).Length() ) );
}

//=========================================================
// FValidateHintType - tells use whether or not the monster cares
// about the type of Hint Node given
//=========================================================
void CBaseMonster ::Look( int iDistance )
{
	int iSighted = 0;

	// DON'T let visibility information from last frame sit around!
	ClearConditions( bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT );

	m_pLink = NULL;

	CBaseEntity *pSightEnt = NULL; // the current visible entity that we're dealing with

	// See no evil if prisoner is set
	if ( !FBitSet( pev->spawnflags, SF_MONSTER_PRISONER ) )
	{
		CBaseEntity *pList[100];

		Vector delta = Vector( iDistance, iDistance, iDistance );

		// Find only monsters/clients in box, NOT limited to PVS
		int count = UTIL_EntitiesInBox( pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER );
		for ( int i = 0; i < count; i++ )
		{
			pSightEnt = pList[i];
			// !!!temporarily only considering other monsters and clients, don't see prisoners
			if ( pSightEnt != this &&
			     !FBitSet( pSightEnt->pev->spawnflags, SF_MONSTER_PRISONER ) &&
			     pSightEnt->pev->health > 0 )
			{
				// the looker will want to consider this entity
				// don't check anything else about an entity that can't be seen, or an entity that you don't care about.
				if ( IRelationship( pSightEnt ) != R_NO && FInViewCone( pSightEnt ) && !FBitSet( pSightEnt->pev->flags, FL_NOTARGET ) && FVisible( pSightEnt ) )
				{
					if ( pSightEnt->IsPlayer() )
					{
						if ( pev->spawnflags & SF_MONSTER_WAIT_TILL_SEEN )
						{
							CBaseMonster *pClient;

							pClient = pSightEnt->MyMonsterPointer();
							// don't link this client in the list if the monster is wait till seen and the player isn't facing the monster
							if ( pSightEnt && !pClient->FInViewCone( this ) )
							{
								// we're not in the player's view cone.
								continue;
							}
							else
							{
								// player sees us, become normal now.
								pev->spawnflags &= ~SF_MONSTER_WAIT_TILL_SEEN;
							}
						}

						// if we see a client, remember that (mostly for scripted AI)
						iSighted |= bits_COND_SEE_CLIENT;
					}

					pSightEnt->m_pLink = m_pLink;
					m_pLink            = pSightEnt;

					if ( pSightEnt == m_hEnemy )
					{
						// we know this ent is visible, so if it also happens to be our enemy, store that now.
						iSighted |= bits_COND_SEE_ENEMY;
					}

					// don't add the Enemy's relationship to the conditions. We only want to worry about conditions when
					// we see monsters other than the Enemy.
					switch ( IRelationship( pSightEnt ) )
					{
					case R_NM:
						iSighted |= bits_COND_SEE_NEMESIS;
						break;
					case R_HT:
						iSighted |= bits_COND_SEE_HATE;
						break;
					case R_DL:
						iSighted |= bits_COND_SEE_DISLIKE;
						break;
					case R_FR:
						iSighted |= bits_COND_SEE_FEAR;
						break;
					case R_AL:
						break;
					default:
						ALERT( at_aiconsole, "%s can't assess %s\n", STRING( pev->classname ), STRING( pSightEnt->pev->classname ) );
						break;
					}
				}
			}
		}
	}

	SetConditions( iSighted );
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CBaseMonster ::ISoundMask( void )
{
	return bits_SOUND_WORLD |
	       bits_SOUND_COMBAT |
	       bits_SOUND_PLAYER;
}

//=========================================================
// PBestSound - returns a pointer to the sound the monster
// should react to. Right now responds only to nearest sound.
//=========================================================
CSound *CBaseMonster ::PBestSound( void )
{
	int iThisSound;
	int iBestSound   = -1;
	float flBestDist = 8192; // so first nearby sound will become best so far.
	float flDist;
	CSound *pSound;

	iThisSound = m_iAudibleList;

	if ( iThisSound == SOUNDLIST_EMPTY )
	{
		ALERT( at_aiconsole, "ERROR! monster %s has no audible sounds!\n", STRING( pev->classname ) );
#if _DEBUG
		ALERT( at_error, "NULL Return from PBestSound\n" );
#endif
		return NULL;
	}

	while ( iThisSound != SOUNDLIST_EMPTY )
	{
CSound *CBaseMonster ::PBestScent( void )
{
	int iThisScent;
	int iBestScent   = -1;
	float flBestDist = 8192; // so first nearby smell will become best so far.
	float flDist;
	CSound *pSound;

	iThisScent = m_iAudibleList; // smells are in the sound list.

	if ( iThisScent == SOUNDLIST_EMPTY )
	{
		ALERT( at_aiconsole, "ERROR! PBestScent() has empty soundlist!\n" );
#if _DEBUG
		ALERT( at_error, "NULL Return from PBestSound\n" );
#endif
		return NULL;
	}

	while ( iThisScent != SOUNDLIST_EMPTY )
	{
int CBaseMonster ::CheckEnemy( CBaseEntity *pEnemy )
{
	float flDistToEnemy;
	int iUpdatedLKP; // set this to TRUE if you update the EnemyLKP in this function.

	iUpdatedLKP = FALSE;
	ClearConditions( bits_COND_ENEMY_FACING_ME );

	if ( !FVisible( pEnemy ) )
	{
		ASSERT( !HasConditions( bits_COND_SEE_ENEMY ) );
		SetConditions( bits_COND_ENEMY_OCCLUDED );
	}
	else
		ClearConditions( bits_COND_ENEMY_OCCLUDED );

	if ( !pEnemy->IsAlive() )
	{
		SetConditions( bits_COND_ENEMY_DEAD );
		ClearConditions( bits_COND_SEE_ENEMY | bits_COND_ENEMY_OCCLUDED );
		return FALSE;
	}

	Vector vecEnemyPos = pEnemy->pev->origin;
	// distance to enemy's origin
	flDistToEnemy = ( vecEnemyPos - pev->origin ).Length();
	vecEnemyPos.z += pEnemy->pev->size.z * 0.5;
	// distance to enemy's head
	float flDistToEnemy2 = ( vecEnemyPos - pev->origin ).Length();
	if ( flDistToEnemy2 < flDistToEnemy )
		flDistToEnemy = flDistToEnemy2;
	else
	{
		// distance to enemy's feet
		vecEnemyPos.z -= pEnemy->pev->size.z;
		float flDistToEnemy2 = ( vecEnemyPos - pev->origin ).Length();
		if ( flDistToEnemy2 < flDistToEnemy )
			flDistToEnemy = flDistToEnemy2;
	}

	if ( HasConditions( bits_COND_SEE_ENEMY ) )
	{
		CBaseMonster *pEnemyMonster;

		iUpdatedLKP   = TRUE;
		m_vecEnemyLKP = pEnemy->pev->origin;

		pEnemyMonster = pEnemy->MyMonsterPointer();

		if ( pEnemyMonster )
		{
			if ( pEnemyMonster->FInViewCone( this ) )
			{
				SetConditions( bits_COND_ENEMY_FACING_ME );
			}
			else
				ClearConditions( bits_COND_ENEMY_FACING_ME );
		}

		if ( pEnemy->pev->velocity != Vector( 0, 0, 0 ) )
		{
			// trail the enemy a bit
			m_vecEnemyLKP = m_vecEnemyLKP - pEnemy->pev->velocity * RANDOM_FLOAT( -0.05, 0 );
		}
		else
		{
			// UNDONE: use pev->oldorigin?
		}
	}
	else if ( !HasConditions( bits_COND_ENEMY_OCCLUDED | bits_COND_SEE_ENEMY ) && ( flDistToEnemy <= 256 ) )
	{
		// if the enemy is not occluded, and unseen, that means it is behind or beside the monster.
		// if the enemy is near enough the monster, we go ahead and let the monster know where the
		// enemy is.
		iUpdatedLKP   = TRUE;
		m_vecEnemyLKP = pEnemy->pev->origin;
	}

	if ( flDistToEnemy >= m_flDistTooFar )
	{
		// enemy is very far away from monster
		SetConditions( bits_COND_ENEMY_TOOFAR );
	}
	else
		ClearConditions( bits_COND_ENEMY_TOOFAR );

	if ( FCanCheckAttacks() )
	{
		CheckAttacks( m_hEnemy, flDistToEnemy );
	}

	if ( m_movementGoal == MOVEGOAL_ENEMY )
	{
		for ( int i = m_iRouteIndex; i < ROUTE_SIZE; i++ )
		{
			if ( m_Route[i].iType == ( bits_MF_IS_GOAL | bits_MF_TO_ENEMY ) )
			{
				// UNDONE: Should we allow monsters to override this distance (80?)
				if ( ( m_Route[i].vecLocation - m_vecEnemyLKP ).Length() > 80 )
				{
					// Refresh
					FRefreshRoute();
					return iUpdatedLKP;
				}
			}
		}
	}

	return iUpdatedLKP;
}

//=========================================================
// PushEnemy - remember the last few enemies, always remember the player
//=========================================================
void CBaseMonster ::PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos )
{
	int i;

	if ( pEnemy == NULL )
		return;

	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	for ( i = 0; i < MAX_OLD_ENEMIES; i++ )
	{
		if ( m_hOldEnemy[i] == pEnemy )
			return;
		if ( m_hOldEnemy[i] == NULL ) // someone died, reuse their slot
			break;
	}
	if ( i >= MAX_OLD_ENEMIES )
		return;

	m_hOldEnemy[i]   = pEnemy;
	m_vecOldEnemy[i] = vecLastKnownPos;
}

//=========================================================
// PopEnemy - try remembering the last few enemies
//=========================================================
BOOL CBaseMonster ::PopEnemy()
{
	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	for ( int i = MAX_OLD_ENEMIES - 1; i >= 0; i-- )
	{
		if ( m_hOldEnemy[i] != NULL )
		{
			if ( m_hOldEnemy[i]->IsAlive() ) // cheat and know when they die
			{
				m_hEnemy      = m_hOldEnemy[i];
				m_vecEnemyLKP = m_vecOldEnemy[i];
				// ALERT( at_console, "remembering\n");
				return TRUE;
			}
			else
			{
				m_hOldEnemy[i] = NULL;
			}
		}
	}
	return FALSE;
}

//=========================================================
// SetActivity
//=========================================================
CBaseEntity *CBaseMonster ::BestVisibleEnemy( void )
{
	CBaseEntity *pReturn;
	CBaseEntity *pNextEnt;
	int iNearest;
	int iDist;
	int iBestRelationship;

	iNearest          = 8192; // so first visible entity will become the closest.
	pNextEnt          = m_pLink;
	pReturn           = NULL;
	iBestRelationship = R_NO;

	while ( pNextEnt != NULL )
	{
		if ( pNextEnt->IsAlive() )
		{
			if ( IRelationship( pNextEnt ) > iBestRelationship )
			{
				// this entity is disliked MORE than the entity that we
				// currently think is the best visible enemy. No need to do
				// a distance check, just get mad at this one for now.
				iBestRelationship = IRelationship( pNextEnt );
				iNearest          = ( pNextEnt->pev->origin - pev->origin ).Length();
				pReturn           = pNextEnt;
			}
			else if ( IRelationship( pNextEnt ) == iBestRelationship )
			{
				// this entity is disliked just as much as the entity that
				// we currently think is the best visible enemy, so we only
				// get mad at it if it is closer.
				iDist = ( pNextEnt->pev->origin - pev->origin ).Length();

				if ( iDist <= iNearest )
				{
					iNearest          = iDist;
					iBestRelationship = IRelationship( pNextEnt );
					pReturn           = pNextEnt;
				}
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}

//=========================================================
// MakeIdealYaw - gets a yaw value for the caller that would
// face the supplied vector. Value is stuffed into the monster's
// ideal_yaw
//=========================================================
BOOL CBaseMonster ::GetEnemy( void )
{
	CBaseEntity *pNewEnemy;

	if ( HasConditions( bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_NEMESIS ) )
	{
		pNewEnemy = BestVisibleEnemy();

		if ( pNewEnemy != m_hEnemy && pNewEnemy != NULL )
		{
			// DO NOT mess with the monster's m_hEnemy pointer unless the schedule the monster is currently running will be interrupted
			// by COND_NEW_ENEMY. This will eliminate the problem of monsters getting a new enemy while they are in a schedule that doesn't care,
			// and then not realizing it by the time they get to a schedule that does. I don't feel this is a good permanent fix.

			if ( m_pSchedule )
			{
				if ( m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY )
				{
					PushEnemy( m_hEnemy, m_vecEnemyLKP );
					SetConditions( bits_COND_NEW_ENEMY );
					m_hEnemy      = pNewEnemy;
					m_vecEnemyLKP = m_hEnemy->pev->origin;
				}
				// if the new enemy has an owner, take that one as well
				if ( pNewEnemy->pev->owner != NULL )
				{
					CBaseEntity *pOwner = GetMonsterPointer( pNewEnemy->pev->owner );
					if ( pOwner && ( pOwner->pev->flags & FL_MONSTER ) && IRelationship( pOwner ) != R_NO )
						PushEnemy( pOwner, m_vecEnemyLKP );
				}
			}
		}
	}

	// remember old enemies
	if ( m_hEnemy == NULL && PopEnemy() )
	{
		if ( m_pSchedule )
		{
			if ( m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY )
			{
				SetConditions( bits_COND_NEW_ENEMY );
			}
		}
	}

	if ( m_hEnemy != NULL )
	{
		// monster has an enemy.
		return TRUE;
	}

	return FALSE; // monster has no enemy
}

//=========================================================
// DropItem - dead monster drops named item
//=========================================================
