/***
 *
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "weapons/weapon_base.h"
#include "weapons/ammo_base.h"
#include "core/player.h"
#include "gameplay/gamerules.h"

#ifndef CLIENT_DLL

//=========================================================
// Standard Ammo Pickup Entities
//=========================================================

// 9mm Glock clip
IMPLEMENT_SIMPLE_AMMO( CGlockAmmo, ammo_glockclip, "models/w_9mmclip.mdl", "9mm", AMMO_GLOCKCLIP_GIVE, _9MM_MAX_CARRY, "items/9mmclip1.wav" )
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );

// 9mm MP5 clip & chain box
IMPLEMENT_SIMPLE_AMMO( CMP5AmmoClip, ammo_mp5clip, "models/w_9mmARclip.mdl", "9mm", AMMO_MP5CLIP_GIVE, _9MM_MAX_CARRY, "items/9mmclip1.wav" )
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CMP5AmmoClip );

IMPLEMENT_SIMPLE_AMMO( CMP5Chainammo, ammo_9mmbox, "models/w_chainammo.mdl", "9mm", AMMO_CHAINBOX_GIVE, _9MM_MAX_CARRY, "items/9mmclip1.wav" )

// MP5 M203 Grenades
IMPLEMENT_SIMPLE_AMMO( CMP5AmmoGrenade, ammo_mp5grenades, "models/w_ARgrenade.mdl", "ARgrenades", AMMO_M203BOX_GIVE, M203_GRENADE_MAX_CARRY, "items/9mmclip1.wav" )
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CMP5AmmoGrenade );

// .357 Python box
IMPLEMENT_SIMPLE_AMMO( CPythonAmmo, ammo_357, "models/w_357ammobox.mdl", "357", AMMO_357BOX_GIVE, _357_MAX_CARRY, "items/9mmclip1.wav" )

// Shotgun 12 gauge buckshot box
IMPLEMENT_SIMPLE_AMMO( CShotgunAmmo, ammo_buckshot, "models/w_shotbox.mdl", "buckshot", AMMO_BUCKSHOTBOX_GIVE, BUCKSHOT_MAX_CARRY, "items/9mmclip1.wav" )

// Crossbow bolts clip
IMPLEMENT_SIMPLE_AMMO( CCrossbowAmmo, ammo_crossbow, "models/w_crossbow_clip.mdl", "bolts", AMMO_CROSSBOWCLIP_GIVE, BOLT_MAX_CARRY, "items/9mmclip1.wav" )

// Gauss / Tau uranium battery
IMPLEMENT_SIMPLE_AMMO( CGaussAmmo, ammo_gaussclip, "models/w_gaussammo.mdl", "uranium", AMMO_URANIUMBOX_GIVE, URANIUM_MAX_CARRY, "items/9mmclip1.wav" )

// Egon uranium box
IMPLEMENT_SIMPLE_AMMO( CEgonAmmo, ammo_egonclip, "models/w_chainammo.mdl", "uranium", AMMO_URANIUMBOX_GIVE, URANIUM_MAX_CARRY, "items/9mmclip1.wav" )

// RPG rocket clip (gives 2x ammo in multiplayer)
class CRpgAmmo : public CBasePlayerAmmo
{
  public:
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_rpgammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_rpgammo.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		int iGive = ( g_pGameRules && g_pGameRules->IsMultiplayer() ) ? ( AMMO_RPGCLIP_GIVE * 2 ) : AMMO_RPGCLIP_GIVE;

		if ( pOther->GiveAmmo( iGive, "rockets", ROCKET_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_rpgclip, CRpgAmmo );

#endif // !CLIENT_DLL
