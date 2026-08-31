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
#include "core/animation.h"
#include "world/xen.h"

//
// CActAnimating
//
TYPEDESCRIPTION CActAnimating::m_SaveData[] =
    {
        DEFINE_FIELD( CActAnimating, m_Activity, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CActAnimating, CBaseAnimating );

void CActAnimating::SetActivity( Activity act )
{
	int sequence = LookupActivity( act );
	if ( sequence != ACTIVITY_NOT_AVAILABLE )
	{
		pev->sequence = sequence;
		m_Activity    = act;
		pev->frame    = 0;
		ResetSequenceInfo();
	}
}

//
// CXenPLight
//
LINK_ENTITY_TO_CLASS( xen_plantlight, CXenPLight );

TYPEDESCRIPTION CXenPLight::m_SaveData[] =
    {
        DEFINE_FIELD( CXenPLight, m_pGlow, FIELD_CLASSPTR ),
};

IMPLEMENT_SAVERESTORE( CXenPLight, CActAnimating );

void CXenPLight::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), "models/light.mdl" );
	pev->movetype = MOVETYPE_NONE;
	pev->solid    = SOLID_TRIGGER;

	UTIL_SetSize( pev, Vector( -80, -80, 0 ), Vector( 80, 80, 32 ) );
	SetActivity( ACT_IDLE );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame     = RANDOM_FLOAT( 0, 255 );

	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector( 0, 0, ( pev->mins.z + pev->maxs.z ) * 0.5 ), FALSE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );
}

void CXenPLight::Precache( void )
{
	PRECACHE_MODEL( "models/light.mdl" );
	PRECACHE_MODEL( XEN_PLANT_GLOW_SPRITE );
}

void CXenPLight::Think( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	switch ( GetActivity() )
	{
	case ACT_CROUCH:
		if ( m_fSequenceFinished )
		{
			SetActivity( ACT_CROUCHIDLE );
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if ( gpGlobals->time > pev->dmgtime )
		{
			SetActivity( ACT_STAND );
			LightOn();
		}
		break;

	case ACT_STAND:
		if ( m_fSequenceFinished )
			SetActivity( ACT_IDLE );
		break;

	case ACT_IDLE:
	default:
		break;
	}
}

void CXenPLight::Touch( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() )
	{
		pev->dmgtime = gpGlobals->time + XEN_PLANT_HIDE_TIME;
		if ( GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND )
		{
			SetActivity( ACT_CROUCH );
		}
	}
}

void CXenPLight::LightOn( void )
{
	SUB_UseTargets( this, USE_ON, 0 );
	if ( m_pGlow )
		m_pGlow->pev->effects &= ~EF_NODRAW;
}

void CXenPLight::LightOff( void )
{
	SUB_UseTargets( this, USE_OFF, 0 );
	if ( m_pGlow )
		m_pGlow->pev->effects |= EF_NODRAW;
}

//
// CXenHair
//
LINK_ENTITY_TO_CLASS( xen_hair, CXenHair );

void CXenHair::Spawn( void )
{
	Precache();
	SET_MODEL( edict(), "models/hair.mdl" );
	UTIL_SetSize( pev, Vector( -4, -4, 0 ), Vector( 4, 4, 32 ) );
	pev->sequence = 0;

	if ( !( pev->spawnflags & SF_HAIR_SYNC ) )
	{
		pev->frame     = RANDOM_FLOAT( 0, 255 );
		pev->framerate = RANDOM_FLOAT( 0.7, 1.4 );
	}
	ResetSequenceInfo();

	pev->solid     = SOLID_NOT;
	pev->movetype  = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.4 ); // Load balance these a bit
}

void CXenHair::Think( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}

void CXenHair::Precache( void )
{
	PRECACHE_MODEL( "models/hair.mdl" );
}

//
// CXenHull
//
LINK_ENTITY_TO_CLASS( xen_hull, CXenHull );

CXenHull *CXenHull::CreateHull( CBaseEntity *source, const Vector &mins, const Vector &maxs, const Vector &offset )
{
	CXenHull *pHull = GetClassPtr( (CXenHull *)NULL );

	UTIL_SetOrigin( pHull->pev, source->pev->origin + offset );
	SET_MODEL( pHull->edict(), STRING( source->pev->model ) );
	pHull->pev->solid     = SOLID_BBOX;
	pHull->pev->classname = MAKE_STRING( "xen_hull" );
	pHull->pev->movetype  = MOVETYPE_NONE;
	pHull->pev->owner     = source->edict();
	UTIL_SetSize( pHull->pev, mins, maxs );
	pHull->pev->renderamt  = 0;
	pHull->pev->rendermode = kRenderTransTexture;

	return pHull;
}
