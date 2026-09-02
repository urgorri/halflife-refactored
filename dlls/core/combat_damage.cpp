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
#include "ai/soundent.h"
#include "core/decals.h"
#include "core/animation.h"
#include "weapons/weapon_base.h"
#include "gameplay/gamerules.h"

extern entvars_t *g_pevLastInflictor;
// take health
int CBaseMonster ::TakeHealth( float flHealth, int bitsDamageType )
{
	if ( !pev->takedamage )
		return 0;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~( bitsDamageType & ~DMG_TIMEBASED );

	return CBaseEntity::TakeHealth( flHealth, bitsDamageType );
}

/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.



GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/
int CBaseMonster ::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float flTake;
	Vector vecDir;

	if ( !pev->takedamage )
		return 0;

	if ( !IsAlive() )
	{
		return DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
	}

	if ( pev->deadflag == DEAD_NO )
	{
		// no pain sound during death animation.
		PainSound(); // "Ouch!"
	}

	//!!!LATER - make armor consideration here!
	flTake = flDamage;

	// set damage type sustained
	m_bitsDamageType |= bitsDamageType;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if ( !FNullEnt( pevInflictor ) )
	{
		CBaseEntity *pInflictor = CBaseEntity ::Instance( pevInflictor );
		if ( pInflictor )
		{
			vecDir = ( pInflictor->Center() - Vector( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( IsPlayer() )
	{
		if ( pevInflictor )
			pev->dmg_inflictor = ENT( pevInflictor );

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if ( pev->flags & FL_GODMODE )
		{
			return 0;
		}
	}

	// if this is a player, move him around!
	if ( ( !FNullEnt( pevInflictor ) ) && ( pev->movetype == MOVETYPE_WALK ) && ( !pevAttacker || pevAttacker->solid != SOLID_TRIGGER ) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

	// do the damage
	pev->health -= flTake;

	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		SetConditions( bits_COND_LIGHT_DAMAGE );
		return 0;
	}

	if ( pev->health <= 0 )
	{
		g_pevLastInflictor = pevInflictor;

		if ( bitsDamageType & DMG_ALWAYSGIB )
		{
			Killed( pevAttacker, GIB_ALWAYS );
		}
		else if ( bitsDamageType & DMG_NEVERGIB )
		{
			Killed( pevAttacker, GIB_NEVER );
		}
		else
		{
			Killed( pevAttacker, GIB_NORMAL );
		}

		g_pevLastInflictor = NULL;

		return 0;
	}

	// react to the damage (get mad)
	if ( ( pev->flags & FL_MONSTER ) && !FNullEnt( pevAttacker ) )
	{
		if ( pevAttacker->flags & ( FL_MONSTER | FL_CLIENT ) )
		{ // only if the attack was a monster or client!

			// enemy's last known position is somewhere down the vector that the attack came from.
			if ( pevInflictor )
			{
				if ( m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions( bits_COND_SEE_ENEMY ) )
				{
					m_vecEnemyLKP = pevInflictor->origin;
				}
			}
			else
			{
				m_vecEnemyLKP = pev->origin + ( g_vecAttackDir * 64 );
			}

			MakeIdealYaw( m_vecEnemyLKP );

			// add pain to the conditions
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and
			// heavy damage per monster class?
			if ( flDamage > 0 )
			{
				SetConditions( bits_COND_LIGHT_DAMAGE );
			}

			if ( flDamage >= 20 )
			{
				SetConditions( bits_COND_HEAVY_DAMAGE );
			}
		}
	}

	return 1;
}

//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
int CBaseMonster ::DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Vector vecDir;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if ( !FNullEnt( pevInflictor ) )
	{
		CBaseEntity *pInflictor = CBaseEntity ::Instance( pevInflictor );
		if ( pInflictor )
		{
			vecDir = ( pInflictor->Center() - Vector( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

#if 0 // turn this back on when the bounding box issues are resolved.

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;

	// let the damage scoot the corpse around a bit.
	if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

#endif

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if ( bitsDamageType & DMG_GIB_CORPSE )
	{
		if ( pev->health <= flDamage )
		{
			pev->health = -50;
			Killed( pevAttacker, GIB_ALWAYS );
			return 0;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}

	return 1;
}

float CBaseMonster ::DamageForce( float damage )
{
	float force = damage * ( ( 32 * 32 * 72.0 ) / ( pev->size.x * pev->size.y * pev->size.z ) ) * 5;

	if ( force > 1000.0 )
	{
		force = 1000.0;
	}

	return force;
}

//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
//
// only damage ents that can clearly be seen by the explosion!

void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity *pEntity = NULL;
	TraceResult tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;

	if ( flRadius )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	int bInWater = ( UTIL_PointContents( vecSrc ) == CONTENTS_WATER );

	vecSrc.z += 1; // in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ( ( pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius ) ) != NULL )
	{
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{ // houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if ( bInWater && pEntity->pev->waterlevel == 0 )
				continue;
			if ( !bInWater && pEntity->pev->waterlevel == 3 )
				continue;

			vecSpot = pEntity->BodyTarget( vecSrc );

			UTIL_TraceLine( vecSrc, vecSpot, dont_ignore_monsters, ENT( pevInflictor ), &tr );

			if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
			{ // the explosion can 'see' this entity, so hurt them!
				if ( tr.fStartSolid )
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos  = vecSrc;
					tr.flFraction = 0.0;
				}

				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;

				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}

				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if ( tr.flFraction != 1.0 )
				{
					ClearMultiDamage();
					pEntity->TraceAttack( pevInflictor, flAdjustedDamage, ( tr.vecEndPos - vecSrc ).Normalize(), &tr, bitsDamageType );
					ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					pEntity->TakeDamage( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType );
				}
			}
		}
	}
}

void CBaseMonster ::RadiusDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}

void CBaseMonster ::RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
	::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}

//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.


void CBaseEntity ::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	if ( BloodColor() == DONT_BLEED )
		return;

	if ( flDamage == 0 )
		return;

	if ( !( bitsDamageType & ( DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR ) ) )
		return;

	// make blood decal on the wall!
	TraceResult Bloodtr;
	Vector vecTraceDir;
	float flNoise;
	int cCount;
	int i;

	/*
	    if ( !IsAlive() )
	    {
	        // dealing with a dead monster.
	        if ( pev->max_health <= 0 )
	        {
	            // no blood decal for a monster that has already decalled its limit.
	            return;
	        }
	        else
	        {
	            pev->max_health--;
	        }
	    }
	*/

	if ( flDamage < 10 )
	{
		flNoise = 0.1;
		cCount  = 1;
	}
	else if ( flDamage < 25 )
	{
		flNoise = 0.2;
		cCount  = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount  = 4;
	}

	for ( i = 0; i < cCount; i++ )
	{
		vecTraceDir = vecDir * -1; // trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT( pev ), &Bloodtr );

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

//=========================================================
//=========================================================
void CBaseMonster ::MakeDamageBloodDecal( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir )
{
	// make blood decal on the wall!
	TraceResult Bloodtr;
	Vector vecTraceDir;
	int i;

	if ( !IsAlive() )
	{
		// dealing with a dead monster.
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return;
		}
		else
		{
			pev->max_health--;
		}
	}

	for ( i = 0; i < cCount; i++ )
	{
		vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT( pev ), &Bloodtr );

		/*
		        MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		            WRITE_BYTE( TE_SHOWLINE);
		            WRITE_COORD( ptr->vecEndPos.x );
		            WRITE_COORD( ptr->vecEndPos.y );
		            WRITE_COORD( ptr->vecEndPos.z );

		            WRITE_COORD( Bloodtr.vecEndPos.x );
		            WRITE_COORD( Bloodtr.vecEndPos.y );
		            WRITE_COORD( Bloodtr.vecEndPos.z );
		        MESSAGE_END();
		*/

		if ( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}
