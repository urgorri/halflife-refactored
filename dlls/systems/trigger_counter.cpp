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

===== trigger_counter.cpp ========================================================

  spawn and use functions for editor-placed triggers

*/

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "player.h"
#include "core/saverestore.h"
#include "world/trains.h" // trigger_camera has train functionality
#include "gameplay/gamerules.h"
#include "trigger_base.h"

class CTriggerCounter : public CBaseTrigger
{
  public:
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( trigger_counter, CTriggerCounter );

void CTriggerCounter ::Spawn( void )
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if ( m_cTriggersLeft == 0 )
		m_cTriggersLeft = 2;
	SetUse( &CBaseTrigger::CounterUse );
}

// ====================== TRIGGER_CHANGELEVEL ================================
