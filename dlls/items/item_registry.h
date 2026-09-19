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

struct ItemDescriptor
{
	const char *pszClassname;
	int iPriorityOrder;
	void ( *pfnPrecache )( void );
};

class CBasePlayer;

class ItemRegistry
{
  public:
	static void Register( const ItemDescriptor &desc );
	static void PrecacheAll( void );
	static const std::vector<ItemDescriptor> &GetDescriptors( void );
	static const ItemDescriptor *FindByClassname( const char *szClassname );
	static void Clear( void );

	// Player auxiliary custom inventory interface (#89)
	static int GetPlayerCustomItemCount( const CBasePlayer *pPlayer, const char *pszItemName );
	static void AddPlayerCustomItem( const CBasePlayer *pPlayer, const char *pszItemName, int count = 1 );
	static void SetPlayerCustomItemCount( const CBasePlayer *pPlayer, const char *pszItemName, int count );
	static bool HasPlayerCustomItem( const CBasePlayer *pPlayer, const char *pszItemName );
	static void ClearPlayerCustomItems( const CBasePlayer *pPlayer );
	static void ClearAllPlayerCustomItems( void );
};

#define REGISTER_ITEM_EX( entityName, priority, precacheCb )                  \
	static struct ItemAutoReg_##entityName                                    \
	{                                                                         \
		ItemAutoReg_##entityName()                                            \
		{                                                                     \
			ItemDescriptor desc;                                              \
			desc.pszClassname   = #entityName;                                \
			desc.iPriorityOrder = ( priority );                               \
			desc.pfnPrecache    = ( precacheCb );                             \
			ItemRegistry::Register( desc );                                   \
		}                                                                     \
	} s_autoRegItem_##entityName;

#define REGISTER_ITEM( entityName, priority )                                 \
	REGISTER_ITEM_EX( entityName, priority, nullptr )

#define REGISTER_ITEM_DEFAULT( entityName )                                   \
	REGISTER_ITEM( entityName, 0 )
