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
#include "core/animation.h"
#include "core/decals.h"
#include "weapons/weapon_base.h"
#include "systems/func_break.h"

extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL int g_iSkillLevel;

extern Vector VecBModelOrigin( entvars_t *pevBModel );
extern entvars_t *g_pevLastInflictor;

#define GERMAN_GIB_COUNT 4
#define HUMAN_GIB_COUNT 6
#define ALIEN_GIB_COUNT 4

// HACKHACK -- The gib velocity equations don't work
void CGib ::LimitVelocity( void )
{
	float length = pev->velocity.Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if ( length > 1500.0 )
		pev->velocity = pev->velocity.Normalize() * 1500; // This should really be sv_maxvelocity * 0.75 or something
}

void CGib ::SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs )
{
	int i;

	if ( g_Language == LANGUAGE_GERMAN )
	{
		// no sticky gibs in germany right now!
		return;
	}

	for ( i = 0; i < cGibs; i++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		pGib->Spawn( "models/stickygib.mdl" );
		pGib->pev->body = RANDOM_LONG( 0, 2 );

		if ( pevVictim )
		{
			pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT( -3, 3 );
			pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT( -3, 3 );

			/*
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) );
			*/

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT( -0.15, 0.15 );
			pGib->pev->velocity.y += RANDOM_FLOAT( -0.15, 0.15 );
			pGib->pev->velocity.z += RANDOM_FLOAT( -0.15, 0.15 );

			pGib->pev->velocity = pGib->pev->velocity * 900;

			pGib->pev->avelocity.x = RANDOM_FLOAT( 250, 400 );
			pGib->pev->avelocity.y = RANDOM_FLOAT( 250, 400 );

			// copy owner's blood color
			pGib->m_bloodColor = ( CBaseEntity::Instance( pevVictim ) )->BloodColor();

			if ( pevVictim->health > -50 )
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200 )
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid    = SOLID_BBOX;
			UTIL_SetSize( pGib->pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
			pGib->SetTouch( &CGib::StickyGibTouch );
			pGib->SetThink( NULL );
		}
		pGib->LimitVelocity();
	}
}

void CGib ::SpawnHeadGib( entvars_t *pevVictim )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	if ( g_Language == LANGUAGE_GERMAN )
	{
		pGib->Spawn( "models/germangibs.mdl" ); // throw one head
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn( "models/hgibs.mdl" ); // throw one head
		pGib->pev->body = 0;
	}

	if ( pevVictim )
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;

		edict_t *pentPlayer = FIND_CLIENT_IN_PVS( pGib->edict() );

		if ( RANDOM_LONG( 0, 100 ) <= 5 && pentPlayer )
		{
			// 5% chance head will be thrown at player's face.
			entvars_t *pevPlayer;

			pevPlayer           = VARS( pentPlayer );
			pGib->pev->velocity = ( ( pevPlayer->origin + pevPlayer->view_ofs ) - pGib->pev->origin ).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
		{
			pGib->pev->velocity = Vector( RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( 200, 300 ) );
		}

		pGib->pev->avelocity.x = RANDOM_FLOAT( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT( 100, 300 );

		// copy owner's blood color
		pGib->m_bloodColor = ( CBaseEntity::Instance( pevVictim ) )->BloodColor();

		if ( pevVictim->health > -50 )
		{
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		}
		else if ( pevVictim->health > -200 )
		{
			pGib->pev->velocity = pGib->pev->velocity * 2;
		}
		else
		{
			pGib->pev->velocity = pGib->pev->velocity * 4;
		}
	}
	pGib->LimitVelocity();
}

void CGib ::SpawnRandomGibs( entvars_t *pevVictim, int cGibs, int human )
{
	int cSplat;

	for ( cSplat = 0; cSplat < cGibs; cSplat++ )
	{
		CGib *pGib = GetClassPtr( (CGib *)NULL );

		if ( g_Language == LANGUAGE_GERMAN )
		{
			pGib->Spawn( "models/germangibs.mdl" );
			pGib->pev->body = RANDOM_LONG( 0, GERMAN_GIB_COUNT - 1 );
		}
		else
		{
			if ( human )
			{
				// human pieces
				pGib->Spawn( "models/hgibs.mdl" );
				pGib->pev->body = RANDOM_LONG( 1, HUMAN_GIB_COUNT - 1 ); // start at one to avoid throwing random amounts of skulls (0th gib)
			}
			else
			{
				// aliens
				pGib->Spawn( "models/agibs.mdl" );
				pGib->pev->body = RANDOM_LONG( 0, ALIEN_GIB_COUNT - 1 );
			}
		}

		if ( pevVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * ( RANDOM_FLOAT( 0, 1 ) );
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * ( RANDOM_FLOAT( 0, 1 ) );
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * ( RANDOM_FLOAT( 0, 1 ) ) + 1; // absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			pGib->pev->velocity = g_vecAttackDir * -1;

			// mix in some noise
			pGib->pev->velocity.x += RANDOM_FLOAT( -0.25, 0.25 );
			pGib->pev->velocity.y += RANDOM_FLOAT( -0.25, 0.25 );
			pGib->pev->velocity.z += RANDOM_FLOAT( -0.25, 0.25 );

			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT( 300, 400 );

			pGib->pev->avelocity.x = RANDOM_FLOAT( 100, 200 );
			pGib->pev->avelocity.y = RANDOM_FLOAT( 100, 300 );

			// copy owner's blood color
			pGib->m_bloodColor = ( CBaseEntity::Instance( pevVictim ) )->BloodColor();

			if ( pevVictim->health > -50 )
			{
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			}
			else if ( pevVictim->health > -200 )
			{
				pGib->pev->velocity = pGib->pev->velocity * 2;
			}
			else
			{
				pGib->pev->velocity = pGib->pev->velocity * 4;
			}

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize( pGib->pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

BOOL CBaseMonster ::HasHumanGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_HUMAN_MILITARY ||
	     myClass == CLASS_PLAYER_ALLY ||
	     myClass == CLASS_HUMAN_PASSIVE ||
	     myClass == CLASS_PLAYER )

		return TRUE;

	return FALSE;
}

BOOL CBaseMonster ::HasAlienGibs( void )
{
	int myClass = Classify();

	if ( myClass == CLASS_ALIEN_MILITARY ||
	     myClass == CLASS_ALIEN_MONSTER ||
	     myClass == CLASS_ALIEN_PASSIVE ||
	     myClass == CLASS_INSECT ||
	     myClass == CLASS_ALIEN_PREDATOR ||
	     myClass == CLASS_ALIEN_PREY )

		return TRUE;

	return FALSE;
}

void CBaseMonster::FadeMonster( void )
{
	StopAnimation();
	pev->velocity  = g_vecZero;
	pev->movetype  = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime  = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
void CBaseMonster ::GibMonster( void )
{
	TraceResult tr;
	BOOL gibbed = FALSE;

	EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM );

	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT( "violence_hgibs" ) != 0 ) // Only the player will ever get here
		{
			CGib::SpawnHeadGib( pev );
			CGib::SpawnRandomGibs( pev, 4, 1 ); // throw some human gibs.
		}
		gibbed = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT( "violence_agibs" ) != 0 ) // Should never get here, but someone might call it directly
		{
			CGib::SpawnRandomGibs( pev, 4, 0 ); // Throw alien gibs
		}
		gibbed = TRUE;
	}

	if ( !IsPlayer() )
	{
		if ( gibbed )
		{
			// don't remove players!
			SetThink( &CBaseMonster::SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			FadeMonster();
		}
	}
}

//=========================================================
// GetDeathActivity - determines the best type of death


BOOL CBaseMonster::ShouldGibMonster( int iGib )
{
	if ( ( iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS ) )
		return TRUE;

	return FALSE;
}

void CBaseMonster::CallGibMonster( void )
{
	BOOL fade = FALSE;

	if ( HasHumanGibs() )
	{
		if ( CVAR_GET_FLOAT( "violence_hgibs" ) == 0 )
			fade = TRUE;
	}
	else if ( HasAlienGibs() )
	{
		if ( CVAR_GET_FLOAT( "violence_agibs" ) == 0 )
			fade = TRUE;
	}

	pev->takedamage = DAMAGE_NO;
	pev->solid      = SOLID_NOT; // do something with the body. while monster blows up

	if ( fade )
	{
		FadeMonster();
	}
	else
	{
		pev->effects = EF_NODRAW; // make the model invisible.
		GibMonster();
	}

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	if ( pev->health < -99 )
	{
		pev->health = 0;
	}

	if ( ShouldFadeOnDeath() && !fade )
		UTIL_Remove( this );
}

// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity ::SUB_StartFadeOut( void )
{
	if ( pev->rendermode == kRenderNormal )
	{
		pev->renderamt  = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid     = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink( &CBaseEntity::SUB_FadeOut );
}

void CBaseEntity ::SUB_FadeOut( void )
{
	if ( pev->renderamt > 7 )
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink( &CBaseEntity::SUB_Remove );
	}
}

//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib ::WaitTillLand( void )
{
	if ( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	if ( pev->velocity == g_vecZero )
	{
		SetThink( &CGib::SUB_StartFadeOut );
		pev->nextthink = gpGlobals->time + m_lifeTime;

		// If you bleed, you stink!
		if ( m_bloodColor != DONT_BLEED )
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound( bits_SOUND_MEAT, pev->origin, 384, 25 );
		}
	}
	else
	{
		// wait and check again in another half second.
		pev->nextthink = gpGlobals->time + 0.5;
	}
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib ::BounceGibTouch( CBaseEntity *pOther )
{
	Vector vecSpot;
	TraceResult tr;

	// if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if ( pev->flags & FL_ONGROUND )
	{
		pev->velocity    = pev->velocity * 0.9;
		pev->angles.x    = 0;
		pev->angles.z    = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{
		if ( g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED )
		{
			vecSpot = pev->origin + Vector( 0, 0, 8 ); // move up a bit, and trace down.
			UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -24 ), ignore_monsters, ENT( pev ), &tr );

			UTIL_BloodDecalTrace( &tr, m_bloodColor );

			m_cBloodDecals--;
		}

		if ( m_material != matNone && RANDOM_LONG( 0, 2 ) == 0 )
		{
			float volume;
			float zvel = fabs( pev->velocity.z );

			volume = 0.8 * min( 1.0, ( (float)zvel ) / 450.0 );

			CBreakable::MaterialSoundRandom( edict(), (Materials)m_material, volume );
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put.
//
void CGib ::StickyGibTouch( CBaseEntity *pOther )
{
	Vector vecSpot;
	TraceResult tr;

	SetThink( &CGib::SUB_Remove );
	pev->nextthink = gpGlobals->time + 10;

	if ( !FClassnameIs( pOther->pev, "worldspawn" ) )
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT( pev ), &tr );

	UTIL_BloodDecalTrace( &tr, m_bloodColor );

	pev->velocity  = tr.vecPlaneNormal * -1;
	pev->angles    = UTIL_VecToAngles( pev->velocity );
	pev->velocity  = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->movetype  = MOVETYPE_NONE;
}

//
// Throw a chunk
//
void CGib ::Spawn( const char *szGibModel )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55; // deading the bounce a bit

	// sometimes an entity inherits the edict from a former piece of glass,
	// and will spawn using the same render FX or rendermode! bad!
	pev->renderamt  = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx   = kRenderFxNone;
	pev->solid      = SOLID_SLIDEBOX; /// hopefully this will fix the VELOCITY TOO LOW crap
	pev->classname  = MAKE_STRING( "gib" );

	SET_MODEL( ENT( pev ), szGibModel );
	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime     = 25;
	SetThink( &CGib::WaitTillLand );
	SetTouch( &CGib::BounceGibTouch );

	m_material     = matNone;
	m_cBloodDecals = 5; // how many blood decals this gib can place (1 per bounce until none remain).
}

