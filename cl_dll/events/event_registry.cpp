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

#include "event_registry.h"
#include <cstring>

#ifdef CLIENT_DLL
#include "hud/hud.h"
#include "cl_util.h"
#endif

static std::vector<EventRegistration> &GetRegistryStorage()
{
	static std::vector<EventRegistration> s_events;
	return s_events;
}

void EventRegistry::Register( const char *pszScriptPath, pfnEventFunc pfnCallback )
{
	if ( !pszScriptPath || !pfnCallback )
		return;

	auto &storage = GetRegistryStorage();
	for ( const auto &existing : storage )
	{
		if ( existing.pszScriptPath && strcmp( existing.pszScriptPath, pszScriptPath ) == 0 )
		{
			return; // Avoid duplicate registration
		}
	}

	EventRegistration reg;
	reg.pszScriptPath = pszScriptPath;
	reg.pfnCallback   = pfnCallback;
	storage.push_back( reg );
}

void EventRegistry::HookAllEvents( void )
{
#ifdef CLIENT_DLL
	const auto &storage = GetRegistryStorage();
	for ( const auto &reg : storage )
	{
		if ( reg.pszScriptPath && reg.pfnCallback )
		{
			gEngfuncs.pfnHookEvent( const_cast<char *>( reg.pszScriptPath ), reg.pfnCallback );
		}
	}
#endif
}

const std::vector<EventRegistration> &EventRegistry::GetRegistrations( void )
{
	return GetRegistryStorage();
}

const EventRegistration *EventRegistry::FindByScript( const char *pszScriptPath )
{
	if ( !pszScriptPath )
		return nullptr;

	const auto &storage = GetRegistryStorage();
	for ( const auto &reg : storage )
	{
		if ( reg.pszScriptPath && strcmp( reg.pszScriptPath, pszScriptPath ) == 0 )
		{
			return &reg;
		}
	}
	return nullptr;
}

void EventRegistry::Clear( void )
{
	GetRegistryStorage().clear();
}
