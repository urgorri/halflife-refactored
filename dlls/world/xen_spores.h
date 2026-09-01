/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#ifndef XEN_SPORES_H
#define XEN_SPORES_H

#include "world/xen.h"

class CXenSpore : public CActAnimating
{
  public:
	void Spawn( void );
	void Precache( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
	int Classify( void ) { return CLASS_BARNACLE; }

	static const char *pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
  public:
	void Spawn( void );
};

class CXenSporeMed : public CXenSpore
{
  public:
	void Spawn( void );
};

class CXenSporeLarge : public CXenSpore
{
  public:
	void Spawn( void );

	static const Vector m_hullSizes[];
};

#endif // XEN_SPORES_H
