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

struct event_args_s;
typedef void ( *pfnEventFunc )( struct event_args_s *args );

struct EventRegistration
{
	const char *pszScriptPath;
	pfnEventFunc pfnCallback;
};

class EventRegistry
{
  public:
	static void Register( const char *pszScriptPath, pfnEventFunc pfnCallback );
	static void HookAllEvents( void );
	static const std::vector<EventRegistration> &GetRegistrations( void );
	static const EventRegistration *FindByScript( const char *pszScriptPath );
	static void Clear( void );
};

#define REGISTER_CLIENT_EVENT( scriptPath, callbackFunc )                     \
	static struct EventAutoReg_##callbackFunc                                 \
	{                                                                         \
		EventAutoReg_##callbackFunc()                                         \
		{                                                                     \
			EventRegistry::Register( scriptPath, callbackFunc );             \
		}                                                                     \
	} s_autoReg_##callbackFunc;
