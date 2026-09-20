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
#include "hud_base.h"

#ifdef CLIENT_DLL
class CHud;
#else
#include <cstdlib>
// Minimal definition of CHud for test harnesses without client engine dependencies
class CHud
{
  public:
	HUDLIST *m_pHudList;
	CHud() : m_pHudList( nullptr ) {}
	~CHud()
	{
		HUDLIST *pList = m_pHudList;
		while ( pList )
		{
			HUDLIST *pNext = pList->pNext;
			free( pList );
			pList = pNext;
		}
		m_pHudList = nullptr;
	}

	void AddHudElem( CHudBase *phudelem );
	void RemoveHudElem( CHudBase *phudelem );
	HUDLIST *GetHudList() const { return m_pHudList; }
};
#endif

typedef void ( *pfnHudCustomInit )( CHudBase *pElement );

struct HudElementDescriptor
{
	CHudBase *pElement;
	int iDrawOrder;
	const char *pszName;
	pfnHudCustomInit pfnInit;
};

class HudRegistry
{
  public:
	// Registers a HUD element with a given draw order and optional name/init callback
	static void RegisterElement( CHudBase *pElement, int iDrawOrder = 0, const char *pszName = nullptr, pfnHudCustomInit pfnInit = nullptr );

	// Unregisters a HUD element by pointer
	static bool UnregisterElement( CHudBase *pElement );

	// Attaches and initializes all registered elements onto the specified CHud instance
	static void AttachAll( CHud *pHud );

	// Triggers VidInit() on all elements currently attached to the HUD
	static void VidInitAll( CHud *pHud );

	// Registers the base/vanilla HUD elements for a CHud instance (called by CHud::Init)
	static void RegisterBaseElements( CHud *pHud );

	// Returns the read-only list of registered element descriptors
	static const std::vector<HudElementDescriptor> &GetElements( void );

	// Finds a registered element by name
	static CHudBase *FindElement( const char *pszName );

	// Finds a registered element by type
	template <typename T>
	static T *FindElement()
	{
		for ( const auto &desc : GetElements() )
		{
			T *p = dynamic_cast<T *>( desc.pElement );
			if ( p )
				return p;
		}
		return nullptr;
	}

	// Clears all registered elements (used for testing and clean reinit)
	static void Clear( void );
};

#define REGISTER_HUD_ELEMENT( elementInstance, drawOrder )                     \
	static struct AutoRegisterHud_##elementInstance                            \
	{                                                                          \
		AutoRegisterHud_##elementInstance()                                    \
		{                                                                      \
			HudRegistry::RegisterElement( &elementInstance, drawOrder, #elementInstance ); \
		}                                                                      \
	} s_autoRegisterHud_##elementInstance;

#define REGISTER_HUD_ELEMENT_NAMED( elementInstance, drawOrder, elementName )  \
	static struct AutoRegisterHud_##elementInstance                            \
	{                                                                          \
		AutoRegisterHud_##elementInstance()                                    \
		{                                                                      \
			HudRegistry::RegisterElement( &elementInstance, drawOrder, elementName ); \
		}                                                                      \
	} s_autoRegisterHud_##elementInstance;
