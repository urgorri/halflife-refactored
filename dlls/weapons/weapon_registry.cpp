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

#include "weapon_registry.h"
#include <algorithm>
#include <cstring>

#ifndef CLIENT_DLL
#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "weapons/weapon_base.h"
#endif

static std::vector<WeaponDescriptor> &GetRegistryStorage()
{
	static std::vector<WeaponDescriptor> s_weapons;
	return s_weapons;
}

void WeaponRegistry::Register( const WeaponDescriptor &desc )
{
	auto &storage = GetRegistryStorage();
	for ( const auto &existing : storage )
	{
		if ( existing.pszClassname && desc.pszClassname &&
		     strcmp( existing.pszClassname, desc.pszClassname ) == 0 )
		{
			return; // Avoid duplicate registration
		}
	}
	storage.push_back( desc );
}

void WeaponRegistry::PrecacheAll( void )
{
	auto &storage = GetRegistryStorage();
	std::sort( storage.begin(), storage.end(),
	           []( const WeaponDescriptor &a, const WeaponDescriptor &b ) {
		           return a.iPriorityOrder < b.iPriorityOrder;
	           } );

	for ( const auto &desc : storage )
	{
#ifndef CLIENT_DLL
		if ( desc.pszClassname && *desc.pszClassname )
		{
			UTIL_PrecacheOtherWeapon( desc.pszClassname );
		}
#endif
		if ( desc.pfnPrecache )
		{
			desc.pfnPrecache();
		}
	}
}

const std::vector<WeaponDescriptor> &WeaponRegistry::GetDescriptors( void )
{
	return GetRegistryStorage();
}

const WeaponDescriptor *WeaponRegistry::FindByClassname( const char *szClassname )
{
	if ( !szClassname )
		return nullptr;

	const auto &storage = GetRegistryStorage();
	for ( const auto &desc : storage )
	{
		if ( desc.pszClassname && strcmp( desc.pszClassname, szClassname ) == 0 )
		{
			return &desc;
		}
	}
	return nullptr;
}

void WeaponRegistry::Clear( void )
{
	GetRegistryStorage().clear();
}
