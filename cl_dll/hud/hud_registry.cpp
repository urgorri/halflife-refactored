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

#include "hud_registry.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>

#ifdef CLIENT_DLL
#include "hud.h"
#else
void CHud::AddHudElem( CHudBase *phudelem )
{
	if ( !phudelem )
		return;

	for ( HUDLIST *p = m_pHudList; p; p = p->pNext )
	{
		if ( p->p == phudelem )
			return;
	}

	HUDLIST *pdl = (HUDLIST *)malloc( sizeof( HUDLIST ) );
	if ( !pdl )
		return;

	pdl->p     = phudelem;
	pdl->pNext = nullptr;

	if ( !m_pHudList )
	{
		m_pHudList = pdl;
		return;
	}

	HUDLIST *ptemp = m_pHudList;
	while ( ptemp->pNext )
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

void CHud::RemoveHudElem( CHudBase *phudelem )
{
	if ( !phudelem || !m_pHudList )
		return;

	HUDLIST *pCurr = m_pHudList;
	HUDLIST *pPrev = nullptr;

	while ( pCurr )
	{
		if ( pCurr->p == phudelem )
		{
			if ( pPrev )
				pPrev->pNext = pCurr->pNext;
			else
				m_pHudList = pCurr->pNext;

			free( pCurr );
			return;
		}
		pPrev = pCurr;
		pCurr = pCurr->pNext;
	}
}
#endif

static std::vector<HudElementDescriptor> &GetRegistryStorage()
{
	static std::vector<HudElementDescriptor> s_elements;
	return s_elements;
}

void HudRegistry::RegisterElement( CHudBase *pElement, int iDrawOrder, const char *pszName, pfnHudCustomInit pfnInit )
{
	if ( !pElement )
		return;

	auto &storage = GetRegistryStorage();
	for ( auto &desc : storage )
	{
		if ( desc.pElement == pElement )
		{
			desc.iDrawOrder = iDrawOrder;
			if ( pszName )
				desc.pszName = pszName;
			if ( pfnInit )
				desc.pfnInit = pfnInit;
			return;
		}
	}

	HudElementDescriptor desc;
	desc.pElement   = pElement;
	desc.iDrawOrder = iDrawOrder;
	desc.pszName    = pszName;
	desc.pfnInit    = pfnInit;
	storage.push_back( desc );
}

bool HudRegistry::UnregisterElement( CHudBase *pElement )
{
	if ( !pElement )
		return false;

	auto &storage = GetRegistryStorage();
	for ( auto it = storage.begin(); it != storage.end(); ++it )
	{
		if ( it->pElement == pElement )
		{
			storage.erase( it );
			return true;
		}
	}
	return false;
}

void HudRegistry::RegisterBaseElements( CHud *pHud )
{
	if ( !pHud )
		return;

#ifdef CLIENT_DLL
	RegisterElement( &pHud->m_Ammo, 100, "Ammo" );
	RegisterElement( &pHud->m_Health, 200, "Health" );
	RegisterElement( &pHud->m_SayText, 300, "SayText" );
	RegisterElement( &pHud->m_Spectator, 400, "Spectator" );
	RegisterElement( &pHud->m_Geiger, 500, "Geiger" );
	RegisterElement( &pHud->m_Train, 600, "Train" );
	RegisterElement( &pHud->m_Battery, 700, "Battery" );
	RegisterElement( &pHud->m_Flash, 800, "Flashlight" );
	RegisterElement( &pHud->m_Message, 900, "Message" );
	RegisterElement( &pHud->m_StatusBar, 1000, "StatusBar" );
	RegisterElement( &pHud->m_DeathNotice, 1100, "DeathNotice" );
	RegisterElement( &pHud->m_AmmoSecondary, 1200, "AmmoSecondary" );
	RegisterElement( &pHud->m_TextMessage, 1300, "TextMessage" );
	RegisterElement( &pHud->m_StatusIcons, 1400, "StatusIcons" );
	RegisterElement( GetClientVoiceMgr(), 1500, "VoiceStatus" );
	RegisterElement( &pHud->m_Menu, 1600, "Menu" );
	RegisterElement( &pHud->m_Benchmark, 1700, "Benchmark" );
#endif
}

void HudRegistry::AttachAll( CHud *pHud )
{
	if ( !pHud )
		return;

	auto &storage = GetRegistryStorage();
	std::stable_sort( storage.begin(), storage.end(),
	    []( const HudElementDescriptor &a, const HudElementDescriptor &b ) {
		    return a.iDrawOrder < b.iDrawOrder;
	    } );

	for ( auto &desc : storage )
	{
		if ( !desc.pElement )
			continue;

		if ( desc.pfnInit )
		{
			desc.pfnInit( desc.pElement );
		}
		else
		{
			desc.pElement->Init();
		}

		pHud->AddHudElem( desc.pElement );
	}
}

void HudRegistry::VidInitAll( CHud *pHud )
{
	if ( !pHud )
		return;

	for ( HUDLIST *pList = pHud->GetHudList(); pList; pList = pList->pNext )
	{
		if ( pList->p )
		{
			pList->p->VidInit();
		}
	}
}

const std::vector<HudElementDescriptor> &HudRegistry::GetElements( void )
{
	return GetRegistryStorage();
}

CHudBase *HudRegistry::FindElement( const char *pszName )
{
	if ( !pszName )
		return nullptr;

	const auto &storage = GetRegistryStorage();
	for ( const auto &desc : storage )
	{
		if ( desc.pszName && strcmp( desc.pszName, pszName ) == 0 )
		{
			return desc.pElement;
		}
	}
	return nullptr;
}

void HudRegistry::Clear( void )
{
	GetRegistryStorage().clear();
}
