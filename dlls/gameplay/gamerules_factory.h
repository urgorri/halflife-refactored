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

class CGameRules;

typedef CGameRules *( *pfnGameRulesCreator )( void );
typedef bool ( *pfnGameRulesCondition )( void );

struct GameRulesRegistration
{
	const char *pszName;
	pfnGameRulesCreator pfnCreator;
	pfnGameRulesCondition pfnCondition;
	int iPriority;
};

class GameRulesFactory
{
  public:
	// Register a game rules creator with predicate condition and priority
	static void Register( const GameRulesRegistration &entry );

	// Instantiates the highest priority game rules matching the current conditions
	static CGameRules *CreateGameRules( void );

	// Registers the canonical Half-Life vanilla gamemodes (Singleplayer, Teamplay, Busters, Deathmatch)
	static void RegisterVanillaRules( void );

	// Returns the current list of registrations sorted by priority (highest first)
	static const std::vector<GameRulesRegistration> &GetRegistrations( void );

	// Look up a registration by name
	static const GameRulesRegistration *FindByName( const char *pszName );

	// Clears all registrations
	static void Clear( void );

	// Clears and re-registers vanilla Half-Life game rules
	static void Reset( void );
};

#define REGISTER_GAMERULES( name, creator, condition, priority )               \
	static struct GameRulesAutoReg_##creator                                   \
	{                                                                          \
		GameRulesAutoReg_##creator()                                           \
		{                                                                      \
			GameRulesRegistration reg;                                         \
			reg.pszName      = name;                                           \
			reg.pfnCreator   = creator;                                        \
			reg.pfnCondition = condition;                                      \
			reg.iPriority    = priority;                                       \
			GameRulesFactory::Register( reg );                                 \
		}                                                                      \
	} s_autoRegGameRules_##creator;
