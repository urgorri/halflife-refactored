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

#include "weapons/weapon_defs.h"
#include <vector>

class CBaseEntity;
class CBasePlayer;
class CBasePlayerWeapon;
struct entvars_s;
typedef struct entvars_s entvars_t;

// Client prediction entity limit (safely expanded from 32 to 64 to match weapondata[64])
#define MAX_CLIENT_ENTITIES 64

class IClientWeaponFactory
{
  public:
	virtual ~IClientWeaponFactory() {}
	virtual CBasePlayerWeapon *Create( void ) = 0;
};

class ClientWeaponManager
{
  public:
	// Lifecycle
	static void Init( CBasePlayer *pPlayer );
	static void Shutdown( void );

	// Entity preparation & storage
	static void PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner );

	// Weapon registry & lookup
	static void RegisterWeapon( int iWeaponId, CBasePlayerWeapon *pInstance );
	static CBasePlayerWeapon *GetWeapon( int iWeaponId );
	static CBasePlayerWeapon *GetWeaponByIndex( int iIndex );
	static int GetMaxWeapons( void );

	// Custom weapon registration for mod extensions
	static void RegisterCustomFactory( IClientWeaponFactory *pFactory );

	// Entity vars debugging/metrics
	static int GetEntityCount( void );
	static void Clear( void );
};

#define REGISTER_CLIENT_WEAPON( WeaponClass )                                  \
	namespace                                                                  \
	{                                                                          \
		class Factory_##WeaponClass : public IClientWeaponFactory              \
		{                                                                      \
		  public:                                                              \
			virtual CBasePlayerWeapon *Create( void ) override                 \
			{                                                                  \
				static WeaponClass s_instance;                                 \
				return &s_instance;                                            \
			}                                                                  \
		};                                                                     \
		static struct AutoRegister_##WeaponClass                               \
		{                                                                      \
			AutoRegister_##WeaponClass()                                       \
			{                                                                  \
				static Factory_##WeaponClass s_factory;                        \
				ClientWeaponManager::RegisterCustomFactory( &s_factory );      \
			}                                                                  \
		} s_autoRegister_##WeaponClass;                                        \
	}
