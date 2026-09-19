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

#include <string>
#include <vector>

class TalkCompanionRegistry
{
  public:
	// Register a new companion / friendly NPC classname
	static void Register( const char *pszClassname );

	// Unregister a companion classname
	static bool Unregister( const char *pszClassname );

	// Check if a classname is registered as a companion
	static bool IsRegistered( const char *pszClassname );

	// Get total count of registered companions
	static int GetCompanionCount();

	// Get companion classname by index (returns nullptr if out of bounds)
	static const char *GetCompanion( int index );

	// Direct access to registered companion list
	static const std::vector<std::string> &GetCompanions();

	// Clear all registered companions
	static void Clear();

	// Reset to canonical Half-Life defaults (barney, scientist, sitting_scientist)
	static void ResetToDefaults();
};

#define REGISTER_TALK_COMPANION( entityName )                                 \
	static struct TalkCompanionAutoReg_##entityName                           \
	{                                                                         \
		TalkCompanionAutoReg_##entityName()                                   \
		{                                                                     \
			TalkCompanionRegistry::Register( #entityName );                   \
		}                                                                     \
	} s_autoRegTalkCompanion_##entityName;
