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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client_weapon_manager.h"
#include <cstring>

#ifdef CLIENT_DLL
#include "weapons/weapon_base.h"
#include "weapons/weapon_crossbow.h"
#include "weapons/weapon_python.h"
#include "weapons/weapon_crowbar.h"
#include "weapons/weapon_mp5.h"
#include "weapons/weapon_rpg.h"
#include "weapons/weapon_glock.h"
#include "weapons/weapon_shotgun.h"
#include "weapons/weapon_gauss.h"
#include "weapons/weapon_egon.h"
#include "weapons/weapon_satchel.h"
#include "weapons/weapon_snark.h"
#include "weapons/weapon_handgrenade.h"
#include "weapons/weapon_tripmine.h"
#include "weapons/weapon_hornetgun.h"
#include "core/player.h"
#endif

// Static storage
static entvars_t s_rgEntVars[MAX_CLIENT_ENTITIES];
static int s_numEnts = 0;
static CBasePlayerWeapon *s_rgpWeapons[MAX_WEAPONS];

#ifdef CLIENT_DLL
// Base Half-Life weapon placeholder entities
static CGlock s_Glock;
static CCrowbar s_Crowbar;
static CPython s_Python;
static CMP5 s_Mp5;
static CCrossbow s_Crossbow;
static CShotgun s_Shotgun;
static CRpg s_Rpg;
static CGauss s_Gauss;
static CEgon s_Egon;
static CHgun s_HGun;
static CHandGrenade s_HandGren;
static CSatchel s_Satchel;
static CTripmine s_Tripmine;
static CSqueak s_Snark;
#endif

static std::vector<IClientWeaponFactory *> &GetCustomFactories()
{
	static std::vector<IClientWeaponFactory *> s_factories;
	return s_factories;
}

void ClientWeaponManager::RegisterCustomFactory( IClientWeaponFactory *pFactory )
{
	if ( pFactory )
	{
		GetCustomFactories().push_back( pFactory );
	}
}

void ClientWeaponManager::PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	if ( !pEntity )
		return;

	if ( s_numEnts >= MAX_CLIENT_ENTITIES )
	{
#ifdef CLIENT_DLL
		ALERT( at_error, "ClientWeaponManager::PrepEntity: Exceeded entity limit (%d)!\n", MAX_CLIENT_ENTITIES );
#endif
		return;
	}

	memset( &s_rgEntVars[s_numEnts], 0, sizeof( entvars_t ) );
	pEntity->pev = &s_rgEntVars[s_numEnts++];

#ifdef CLIENT_DLL
	pEntity->Precache();
	pEntity->Spawn();

	if ( pWeaponOwner )
	{
		ItemInfo info;
		memset( &info, 0, sizeof( info ) );
		CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)pEntity;
		pWeapon->m_pPlayer = pWeaponOwner;
		pWeapon->GetItemInfo( &info );
		RegisterWeapon( info.iId, pWeapon );
	}
#endif
}

void ClientWeaponManager::Init( CBasePlayer *pPlayer )
{
	static bool s_initialized = false;
	if ( s_initialized )
		return;
	s_initialized = true;

	memset( s_rgpWeapons, 0, sizeof( s_rgpWeapons ) );
	s_numEnts = 0;

	// Allocate a slot for the local player
	PrepEntity( (CBaseEntity *)pPlayer, nullptr );

#ifdef CLIENT_DLL
	// Prep and register all base Half-Life predicted weapons
	PrepEntity( &s_Glock, pPlayer );
	PrepEntity( &s_Crowbar, pPlayer );
	PrepEntity( &s_Python, pPlayer );
	PrepEntity( &s_Mp5, pPlayer );
	PrepEntity( &s_Crossbow, pPlayer );
	PrepEntity( &s_Shotgun, pPlayer );
	PrepEntity( &s_Rpg, pPlayer );
	PrepEntity( &s_Gauss, pPlayer );
	PrepEntity( &s_Egon, pPlayer );
	PrepEntity( &s_HGun, pPlayer );
	PrepEntity( &s_HandGren, pPlayer );
	PrepEntity( &s_Satchel, pPlayer );
	PrepEntity( &s_Tripmine, pPlayer );
	PrepEntity( &s_Snark, pPlayer );

	// Prep any custom mod weapons registered dynamically
	for ( auto *factory : GetCustomFactories() )
	{
		if ( factory )
		{
			CBasePlayerWeapon *pCustom = factory->Create();
			if ( pCustom )
			{
				PrepEntity( pCustom, pPlayer );
			}
		}
	}
#endif
}

void ClientWeaponManager::Shutdown( void )
{
	memset( s_rgpWeapons, 0, sizeof( s_rgpWeapons ) );
	s_numEnts = 0;
}

void ClientWeaponManager::RegisterWeapon( int iWeaponId, CBasePlayerWeapon *pInstance )
{
	if ( iWeaponId > 0 && iWeaponId < MAX_WEAPONS )
	{
		s_rgpWeapons[iWeaponId] = pInstance;
	}
}

CBasePlayerWeapon *ClientWeaponManager::GetWeapon( int iWeaponId )
{
	if ( iWeaponId > 0 && iWeaponId < MAX_WEAPONS )
	{
		return s_rgpWeapons[iWeaponId];
	}
	return nullptr;
}

CBasePlayerWeapon *ClientWeaponManager::GetWeaponByIndex( int iIndex )
{
	if ( iIndex >= 0 && iIndex < MAX_WEAPONS )
	{
		return s_rgpWeapons[iIndex];
	}
	return nullptr;
}

int ClientWeaponManager::GetMaxWeapons( void )
{
	return MAX_WEAPONS;
}

int ClientWeaponManager::GetEntityCount( void )
{
	return s_numEnts;
}

void ClientWeaponManager::Clear( void )
{
	memset( s_rgpWeapons, 0, sizeof( s_rgpWeapons ) );
	s_numEnts = 0;
	GetCustomFactories().clear();
}
