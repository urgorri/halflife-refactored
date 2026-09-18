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
#include "weapons/weapon_base.h"
#include "ai/nodes.h"
#include "ai/soundent.h"
#include "systems/effects.h"
#include "customentity.h"
#include "monsters/osprey.h"
#include "monsters/aircraft_fx.h"

LINK_ENTITY_TO_CLASS( monster_osprey, COsprey );

TYPEDESCRIPTION COsprey::m_SaveData[] =
    {
        DEFINE_FIELD( COsprey, m_pGoalEnt, FIELD_CLASSPTR ),
        DEFINE_FIELD( COsprey, m_vel1, FIELD_VECTOR ),
        DEFINE_FIELD( COsprey, m_vel2, FIELD_VECTOR ),
        DEFINE_FIELD( COsprey, m_pos1, FIELD_POSITION_VECTOR ),
        DEFINE_FIELD( COsprey, m_pos2, FIELD_POSITION_VECTOR ),
        DEFINE_FIELD( COsprey, m_ang1, FIELD_VECTOR ),
        DEFINE_FIELD( COsprey, m_ang2, FIELD_VECTOR ),

        DEFINE_FIELD( COsprey, m_startTime, FIELD_TIME ),
        DEFINE_FIELD( COsprey, m_dTime, FIELD_FLOAT ),
        DEFINE_FIELD( COsprey, m_velocity, FIELD_VECTOR ),

        DEFINE_FIELD( COsprey, m_flIdealtilt, FIELD_FLOAT ),
        DEFINE_FIELD( COsprey, m_flRotortilt, FIELD_FLOAT ),

        DEFINE_FIELD( COsprey, m_flRightHealth, FIELD_FLOAT ),
        DEFINE_FIELD( COsprey, m_flLeftHealth, FIELD_FLOAT ),

        DEFINE_FIELD( COsprey, m_iUnits, FIELD_INTEGER ),
        DEFINE_ARRAY( COsprey, m_hGrunt, FIELD_EHANDLE, MAX_CARRY ),
        DEFINE_ARRAY( COsprey, m_vecOrigin, FIELD_POSITION_VECTOR, MAX_CARRY ),
        DEFINE_ARRAY( COsprey, m_hRepel, FIELD_EHANDLE, 4 ),

        // DEFINE_FIELD( COsprey, m_iSoundState, FIELD_INTEGER ),
        // DEFINE_FIELD( COsprey, m_iSpriteTexture, FIELD_INTEGER ),
        // DEFINE_FIELD( COsprey, m_iPitch, FIELD_INTEGER ),

        DEFINE_FIELD( COsprey, m_iDoLeftSmokePuff, FIELD_INTEGER ),
        DEFINE_FIELD( COsprey, m_iDoRightSmokePuff, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( COsprey, CBaseMonster );

void COsprey ::Spawn( void )
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid    = SOLID_BBOX;

	SET_MODEL( ENT( pev ), "models/osprey.mdl" );
	UTIL_SetSize( pev, Vector( -400, -400, -100 ), Vector( 400, 400, 32 ) );
	UTIL_SetOrigin( pev, pev->origin );

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_YES;
	m_flRightHealth = 200;
	m_flLeftHealth  = 200;
	pev->health     = 400;

	m_flFieldOfView = 0; // 180 degrees

	pev->sequence = 0;
	ResetSequenceInfo();
	pev->frame = RANDOM_LONG( 0, 0xFF );

	InitBoneControllers();

	SetThink( &COsprey::FindAllThink );
	SetUse( &COsprey::CommandUse );

	if ( !( pev->spawnflags & SF_WAITFORTRIGGER ) )
	{
		pev->nextthink = gpGlobals->time + 1.0;
	}

	m_pos2 = pev->origin;
	m_ang2 = pev->angles;
	m_vel2 = pev->velocity;
}

void COsprey::Precache( void )
{
	UTIL_PrecacheOther( "monster_human_grunt" );

	PRECACHE_MODEL( "models/osprey.mdl" );
	PRECACHE_MODEL( "models/HVR.mdl" );

	PRECACHE_SOUND( "apache/ap_rotor4.wav" );
	PRECACHE_SOUND( "weapons/mortarhit.wav" );

	m_iSpriteTexture = PRECACHE_MODEL( "sprites/rope.spr" );

	m_iExplode    = PRECACHE_MODEL( "sprites/fexplo.spr" );
	m_iTailGibs   = PRECACHE_MODEL( "models/osprey_tailgibs.mdl" );
	m_iBodyGibs   = PRECACHE_MODEL( "models/osprey_bodygibs.mdl" );
	m_iEngineGibs = PRECACHE_MODEL( "models/osprey_enginegibs.mdl" );
}

void COsprey::CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void COsprey ::FindAllThink( void )
{
	CBaseEntity *pEntity = NULL;

	m_iUnits = 0;
	while ( m_iUnits < MAX_CARRY && ( pEntity = UTIL_FindEntityByClassname( pEntity, "monster_human_grunt" ) ) != NULL )
	{
		if ( pEntity->IsAlive() )
		{
			m_hGrunt[m_iUnits]    = pEntity;
			m_vecOrigin[m_iUnits] = pEntity->pev->origin;
			m_iUnits++;
		}
	}

	if ( m_iUnits == 0 )
	{
		ALERT( at_console, "osprey error: no grunts to resupply\n" );
		UTIL_Remove( this );
		return;
	}
	SetThink( &COsprey::FlyThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_startTime    = gpGlobals->time;
}

void COsprey ::DeployThink( void )
{
	UTIL_MakeAimVectors( pev->angles );

	Vector vecForward = gpGlobals->v_forward;
	Vector vecRight   = gpGlobals->v_right;
	Vector vecUp      = gpGlobals->v_up;

	Vector vecSrc;

	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, -4096.0 ), ignore_monsters, ENT( pev ), &tr );
	CSoundEnt::InsertSound( bits_SOUND_DANGER, tr.vecEndPos, 400, 0.3 );

	vecSrc      = pev->origin + vecForward * 32 + vecRight * 100 + vecUp * -96;
	m_hRepel[0] = MakeGrunt( vecSrc );

	vecSrc      = pev->origin + vecForward * -64 + vecRight * 100 + vecUp * -96;
	m_hRepel[1] = MakeGrunt( vecSrc );

	vecSrc      = pev->origin + vecForward * 32 + vecRight * -100 + vecUp * -96;
	m_hRepel[2] = MakeGrunt( vecSrc );

	vecSrc      = pev->origin + vecForward * -64 + vecRight * -100 + vecUp * -96;
	m_hRepel[3] = MakeGrunt( vecSrc );

	SetThink( &COsprey::HoverThink );
	pev->nextthink = gpGlobals->time + 0.1;
}

BOOL COsprey ::HasDead()
{
	for ( int i = 0; i < m_iUnits; i++ )
	{
		if ( m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive() )
		{
			return TRUE;
		}
		else
		{
			m_vecOrigin[i] = m_hGrunt[i]->pev->origin; // send them to where they died
		}
	}
	return FALSE;
}

CBaseMonster *COsprey ::MakeGrunt( Vector vecSrc )
{
	CBaseEntity *pEntity;
	CBaseMonster *pGrunt;

	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecSrc + Vector( 0, 0, -4096.0 ), dont_ignore_monsters, ENT( pev ), &tr );
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP )
		return NULL;

	for ( int i = 0; i < m_iUnits; i++ )
	{
		if ( m_hGrunt[i] == NULL || !m_hGrunt[i]->IsAlive() )
		{
			if ( m_hGrunt[i] != NULL && m_hGrunt[i]->pev->rendermode == kRenderNormal )
			{
				m_hGrunt[i]->SUB_StartFadeOut();
			}
			pEntity               = Create( "monster_human_grunt", vecSrc, pev->angles );
			pGrunt                = pEntity->MyMonsterPointer();
			pGrunt->pev->movetype = MOVETYPE_FLY;
			pGrunt->pev->velocity = Vector( 0, 0, RANDOM_FLOAT( -196, -128 ) );
			pGrunt->SetActivity( ACT_GLIDE );

			CBeam *pBeam = CBeam::BeamCreate( "sprites/rope.spr", 10 );
			pBeam->PointEntInit( vecSrc + Vector( 0, 0, 112 ), pGrunt->entindex() );
			pBeam->SetFlags( BEAM_FSOLID );
			pBeam->SetColor( 255, 255, 255 );
			pBeam->SetThink( &CBeam::SUB_Remove );
			pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

			// ALERT( at_console, "%d at %.0f %.0f %.0f\n", i, m_vecOrigin[i].x, m_vecOrigin[i].y, m_vecOrigin[i].z );
			pGrunt->m_vecLastPosition = m_vecOrigin[i];
			m_hGrunt[i]               = pGrunt;
			return pGrunt;
		}
	}
	// ALERT( at_console, "none dead\n");
	return NULL;
}

void COsprey ::HoverThink( void )
{
	int i;
	for ( i = 0; i < 4; i++ )
	{
		if ( m_hRepel[i] != NULL && m_hRepel[i]->pev->health > 0 && !( m_hRepel[i]->pev->flags & FL_ONGROUND ) )
		{
			break;
		}
	}

	if ( i == 4 )
	{
		m_startTime = gpGlobals->time;
		SetThink( &COsprey::FlyThink );
	}

	pev->nextthink = gpGlobals->time + 0.1;
	UTIL_MakeAimVectors( pev->angles );
	ShowDamage();
}

void COsprey::UpdateGoal()
{
	if ( m_pGoalEnt )
	{
		m_pos1 = m_pos2;
		m_ang1 = m_ang2;
		m_vel1 = m_vel2;
		m_pos2 = m_pGoalEnt->pev->origin;
		m_ang2 = m_pGoalEnt->pev->angles;
		UTIL_MakeAimVectors( Vector( 0, m_ang2.y, 0 ) );
		m_vel2 = gpGlobals->v_forward * m_pGoalEnt->pev->speed;

		m_startTime = m_startTime + m_dTime;
		m_dTime     = 2.0 * ( m_pos1 - m_pos2 ).Length() / ( m_vel1.Length() + m_pGoalEnt->pev->speed );

		if ( m_ang1.y - m_ang2.y < -180 )
		{
			m_ang1.y += 360;
		}
		else if ( m_ang1.y - m_ang2.y > 180 )
		{
			m_ang1.y -= 360;
		}

		if ( m_pGoalEnt->pev->speed < 400 )
			m_flIdealtilt = 0;
		else
			m_flIdealtilt = -90;
	}
	else
	{
		ALERT( at_console, "osprey missing target" );
	}
}

void COsprey::FlyThink( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if ( m_pGoalEnt == NULL && !FStringNull( pev->target ) ) // this monster has a target
	{
		m_pGoalEnt = CBaseEntity::Instance( FIND_ENTITY_BY_TARGETNAME( NULL, STRING( pev->target ) ) );
		UpdateGoal();
	}

	if ( gpGlobals->time > m_startTime + m_dTime )
	{
		if ( m_pGoalEnt->pev->speed == 0 )
		{
			SetThink( &COsprey::DeployThink );
		}
		do
		{
			m_pGoalEnt = CBaseEntity::Instance( FIND_ENTITY_BY_TARGETNAME( NULL, STRING( m_pGoalEnt->pev->target ) ) );
		} while ( m_pGoalEnt->pev->speed < 400 && !HasDead() );
		UpdateGoal();
	}

	Flight();
	ShowDamage();
}

void COsprey::Flight()
{
	float t     = ( gpGlobals->time - m_startTime );
	float scale = 1.0 / m_dTime;

	float f = UTIL_SplineFraction( t * scale, 1.0 );

	Vector pos = ( m_pos1 + m_vel1 * t ) * ( 1.0 - f ) + ( m_pos2 - m_vel2 * ( m_dTime - t ) ) * f;
	Vector ang = ( m_ang1 ) * ( 1.0 - f ) + (m_ang2)*f;
	m_velocity = m_vel1 * ( 1.0 - f ) + m_vel2 * f;

	UTIL_SetOrigin( pev, pos );
	pev->angles = ang;
	UTIL_MakeAimVectors( pev->angles );
	float flSpeed = DotProduct( gpGlobals->v_forward, m_velocity );

	// float flSpeed = DotProduct( gpGlobals->v_forward, pev->velocity );

	float m_flIdealtilt = ( 160 - flSpeed ) / 10.0;

	// ALERT( at_console, "%f %f\n", flSpeed, flIdealtilt );
	if ( m_flRotortilt < m_flIdealtilt )
	{
		m_flRotortilt += 0.5;
		if ( m_flRotortilt > 0 )
			m_flRotortilt = 0;
	}
	if ( m_flRotortilt > m_flIdealtilt )
	{
		m_flRotortilt -= 0.5;
		if ( m_flRotortilt < -90 )
			m_flRotortilt = -90;
	}
	SetBoneController( 0, m_flRotortilt );

	if ( m_iSoundState == 0 )
	{
		EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC, "apache/ap_rotor4.wav", 1.0, 0.15, 0, 110 );
		// EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "apache/ap_whine1.wav", 0.5, 0.2, 0, 110 );

		m_iSoundState = SND_CHANGE_PITCH; // Preserve sound pitch state across level transition
	}
	else
	{
		CBaseEntity *pPlayer = NULL;

		pPlayer = UTIL_FindEntityByClassname( NULL, "player" );
		// Client-specific engine audio pitch modulation
		if ( pPlayer )
		{
			float pitch = DotProduct( m_velocity - pPlayer->pev->velocity, ( pPlayer->pev->origin - pev->origin ).Normalize() );

			pitch = (int)( 100 + pitch / 75.0 );

			if ( pitch > 250 )
				pitch = 250;
			if ( pitch < 50 )
				pitch = 50;

			if ( pitch == 100 )
				pitch = 101;

			if ( pitch != m_iPitch )
			{
				m_iPitch = pitch;
				EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC, "apache/ap_rotor4.wav", 1.0, 0.15, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch );
				// ALERT( at_console, "%.0f\n", pitch );
			}
		}
		// EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "apache/ap_whine1.wav", flVol, 0.2, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);
	}
}

void COsprey::HitTouch( CBaseEntity *pOther )
{
	pev->nextthink = gpGlobals->time + 2.0;
}

/*
int COsprey::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
    if (m_flRotortilt <= -90)
    {
        m_flRotortilt = 0;
    }
    else
    {
        m_flRotortilt -= 45;
    }
    SetBoneController( 0, m_flRotortilt );
    return 0;
}
*/

void COsprey ::Killed( entvars_t *pevAttacker, int iGib )
{
	pev->movetype  = MOVETYPE_TOSS;
	pev->gravity   = 0.3;
	pev->velocity  = m_velocity;
	pev->avelocity = Vector( RANDOM_FLOAT( -20, 20 ), 0, RANDOM_FLOAT( -50, 50 ) );
	STOP_SOUND( ENT( pev ), CHAN_STATIC, "apache/ap_rotor4.wav" );

	UTIL_SetSize( pev, Vector( -32, -32, -64 ), Vector( 32, 32, 0 ) );
	SetThink( &COsprey::DyingThink );
	SetTouch( &COsprey::CrashTouch );
	pev->nextthink  = gpGlobals->time + 0.1;
	pev->health     = 0;
	pev->takedamage = DAMAGE_NO;

	m_startTime = gpGlobals->time + 4.0;
}

void COsprey::CrashTouch( CBaseEntity *pOther )
{
	// only crash if we hit something solid
	if ( pOther->pev->solid == SOLID_BSP )
	{
		SetTouch( NULL );
		m_startTime    = gpGlobals->time;
		pev->nextthink = gpGlobals->time;
		m_velocity     = pev->velocity;
	}
}

void COsprey ::DyingThink( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	pev->avelocity = pev->avelocity * 1.02;

	// still falling?
	if ( m_startTime > gpGlobals->time )
	{
		UTIL_MakeAimVectors( pev->angles );
		ShowDamage();

		Vector vecSpot = pev->origin + pev->velocity * 0.2;

		Aircraft_FallingEffects( vecSpot, g_sModelIndexFireball, g_sModelIndexSmoke );

		vecSpot = pev->origin + ( pev->mins + pev->maxs ) * 0.5;
		Aircraft_BreakModel( vecSpot, Vector( 800, 800, 132 ), pev->velocity, 50, m_iTailGibs, 8, 200, BREAK_METAL );

		// don't stop it we touch a entity
		pev->flags &= ~FL_ONGROUND;
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}
	else
	{
		Vector vecSpot = pev->origin + ( pev->mins + pev->maxs ) * 0.5;

		// gibs
		Aircraft_CrashExplosionSprite( vecSpot + Vector( 0, 0, 512 ), m_iExplode, 250, 255 );

		// blast circle
		Aircraft_CrashBlastCylinder( pev->origin, m_iSpriteTexture, MSG_PAS );

		EMIT_SOUND( ENT( pev ), CHAN_STATIC, "weapons/mortarhit.wav", 1.0, 0.3 );

		RadiusDamage( pev->origin, pev, pev, 300, CLASS_NONE, DMG_BLAST );

		// gibs
		vecSpot = pev->origin + ( pev->mins + pev->maxs ) * 0.5;
		Aircraft_BreakModel( vecSpot + Vector( 0, 0, 64 ), Vector( 800, 800, 128 ), Vector( m_velocity.x, m_velocity.y, fabs( m_velocity.z ) * 0.25 ), 40, m_iBodyGibs, 128, 200, BREAK_METAL, MSG_PAS );

		UTIL_Remove( this );
	}
}

void COsprey ::ShowDamage( void )
{
	if ( m_iDoLeftSmokePuff > 0 || RANDOM_LONG( 0, 99 ) > m_flLeftHealth )
	{
		Vector vecSrc = pev->origin + gpGlobals->v_right * -340;
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE( TE_SMOKE );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
		WRITE_SHORT( g_sModelIndexSmoke );
		WRITE_BYTE( RANDOM_LONG( 0, 9 ) + 20 ); // scale * 10
		WRITE_BYTE( 12 );                       // framerate
		MESSAGE_END();
		if ( m_iDoLeftSmokePuff > 0 )
			m_iDoLeftSmokePuff--;
	}
	if ( m_iDoRightSmokePuff > 0 || RANDOM_LONG( 0, 99 ) > m_flRightHealth )
	{
		Vector vecSrc = pev->origin + gpGlobals->v_right * 340;
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE( TE_SMOKE );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
		WRITE_SHORT( g_sModelIndexSmoke );
		WRITE_BYTE( RANDOM_LONG( 0, 9 ) + 20 ); // scale * 10
		WRITE_BYTE( 12 );                       // framerate
		MESSAGE_END();
		if ( m_iDoRightSmokePuff > 0 )
			m_iDoRightSmokePuff--;
	}
}

void COsprey::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	// ALERT( at_console, "%d %.0f\n", ptr->iHitgroup, flDamage );

	// only so much per engine
	if ( ptr->iHitgroup == 3 )
	{
		if ( m_flRightHealth < 0 )
			return;
		else
			m_flRightHealth -= flDamage;
		m_iDoLeftSmokePuff = 3 + ( flDamage / 5.0 );
	}

	if ( ptr->iHitgroup == 2 )
	{
		if ( m_flLeftHealth < 0 )
			return;
		else
			m_flLeftHealth -= flDamage;
		m_iDoRightSmokePuff = 3 + ( flDamage / 5.0 );
	}

	// hit hard, hits cockpit, hits engines
	if ( flDamage > 50 || ptr->iHitgroup == 1 || ptr->iHitgroup == 2 || ptr->iHitgroup == 3 )
	{
		// ALERT( at_console, "%.0f\n", flDamage );
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
	else
	{
		UTIL_Sparks( ptr->vecEndPos );
	}
}
