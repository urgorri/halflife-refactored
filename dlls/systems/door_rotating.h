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
#ifndef DOOR_ROTATING_H
#define DOOR_ROTATING_H

#include "systems/doors.h"

class CRotDoor : public CBaseDoor
{
  public:
	void Spawn( void );
	virtual void SetToggleState( int state );
};

#endif // DOOR_ROTATING_H
