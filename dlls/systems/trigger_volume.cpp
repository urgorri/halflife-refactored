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
/*

===== trigger_volume.cpp ========================================================

  spawn and use functions for editor-placed triggers

*/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/player.h"
#include "core/saverestore.h"
#include "world/trains.h" // trigger_camera has train functionality
#include "gameplay/gamerules.h"

class CTriggerVolume : public CPointEntity // Derive from point entity so this doesn't move across levels
{
  public:
	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_transition, CTriggerVolume );

// Define space that travels across a level transition
void CTriggerVolume ::Spawn( void )
{
	pev->solid    = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) ); // set size and link into world
	pev->model      = NULL;
	pev->modelindex = 0;
}

// Fires a target after level transition and then dies
