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

#pragma once

#include <vector>

struct WeaponDescriptor
{
	const char *pszClassname;
	int iPriorityOrder;
	void ( *pfnPrecache )( void );
};

class WeaponRegistry
{
  public:
	static void Register( const WeaponDescriptor &desc );
	static void PrecacheAll( void );
	static const std::vector<WeaponDescriptor> &GetDescriptors( void );
	static const WeaponDescriptor *FindByClassname( const char *szClassname );
	static void Clear( void );
};

#define REGISTER_WEAPON_EX( entityName, priority, precacheCb )                \
	static struct WeaponAutoReg_##entityName                                  \
	{                                                                         \
		WeaponAutoReg_##entityName()                                          \
		{                                                                     \
			WeaponDescriptor desc;                                            \
			desc.pszClassname   = #entityName;                                \
			desc.iPriorityOrder = ( priority );                               \
			desc.pfnPrecache    = ( precacheCb );                             \
			WeaponRegistry::Register( desc );                                 \
		}                                                                     \
	} s_autoReg_##entityName;

#define REGISTER_WEAPON( entityName, priority )                               \
	REGISTER_WEAPON_EX( entityName, priority, NULL )
