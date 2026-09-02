//========= Copyright ? 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Spectator mode selection, controls, inset toggling, and player targeting.
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "vgui/vgui_TeamFortressViewport.h"
#include "vgui/vgui_SpectatorPanel.h"
#include "hltv.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "entity_types.h"

#include "com_model.h"
#include "demo_api.h"
#include "event_api.h"
#include "studio/studio_util.h"
#include "screenfade.h"

#pragma warning( disable : 4244 )

extern "C" int iJumpSpectator;
extern "C" float vJumpOrigin[3];
extern "C" float vJumpAngles[3];

extern void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles );
extern vec3_t v_cl_angles;

void CHudSpectator::FindNextPlayer( bool bReverse )
{
	// if we are NOT in HLTV mode, spectator targets are set on server
	if ( !gEngfuncs.IsSpectateOnly() )
	{
		char cmdstring[32];
		// forward command to server
		sprintf( cmdstring, "follownext %i", bReverse ? 1 : 0 );
		gEngfuncs.pfnServerCmd( cmdstring );
		return;
	}

	int iStart = g_iUser2;

	if ( iStart < 1 || iStart > MAX_PLAYERS )
		iStart = 1;

	int iCurrent = iStart;

	int iDir = bReverse ? -1 : 1;

	cl_entity_t *pEnt = NULL;

	// make sure we have player info
	gViewPort->GetAllPlayersInfo();

	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if ( iCurrent > MAX_PLAYERS )
			iCurrent = 1;
		if ( iCurrent < 1 )
			iCurrent = MAX_PLAYERS;

		pEnt = gEngfuncs.GetEntityByIndex( iCurrent );

		if ( !IsActivePlayer( pEnt ) )
			continue;

		g_iUser2 = iCurrent;
		break;

	} while ( iCurrent != iStart );

	// Did we find a target?
	if ( !g_iUser2 )
	{
		gEngfuncs.Con_DPrintf( "No observer targets.\n" );
		// take save camera position
		VectorCopy( m_cameraOrigin, vJumpOrigin );
		VectorCopy( m_cameraAngles, vJumpAngles );
	}
	else
	{
		// use new entity position for roaming
		VectorCopy( pEnt->origin, vJumpOrigin );
		VectorCopy( pEnt->angles, vJumpAngles );
	}

	iJumpSpectator = 1;
	gViewPort->MsgFunc_ResetFade( NULL, 0, NULL );
}

void CHudSpectator::FindPlayer( const char *name )
{
	// if we are NOT in HLTV mode, spectator targets are set on server
	if ( !gEngfuncs.IsSpectateOnly() )
	{
		char cmdstring[32];
		// forward command to server
		sprintf( cmdstring, "follow %s", name );
		gEngfuncs.pfnServerCmd( cmdstring );
		return;
	}

	g_iUser2 = 0;

	// make sure we have player info
	gViewPort->GetAllPlayersInfo();

	cl_entity_t *pEnt = NULL;

	for ( int i = 1; i < MAX_PLAYERS; i++ )
	{
		pEnt = gEngfuncs.GetEntityByIndex( i );

		if ( !IsActivePlayer( pEnt ) )
			continue;

		if ( !stricmp( g_PlayerInfoList[pEnt->index].name, name ) )
		{
			g_iUser2 = i;
			break;
		}
	}

	// Did we find a target?
	if ( !g_iUser2 )
	{
		gEngfuncs.Con_DPrintf( "No observer targets.\n" );
		// take save camera position
		VectorCopy( m_cameraOrigin, vJumpOrigin );
		VectorCopy( m_cameraAngles, vJumpAngles );
	}
	else
	{
		// use new entity position for roaming
		VectorCopy( pEnt->origin, vJumpOrigin );
		VectorCopy( pEnt->angles, vJumpAngles );
	}

	iJumpSpectator = 1;
	gViewPort->MsgFunc_ResetFade( NULL, 0, NULL );
}

void CHudSpectator::HandleButtonsDown( int ButtonPressed )
{
	double time = gEngfuncs.GetClientTime();

	int newMainMode  = g_iUser1;
	int newInsetMode = m_pip->value;

	if ( !gViewPort )
		return;

	// Not in intermission.
	if ( gHUD.m_iIntermission )
		return;

	if ( !g_iUser1 )
		return; // dont do anything if not in spectator mode

	// don't handle buttons during normal demo playback
	if ( gEngfuncs.pDemoAPI->IsPlayingback() && !gEngfuncs.IsSpectateOnly() )
		return;
	// Slow down mouse clicks.
	if ( m_flNextObserverInput > time )
		return;

	// enable spectator screen
	if ( ButtonPressed & IN_DUCK )
		gViewPort->m_pSpectatorPanel->ShowMenu( !gViewPort->m_pSpectatorPanel->m_menuVisible );

	//  'Use' changes inset window mode
	if ( ButtonPressed & IN_USE )
	{
		newInsetMode = ToggleInset( true );
	}

	// if not in HLTV mode, buttons are handled server side
	if ( gEngfuncs.IsSpectateOnly() )
	{
		// changing target or chase mode not in overviewmode without inset window

		// Jump changes main window modes
		if ( ButtonPressed & IN_JUMP )
		{
			if ( g_iUser1 == OBS_CHASE_LOCKED )
				newMainMode = OBS_CHASE_FREE;

			else if ( g_iUser1 == OBS_CHASE_FREE )
				newMainMode = OBS_IN_EYE;

			else if ( g_iUser1 == OBS_IN_EYE )
				newMainMode = OBS_ROAMING;

			else if ( g_iUser1 == OBS_ROAMING )
				newMainMode = OBS_MAP_FREE;

			else if ( g_iUser1 == OBS_MAP_FREE )
				newMainMode = OBS_MAP_CHASE;

			else
				newMainMode = OBS_CHASE_FREE;
		}

		// Attack moves to the next player
		if ( ButtonPressed & ( IN_ATTACK | IN_ATTACK2 ) )
		{
			FindNextPlayer( ( ButtonPressed & IN_ATTACK2 ) ? true : false );

			if ( g_iUser1 == OBS_ROAMING )
			{
				gEngfuncs.SetViewAngles( vJumpAngles );
				iJumpSpectator = 1;
			}
			// release directed mode if player wants to see another player
			m_autoDirector->value = 0.0f;
		}
	}

	SetModes( newMainMode, newInsetMode );

	if ( g_iUser1 == OBS_MAP_FREE )
	{
		if ( ButtonPressed & IN_FORWARD )
			m_zoomDelta = 0.01f;

		if ( ButtonPressed & IN_BACK )
			m_zoomDelta = -0.01f;

		if ( ButtonPressed & IN_MOVELEFT )
			m_moveDelta = -12.0f;

		if ( ButtonPressed & IN_MOVERIGHT )
			m_moveDelta = 12.0f;
	}

	m_flNextObserverInput = time + 0.2;
}

void CHudSpectator::HandleButtonsUp( int ButtonPressed )
{
	if ( !gViewPort )
		return;

	if ( !gViewPort->m_pSpectatorPanel->isVisible() )
		return;

	if ( ButtonPressed & ( IN_FORWARD | IN_BACK ) )
		m_zoomDelta = 0.0f;

	if ( ButtonPressed & ( IN_MOVELEFT | IN_MOVERIGHT ) )
		m_moveDelta = 0.0f;
}

void CHudSpectator::SetModes( int iNewMainMode, int iNewInsetMode )
{
	// if value == -1 keep old value
	if ( iNewMainMode == -1 )
		iNewMainMode = g_iUser1;

	if ( iNewInsetMode == -1 )
		iNewInsetMode = m_pip->value;

	// inset mode is handled only clients side
	m_pip->value = iNewInsetMode;

	if ( iNewMainMode < OBS_CHASE_LOCKED || iNewMainMode > OBS_MAP_CHASE )
	{
		gEngfuncs.Con_Printf( "Invalid spectator mode.\n" );
		return;
	}

	m_IsInterpolating = false;
	m_ChaseEntity     = 0;

	// main mode settings will override inset window settings
	if ( iNewMainMode != g_iUser1 )
	{
		// if we are NOT in HLTV mode, main spectator mode is set on server
		if ( !gEngfuncs.IsSpectateOnly() )
		{
			char cmdstring[32];
			// forward command to server
			sprintf( cmdstring, "specmode %i", iNewMainMode );
			gEngfuncs.pfnServerCmd( cmdstring );
			return;
		}

		if ( !g_iUser2 && ( iNewMainMode != OBS_ROAMING ) )
		{
			// choose last Director object if still available
			if ( IsActivePlayer( gEngfuncs.GetEntityByIndex( m_lastPrimaryObject ) ) )
			{
				g_iUser2 = m_lastPrimaryObject;
				g_iUser3 = m_lastSecondaryObject;
			}
			else
				FindNextPlayer( false );
		}

		switch ( iNewMainMode )
		{
		case OBS_CHASE_LOCKED:
			g_iUser1 = OBS_CHASE_LOCKED;
			break;

		case OBS_CHASE_FREE:
			g_iUser1 = OBS_CHASE_FREE;
			break;

		case OBS_ROAMING:
			g_iUser1 = OBS_ROAMING;
			if ( g_iUser2 )
			{
				V_GetChasePos( g_iUser2, v_cl_angles, vJumpOrigin, vJumpAngles );
				gEngfuncs.SetViewAngles( vJumpAngles );
				iJumpSpectator = 1;
			}
			break;

		case OBS_IN_EYE:
			g_iUser1 = OBS_IN_EYE;
			break;

		case OBS_MAP_FREE:
			g_iUser1 = OBS_MAP_FREE;
			m_mapZoom   = m_OverviewData.zoom;
			m_mapOrigin = m_OverviewData.origin;
			break;

		case OBS_MAP_CHASE:
			g_iUser1 = OBS_MAP_CHASE;
			m_mapZoom   = m_OverviewData.zoom;
			m_mapOrigin = m_OverviewData.origin;
			break;
		}

		if ( ( g_iUser1 == OBS_IN_EYE ) || ( g_iUser1 == OBS_ROAMING ) )
		{
			m_crosshairRect.left   = 24;
			m_crosshairRect.top    = 0;
			m_crosshairRect.right  = 48;
			m_crosshairRect.bottom = 24;

			SetCrosshair( m_hCrosshair, m_crosshairRect, 255, 255, 255 );
		}
		else
		{
			memset( &m_crosshairRect, 0, sizeof( m_crosshairRect ) );
			SetCrosshair( 0, m_crosshairRect, 0, 0, 0 );
		}

		gViewPort->MsgFunc_ResetFade( NULL, 0, NULL );

		char string[128];
		sprintf( string, "#Spec_Mode%d", g_iUser1 );
		sprintf( string, "%c%s", HUD_PRINTCENTER, CHudTextMessage::BufferedLocaliseTextString( string ) );
		gHUD.m_TextMessage.MsgFunc_TextMsg( NULL, strlen( string ) + 1, string );
	}

	gViewPort->UpdateSpectatorPanel();
}

bool CHudSpectator::IsActivePlayer( cl_entity_t *ent )
{
	return ( ent &&
	         ent->player &&
	         ent->curstate.solid != SOLID_NOT &&
	         ent != gEngfuncs.GetLocalPlayer() &&
	         g_PlayerInfoList[ent->index].name != NULL );
}

void CHudSpectator::CheckSettings()
{
	m_pip->value = (int)m_pip->value;

	if ( ( g_iUser1 < OBS_MAP_FREE ) && ( m_pip->value == INSET_CHASE_FREE || m_pip->value == INSET_IN_EYE ) )
	{
		m_pip->value = INSET_MAP_FREE;
	}

	if ( ( g_iUser1 >= OBS_MAP_FREE ) && ( m_pip->value >= INSET_MAP_FREE ) )
	{
		m_pip->value = INSET_CHASE_FREE;
	}

	if ( gHUD.m_iIntermission )
		m_pip->value = INSET_OFF;

	if ( m_chatEnabled != ( gHUD.m_SayText.m_HUD_saytext->value != 0 ) )
	{
		m_chatEnabled = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );

		if ( gEngfuncs.IsSpectateOnly() )
		{
			char chatcmd[32];
			sprintf( chatcmd, "ignoremsg %i", m_chatEnabled ? 0 : 1 );
			gEngfuncs.pfnServerCmd( chatcmd );
		}
	}

	if ( ( g_iUser1 == OBS_IN_EYE ) || ( g_iUser1 == OBS_ROAMING ) )
	{
		m_crosshairRect.left   = 24;
		m_crosshairRect.top    = 0;
		m_crosshairRect.right  = 48;
		m_crosshairRect.bottom = 24;

		SetCrosshair( m_hCrosshair, m_crosshairRect, 255, 255, 255 );
	}
	else
	{
		memset( &m_crosshairRect, 0, sizeof( m_crosshairRect ) );
		SetCrosshair( 0, m_crosshairRect, 0, 0, 0 );
	}

	if ( ( ( g_iTeamNumber == 1 ) || ( g_iTeamNumber == 2 ) ) && ( g_iUser1 == OBS_IN_EYE ) )
		m_pip->value = INSET_OFF;

	gViewPort->m_pSpectatorPanel->EnableInsetView( m_pip->value != INSET_OFF );
}

int CHudSpectator::ToggleInset( bool allowOff )
{
	int newInsetMode = (int)m_pip->value + 1;

	if ( g_iUser1 < OBS_MAP_FREE )
	{
		if ( newInsetMode > INSET_MAP_CHASE )
		{
			if ( allowOff )
				newInsetMode = INSET_OFF;
			else
				newInsetMode = INSET_MAP_FREE;
		}

		if ( newInsetMode == INSET_CHASE_FREE )
			newInsetMode = INSET_MAP_FREE;
	}
	else
	{
		if ( newInsetMode > INSET_IN_EYE )
		{
			if ( allowOff )
				newInsetMode = INSET_OFF;
			else
				newInsetMode = INSET_CHASE_FREE;
		}
	}

	return newInsetMode;
}
