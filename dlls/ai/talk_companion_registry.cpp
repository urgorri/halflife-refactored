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

#include <algorithm>
#include <cstring>
#include "ai/talk_companion_registry.h"

static std::vector<std::string> s_companions = {
	"monster_barney",
	"monster_scientist",
	"monster_sitting_scientist"
};

static bool s_initialized = true;

static void EnsureInitialized()
{
	if ( !s_initialized )
	{
		s_companions = {
			"monster_barney",
			"monster_scientist",
			"monster_sitting_scientist"
		};
		s_initialized = true;
	}
}

void TalkCompanionRegistry::Register( const char *pszClassname )
{
	if ( !pszClassname || !pszClassname[0] )
		return;

	EnsureInitialized();

	// Avoid duplicates
	for ( const auto &companion : s_companions )
	{
		if ( companion == pszClassname )
			return;
	}

	s_companions.push_back( pszClassname );
}

bool TalkCompanionRegistry::Unregister( const char *pszClassname )
{
	if ( !pszClassname || !pszClassname[0] )
		return false;

	EnsureInitialized();

	auto it = std::find( s_companions.begin(), s_companions.end(), pszClassname );
	if ( it != s_companions.end() )
	{
		s_companions.erase( it );
		return true;
	}

	return false;
}

bool TalkCompanionRegistry::IsRegistered( const char *pszClassname )
{
	if ( !pszClassname || !pszClassname[0] )
		return false;

	EnsureInitialized();

	for ( const auto &companion : s_companions )
	{
		if ( companion == pszClassname )
			return true;
	}

	return false;
}

int TalkCompanionRegistry::GetCompanionCount()
{
	EnsureInitialized();
	return static_cast<int>( s_companions.size() );
}

const char *TalkCompanionRegistry::GetCompanion( int index )
{
	EnsureInitialized();

	if ( index >= 0 && index < static_cast<int>( s_companions.size() ) )
	{
		return s_companions[index].c_str();
	}

	return nullptr;
}

const std::vector<std::string> &TalkCompanionRegistry::GetCompanions()
{
	EnsureInitialized();
	return s_companions;
}

void TalkCompanionRegistry::Clear()
{
	s_companions.clear();
	s_initialized = true; // Mark as explicitly cleared so EnsureInitialized doesn't restore defaults
}

void TalkCompanionRegistry::ResetToDefaults()
{
	s_companions = {
		"monster_barney",
		"monster_scientist",
		"monster_sitting_scientist"
	};
	s_initialized = true;
}
