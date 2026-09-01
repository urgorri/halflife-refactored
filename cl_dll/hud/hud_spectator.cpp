//========= Copyright ? 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Main spectator HUD lifecycle, initialization, reset, and drawing coordinator.
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

extern void V_GetInEyePos( int entity, float *origin, float *angles );
extern void V_ResetChaseCam();
extern void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles );
extern float *GetClientColor( int clientIndex );

extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_cl_angles;
extern vec3_t v_sim_org;

void SpectatorMode( void )
{
	if ( gEngfuncs.Cmd_Argc() <= 1 )
	{
		gEngfuncs.Con_Printf( "usage:  spec_mode <Main Mode> [<Inset Mode>]\n" );
		return;
	}

	if ( gEngfuncs.Cmd_Argc() == 2 )
		gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), -1 );
	else if ( gEngfuncs.Cmd_Argc() == 3 )
		gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), atoi( gEngfuncs.Cmd_Argv( 2 ) ) );
}

void SpectatorSpray( void )
{
	vec3_t forward;
	char string[128];

	if ( !gEngfuncs.IsSpectateOnly() )
		return;

	AngleVectors( v_angles, forward, NULL, NULL );
	VectorScale( forward, 128, forward );
	VectorAdd( forward, v_origin, forward );
	pmtrace_t *trace = gEngfuncs.PM_TraceLine( v_origin, forward, PM_TRACELINE_PHYSENTSONLY, 2, -1 );
	if ( trace->fraction != 1.0 )
	{
		sprintf( string, "drc_spray %.2f %.2f %.2f %i", trace->endpos[0], trace->endpos[1], trace->endpos[2], trace->ent );
		gEngfuncs.pfnServerCmd( string );
	}
}

void SpectatorHelp( void )
{
	if ( gViewPort )
	{
		gViewPort->ShowVGUIMenu( MENU_SPECHELP );
	}
	else
	{
		char *text = CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help_Text" );

		if ( text )
		{
			while ( *text )
			{
				if ( *text != 13 )
					gEngfuncs.Con_Printf( "%c", *text );
				text++;
			}
		}
	}
}

void SpectatorMenu( void )
{
	if ( gEngfuncs.Cmd_Argc() <= 1 )
	{
		gEngfuncs.Con_Printf( "usage:  spec_menu <0|1>\n" );
		return;
	}

	gViewPort->m_pSpectatorPanel->ShowMenu( atoi( gEngfuncs.Cmd_Argv( 1 ) ) != 0 );
}

void ToggleScores( void )
{
	if ( gViewPort )
	{
		if ( gViewPort->IsScoreBoardVisible() )
		{
			gViewPort->HideScoreBoard();
		}
		else
		{
			gViewPort->ShowScoreBoard();
		}
	}
}

int CHudSpectator::Init()
{
	gHUD.AddHudElem( this );

	m_iFlags |= HUD_ACTIVE;
	m_flNextObserverInput = 0.0f;
	m_zoomDelta           = 0.0f;
	m_moveDelta           = 0.0f;
	m_chatEnabled         = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );
	m_iObserverFlags      = 0;
	m_iSpectatorNumber    = 0;
	m_FOV                 = 90.0f;
	m_IsInterpolating     = false;
	m_ChaseEntity         = 0;

	memset( &m_OverviewData, 0, sizeof( m_OverviewData ) );
	memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );
	memset( &m_HUDMessages, 0, sizeof( m_HUDMessages ) );
	m_lastHudMessage = 0;

	m_autoDirector = gEngfuncs.pfnRegisterVariable( "spec_autodirector", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	m_pip          = gEngfuncs.pfnRegisterVariable( "spec_pip", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	m_drawnames    = gEngfuncs.pfnRegisterVariable( "spec_drawnames", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	m_drawcone     = gEngfuncs.pfnRegisterVariable( "spec_drawcone", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
	m_drawstatus   = gEngfuncs.pfnRegisterVariable( "spec_drawstatus", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

	if ( !m_autoDirector || !m_pip || !m_drawnames || !m_drawcone || !m_drawstatus )
	{
		gEngfuncs.Con_Printf( "ERROR! Couldn't register spectator cvars!\n" );
		return 0;
	}

	gEngfuncs.pfnAddCommand( "spec_mode", SpectatorMode );
	gEngfuncs.pfnAddCommand( "spec_decal", SpectatorSpray );
	gEngfuncs.pfnAddCommand( "spec_help", SpectatorHelp );
	gEngfuncs.pfnAddCommand( "spec_menu", SpectatorMenu );
	gEngfuncs.pfnAddCommand( "togglescores", ToggleScores );

	m_hsprPlayerBlue = 0;
	m_hsprPlayerRed  = 0;
	m_hsprPlayer     = 0;
	m_hsprCamera     = 0;
	m_hsprPlayerDead = 0;
	m_hsprViewcone   = 0;
	m_hsprUnkownMap  = 0;
	m_hsprBeam       = 0;
	m_hCrosshair     = 0;

	return 1;
}

int CHudSpectator::VidInit()
{
	m_hsprPlayerBlue = gHUD.GetSpriteIndex( "spec_player_blue" );
	m_hsprPlayerRed  = gHUD.GetSpriteIndex( "spec_player_red" );
	m_hsprPlayer     = gHUD.GetSpriteIndex( "spec_player" );
	m_hsprCamera     = gHUD.GetSpriteIndex( "spec_camera" );
	m_hsprPlayerDead = gHUD.GetSpriteIndex( "spec_player_dead" );
	m_hsprViewcone   = gHUD.GetSpriteIndex( "spec_viewcone" );
	m_hsprUnkownMap  = gHUD.GetSpriteIndex( "spec_unknown_map" );
	m_hsprBeam       = gHUD.GetSpriteIndex( "spec_beam" );
	m_hCrosshair     = gHUD.GetSpriteIndex( "spec_crosshair" );

	return 1;
}

int CHudSpectator::Draw( float flTime )
{
	int lx;
	char string[256];
	float *color;

	if ( !g_iUser1 )
		return 0;

	if ( ( m_zoomDelta != 0.0f ) && ( g_iUser1 == OBS_MAP_FREE ) )
	{
		m_mapZoom += m_zoomDelta;

		if ( m_mapZoom > 3.0f )
			m_mapZoom = 3.0f;

		if ( m_mapZoom < 0.5f )
			m_mapZoom = 0.5f;
	}

	if ( ( m_moveDelta != 0.0f ) && ( g_iUser1 != OBS_ROAMING ) )
	{
		vec3_t right;
		AngleVectors( v_angles, NULL, right, NULL );
		VectorNormalize( right );
		VectorScale( right, m_moveDelta, right );

		VectorAdd( m_mapOrigin, right, m_mapOrigin );
	}

	if ( g_iUser1 < OBS_MAP_FREE )
		return 1;

	if ( !m_drawnames->value )
		return 1;

	gViewPort->GetAllPlayersInfo();

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if ( m_vPlayerPos[i][2] < 0 )
			continue;

		if ( m_pip->value != INSET_OFF )
		{
			if ( m_vPlayerPos[i][0] > XRES_HD( m_OverviewData.insetWindowX ) &&
			     m_vPlayerPos[i][1] > YRES_HD( m_OverviewData.insetWindowY ) &&
			     m_vPlayerPos[i][0] < XRES_HD( m_OverviewData.insetWindowX + m_OverviewData.insetWindowWidth ) &&
			     m_vPlayerPos[i][1] < YRES_HD( m_OverviewData.insetWindowY + m_OverviewData.insetWindowHeight ) )
				continue;
		}

		color = GetClientColor( i + 1 );

		sprintf( string, "%s", g_PlayerInfoList[i + 1].name );

		lx = strlen( string ) * 3;

		gEngfuncs.pfnDrawSetTextColor( color[0], color[1], color[2] );
		DrawConsoleString( m_vPlayerPos[i][0] - lx, m_vPlayerPos[i][1], string );
	}

	return 1;
}

void CHudSpectator::Reset()
{
	m_IsInterpolating = false;
	m_ChaseEntity     = 0;
	m_WayPoint        = 0;
	m_NumWayPoints    = 0;
	m_FOV             = 90.0f;

	SetModes( OBS_CHASE_FREE, INSET_OFF );
	SetSpectatorStartPosition();

	memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );
	memset( &m_HUDMessages, 0, sizeof( m_HUDMessages ) );
	m_lastHudMessage = 0;

	CheckSettings();
}

void CHudSpectator::InitHUDData( void )
{
	m_lastPrimaryObject   = 0;
	m_lastSecondaryObject = 0;
	m_iObserverFlags      = 0;
	m_iSpectatorNumber    = 0;
	m_lastHudMessage      = 0;

	memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );
	memset( &m_HUDMessages, 0, sizeof( m_HUDMessages ) );

	if ( ParseOverviewFile() )
	{
		LoadMapSprites();
	}

	CheckSettings();
}
