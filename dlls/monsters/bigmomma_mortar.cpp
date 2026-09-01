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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/decals.h"
#include "core/skill.h"
#include "weapons/weapon_base.h"
#include "weapons/weapon_damage.h"
#include "monsters/bigmomma_mortar.h"

LINK_ENTITY_TO_CLASS( bmortar, CBMortar );

TYPEDESCRIPTION CBMortar::m_SaveData[] =
    {
        DEFINE_FIELD( CBMortar, m_maxFrame, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CBMortar, CBaseEntity );

void CBMortar::Spawn( void )
{
	pev->movetype  = MOVETYPE_TOSS;
	pev->classname = MAKE_STRING( "bmortar" );

	pev->solid      = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt  = 255;

	SET_MODEL( ENT( pev ), "sprites/mommaspit.spr" );
	pev->frame = 0;
	pev->scale = 0.5;

	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	m_maxFrame   = (float)MODEL_FRAMES( pev->modelindex ) - 1;
	pev->dmgtime = gpGlobals->time + 0.4;
}

void CBMortar::Animate( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if ( gpGlobals->time > pev->dmgtime )
	{
		pev->dmgtime = gpGlobals->time + 0.2;
		MortarSpray( pev->origin, -pev->velocity.Normalize(), gSpitSprite, 3 );
	}
	if ( pev->frame++ )
	{
		if ( pev->frame > m_maxFrame )
		{
			pev->frame = 0;
		}
	}
}

CBMortar *CBMortar::Shoot( edict_t *pOwner, Vector vecStart, Vector vecVelocity )
{
	CBMortar *pSpit = GetClassPtr( (CBMortar *)NULL );
	pSpit->Spawn();

	UTIL_SetOrigin( pSpit->pev, vecStart );
	pSpit->pev->velocity = vecVelocity;
	pSpit->pev->owner    = pOwner;
	pSpit->pev->scale    = 2.5;
	pSpit->SetThink( &CBMortar::Animate );
	pSpit->pev->nextthink = gpGlobals->time + 0.1;

	return pSpit;
}

void CBMortar::Touch( CBaseEntity *pOther )
{
	TraceResult tr;
	int iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT( 90, 110 );

	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch );

	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 1:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	}

	if ( pOther->IsBSPModel() )
	{
		// make a splat on the wall
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
		UTIL_DecalTrace( &tr, DECAL_MOMMASPLAT );
	}
	else
	{
		tr.vecEndPos      = pev->origin;
		tr.vecPlaneNormal = -1 * pev->velocity.Normalize();
	}
	// make some flecks
	MortarSpray( tr.vecEndPos, tr.vecPlaneNormal, gSpitSprite, 24 );

	entvars_t *pevOwner = NULL;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );

	RadiusDamage( pev->origin, pev, pevOwner, gSkillData.bigmommaDmgBlast, gSkillData.bigmommaRadiusBlast, CLASS_NONE, DMG_ACID );
	UTIL_Remove( this );
}

#endif
