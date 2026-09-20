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

#ifndef HUD_ACTIVE
#define HUD_ACTIVE 1
#endif

#ifndef HUD_INTERMISSION
#define HUD_INTERMISSION 2
#endif

#ifndef _POSITION_DEFINED
#define _POSITION_DEFINED
typedef struct
{
	int x, y;
} POSITION;
#endif

//
//-----------------------------------------------------
// Base class for all HUD elements
//
class CHudBase
{
  public:
	POSITION m_pos;
	int m_type;
	int m_iFlags; // active, moving,
	virtual ~CHudBase() {}
	virtual int Init( void ) { return 0; }
	virtual int VidInit( void ) { return 0; }
	virtual int Draw( float flTime ) { return 0; }
	virtual void Think( void ) { return; }
	virtual void Reset( void ) { return; }
	virtual void InitHUDData( void ) {} // called every time a server is connected to
};

struct HUDLIST
{
	CHudBase *p;
	HUDLIST *pNext;
};
