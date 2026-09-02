//========= Copyright ? 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Spectator director message processing, waypoints, and camera control.
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

extern void V_ResetChaseCam();
extern vec3_t v_origin;

static int UTIL_FindEntityInMap( const char *name, float *origin, float *angle )
{
	int n, found = 0;
	char keyname[256];
	char token[1024];

	cl_entity_t *pEnt = gEngfuncs.GetEntityByIndex( 0 );

	if ( !pEnt )
		return 0;

	if ( !pEnt->model )
		return 0;

	char *data = pEnt->model->entities;

	while ( data )
	{
		data = gEngfuncs.COM_ParseFile( data, token );

		if ( ( token[0] == '}' ) || ( token[0] == 0 ) )
			break;

		if ( !data )
		{
			gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
			return 0;
		}

		if ( token[0] != '{' )
		{
			gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: expected {\n" );
			return 0;
		}

		while ( 1 )
		{
			data = gEngfuncs.COM_ParseFile( data, token );
			if ( token[0] == '}' )
				break;

			if ( !data )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
				return 0;
			}

			strcpy( keyname, token );

			n = strlen( keyname );
			while ( n && keyname[n - 1] == ' ' )
			{
				keyname[n - 1] = 0;
				n--;
			}

			data = gEngfuncs.COM_ParseFile( data, token );
			if ( !data )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
				return 0;
			}

			if ( token[0] == '}' )
			{
				gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: closing brace without data" );
				return 0;
			}

			if ( !strcmp( keyname, "classname" ) )
			{
				if ( !strcmp( token, name ) )
				{
					found = 1;
				}
			}

			if ( !strcmp( keyname, "angle" ) )
			{
				float y = atof( token );

				if ( y >= 0 )
				{
					angle[0] = 0.0f;
					angle[1] = y;
					angle[2] = 0.0f;
				}
				else if ( (int)y == -1 )
				{
					angle[0] = -90.0f;
					angle[1] = 0.0f;
					angle[2] = 0.0f;
				}
				else
				{
					angle[0] = 90.0f;
					angle[1] = 0.0f;
					angle[2] = 0.0f;
				}
			}

			if ( !strcmp( keyname, "angles" ) )
			{
				sscanf( token, "%f %f %f", &angle[0], &angle[1], &angle[2] );
			}

			if ( !strcmp( keyname, "origin" ) )
			{
				sscanf( token, "%f %f %f", &origin[0], &origin[1], &origin[2] );
			}
		}

		if ( found )
			return 1;
	}

	return 0;
}

void CHudSpectator::SetSpectatorStartPosition()
{
	// search for info_player start
	if ( UTIL_FindEntityInMap( "trigger_camera", m_cameraOrigin, m_cameraAngles ) )
		iJumpSpectator = 1;

	else if ( UTIL_FindEntityInMap( "info_player_start", m_cameraOrigin, m_cameraAngles ) )
		iJumpSpectator = 1;

	else if ( UTIL_FindEntityInMap( "info_player_deathmatch", m_cameraOrigin, m_cameraAngles ) )
		iJumpSpectator = 1;

	else if ( UTIL_FindEntityInMap( "info_player_coop", m_cameraOrigin, m_cameraAngles ) )
		iJumpSpectator = 1;
	else
	{
		// jump to 0,0,0 if no better position was found
		VectorCopy( vec3_origin, m_cameraOrigin );
		VectorCopy( vec3_origin, m_cameraAngles );
	}

	VectorCopy( m_cameraOrigin, vJumpOrigin );
	VectorCopy( m_cameraAngles, vJumpAngles );

	iJumpSpectator = 1; // jump anyway
}

void CHudSpectator::SetCameraView( vec3_t pos, vec3_t angle, float fov )
{
	m_FOV = fov;
	VectorCopy( pos, vJumpOrigin );
	VectorCopy( angle, vJumpAngles );
	gEngfuncs.SetViewAngles( vJumpAngles );
	iJumpSpectator = 1; // jump anyway
}

void CHudSpectator::AddWaypoint( float time, vec3_t pos, vec3_t angle, float fov, int flags )
{
	if ( !flags == 0 && time == 0.0f )
	{
		// switch instantly to this camera view
		SetCameraView( pos, angle, fov );
		return;
	}

	if ( m_NumWayPoints >= MAX_CAM_WAYPOINTS )
	{
		gEngfuncs.Con_Printf( "Too many camera waypoints!\n" );
		return;
	}

	VectorCopy( angle, m_CamPath[m_NumWayPoints].angle );
	VectorCopy( pos, m_CamPath[m_NumWayPoints].position );
	m_CamPath[m_NumWayPoints].flags = flags;
	m_CamPath[m_NumWayPoints].fov   = fov;
	m_CamPath[m_NumWayPoints].time  = time;

	gEngfuncs.Con_DPrintf( "Added waypoint %i\n", m_NumWayPoints );

	m_NumWayPoints++;
}

void CHudSpectator::SetWayInterpolation( cameraWayPoint_t *prev, cameraWayPoint_t *start, cameraWayPoint_t *end, cameraWayPoint_t *next )
{
	m_WayInterpolation.SetViewAngles( start->angle, end->angle );

	m_WayInterpolation.SetFOVs( start->fov, end->fov );

	m_WayInterpolation.SetSmoothing( ( start->flags & DRC_FLAG_SLOWSTART ) != 0,
	                                 ( start->flags & DRC_FLAG_SLOWEND ) != 0 );

	if ( prev && next )
	{
		m_WayInterpolation.SetWaypoints( &prev->position, start->position, end->position, &next->position );
	}
	else if ( prev )
	{
		m_WayInterpolation.SetWaypoints( &prev->position, start->position, end->position, NULL );
	}
	else if ( next )
	{
		m_WayInterpolation.SetWaypoints( NULL, start->position, end->position, &next->position );
	}
	else
	{
		m_WayInterpolation.SetWaypoints( NULL, start->position, end->position, NULL );
	}
}

bool CHudSpectator::GetDirectorCamera( vec3_t &position, vec3_t &angle )
{
	float now = gHUD.m_flTime;
	float fov = 90.0f;

	if ( m_ChaseEntity )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( m_ChaseEntity );

		if ( ent )
		{
			vec3_t vt = ent->curstate.origin;

			if ( m_ChaseEntity <= gEngfuncs.GetMaxClients() )
			{
				if ( ent->curstate.solid == SOLID_NOT )
				{
					vt[2] += -8; // PM_DEAD_VIEWHEIGHT
				}
				else if ( ent->curstate.usehull == 1 )
				{
					vt[2] += 12; // VEC_DUCK_VIEW;
				}
				else
				{
					vt[2] += 28; // DEFAULT_VIEWHEIGHT
				}
			}

			vt = vt - position;
			VectorAngles( vt, angle );
			angle[0] = -angle[0];
			return true;
		}
		else
		{
			return false;
		}
	}

	if ( !m_IsInterpolating )
		return false;

	if ( m_WayPoint < 0 || m_WayPoint >= ( m_NumWayPoints - 1 ) )
		return false;

	cameraWayPoint_t *wp1 = &m_CamPath[m_WayPoint];
	cameraWayPoint_t *wp2 = &m_CamPath[m_WayPoint + 1];

	if ( now < wp1->time )
		return false;

	while ( now > wp2->time )
	{
		// go to next waypoint, if possible
		m_WayPoint++;

		if ( m_WayPoint >= ( m_NumWayPoints - 1 ) )
		{
			m_IsInterpolating = false;
			return false; // there is no following waypoint
		}

		wp1 = wp2;
		wp2 = &m_CamPath[m_WayPoint + 1];

		if ( m_WayPoint > 0 )
		{
			// we have a predecessor

			if ( m_WayPoint < ( m_NumWayPoints - 1 ) )
			{
				// we have also a successor
				SetWayInterpolation( &m_CamPath[m_WayPoint - 1], wp1, wp2, &m_CamPath[m_WayPoint + 2] );
			}
			else
			{
				SetWayInterpolation( &m_CamPath[m_WayPoint - 1], wp1, wp2, NULL );
			}
		}
		else if ( m_WayPoint < ( m_NumWayPoints - 1 ) )
		{
			// we only have a successor
			SetWayInterpolation( NULL, wp1, wp2, &m_CamPath[m_WayPoint + 2] );
		}
		else
		{
			// we have only two waypoints
			SetWayInterpolation( NULL, wp1, wp2, NULL );
		}
	}

	if ( wp2->time <= wp1->time )
		return false;

	float fraction = ( now - wp1->time ) / ( wp2->time - wp1->time );

	if ( fraction < 0.0f )
		fraction = 0.0f;
	else if ( fraction > 1.0f )
		fraction = 1.0f;

	m_WayInterpolation.Interpolate( fraction, position, angle, &fov );

	SetCameraView( position, angle, fov );

	return true;
}

float CHudSpectator::GetFOV()
{
	return m_FOV;
}

void CHudSpectator::DirectorMessage( int iSize, void *pbuf )
{
	float f1, f2;
	char *string;
	vec3_t v1, v2;
	int i1, i2, i3;

	BEGIN_READ( pbuf, iSize );

	int cmd = READ_BYTE();

	switch ( cmd ) // director command byte
	{
	case DRC_CMD_START:
		// now we have to do some things clientside, since the proxy doesn't know our mod
		g_iPlayerClass = 0;
		g_iTeamNumber  = 0;

		// fake a InitHUD & ResetHUD message
		gHUD.MsgFunc_InitHUD( NULL, 0, NULL );
		gHUD.MsgFunc_ResetHUD( NULL, 0, NULL );

		break;

	case DRC_CMD_EVENT: // old director style message
		m_lastPrimaryObject   = READ_WORD();
		m_lastSecondaryObject = READ_WORD();
		m_iObserverFlags      = READ_LONG();

		if ( m_autoDirector->value )
		{
			if ( ( g_iUser2 != m_lastPrimaryObject ) || ( g_iUser3 != m_lastSecondaryObject ) )
				V_ResetChaseCam();

			g_iUser2          = m_lastPrimaryObject;
			g_iUser3          = m_lastSecondaryObject;
			m_IsInterpolating = false;
			m_ChaseEntity     = 0;
		}
		break;
	case DRC_CMD_MODE:
		if ( m_autoDirector->value )
		{
			SetModes( READ_BYTE(), -1 );
		}
		break;

	case DRC_CMD_CAMERA:
		v1[0] = READ_COORD(); // position
		v1[1] = READ_COORD();
		v1[2] = READ_COORD(); // vJumpOrigin

		v2[0] = READ_COORD(); // view angle
		v2[1] = READ_COORD(); // vJumpAngles
		v2[2] = READ_COORD();
		f1    = READ_BYTE(); // fov
		i1    = READ_WORD(); // target

		if ( m_autoDirector->value )
		{
			SetModes( OBS_ROAMING, -1 );
			SetCameraView( v1, v2, f1 );
			m_ChaseEntity = i1;
		}
		break;

	case DRC_CMD_MESSAGE:
	{
		client_textmessage_t *msg = &m_HUDMessages[m_lastHudMessage];

		msg->effect = READ_BYTE(); // effect

		UnpackRGB( (int &)msg->r1, (int &)msg->g1, (int &)msg->b1, READ_LONG() ); // color
		msg->r2 = msg->r1;
		msg->g2 = msg->g1;
		msg->b2 = msg->b1;
		msg->a2 = msg->a1 = 0xFF; // not transparent

		msg->x = READ_FLOAT(); // x pos
		msg->y = READ_FLOAT(); // y pos

		msg->fadein   = READ_FLOAT(); // fadein
		msg->fadeout  = READ_FLOAT(); // fadeout
		msg->holdtime = READ_FLOAT(); // holdtime
		msg->fxtime   = READ_FLOAT(); // fxtime;

		strncpy( m_HUDMessageText[m_lastHudMessage], READ_STRING(), 128 );
		m_HUDMessageText[m_lastHudMessage][127] = 0; // text

		msg->pMessage = m_HUDMessageText[m_lastHudMessage];
		msg->pName    = "HUD_MESSAGE";

		gHUD.m_Message.MessageAdd( msg );

		m_lastHudMessage++;
		m_lastHudMessage %= MAX_SPEC_HUD_MESSAGES;
	}

	break;

	case DRC_CMD_SOUND:
		string = READ_STRING();
		f1     = READ_FLOAT();

		gEngfuncs.pEventAPI->EV_PlaySound( 0, v_origin, CHAN_BODY, string, f1, ATTN_NORM, 0, PITCH_NORM );

		break;

	case DRC_CMD_TIMESCALE:
		f1 = READ_FLOAT(); // ignore this command (maybe show slowmo sign)
		break;

	case DRC_CMD_STATUS:
		READ_LONG();                      // total number of spectator slots
		m_iSpectatorNumber = READ_LONG(); // total number of spectator
		READ_WORD();                      // total number of relay proxies

		gViewPort->UpdateSpectatorPanel();
		break;

	case DRC_CMD_BANNER:
		gViewPort->m_pSpectatorPanel->m_TopBanner->LoadImage( READ_STRING() );
		gViewPort->UpdateSpectatorPanel();
		break;

	case DRC_CMD_STUFFTEXT:
		EngineFilteredClientCmd( READ_STRING() );
		break;

	case DRC_CMD_CAMPATH:
		v1[0] = READ_COORD(); // position
		v1[1] = READ_COORD();
		v1[2] = READ_COORD(); // vJumpOrigin

		v2[0] = READ_COORD(); // view angle
		v2[1] = READ_COORD(); // vJumpAngles
		v2[2] = READ_COORD();
		f1    = READ_BYTE(); // FOV
		i1    = READ_BYTE(); // flags

		if ( m_autoDirector->value )
		{
			SetModes( OBS_ROAMING, -1 );
			SetCameraView( v1, v2, f1 );
		}
		break;

	case DRC_CMD_WAYPOINTS:
		i1             = READ_BYTE();
		m_NumWayPoints = 0;
		m_WayPoint     = 0;
		for ( i2 = 0; i2 < i1; i2++ )
		{
			f1 = gHUD.m_flTime + (float)( READ_SHORT() ) / 100.0f;

			v1[0] = READ_COORD(); // position
			v1[1] = READ_COORD();
			v1[2] = READ_COORD(); // vJumpOrigin

			v2[0] = READ_COORD(); // view angle
			v2[1] = READ_COORD(); // vJumpAngles
			v2[2] = READ_COORD();
			f2    = READ_BYTE(); // fov
			i3    = READ_BYTE(); // flags

			AddWaypoint( f1, v1, v2, f2, i3 );
		}

		if ( !m_autoDirector->value )
		{
			// ignore waypoints
			m_NumWayPoints = 0;
			break;
		}

		SetModes( OBS_ROAMING, -1 );

		m_IsInterpolating = true;

		if ( m_NumWayPoints > 2 )
		{
			SetWayInterpolation( NULL, &m_CamPath[0], &m_CamPath[1], &m_CamPath[2] );
		}
		else
		{
			SetWayInterpolation( NULL, &m_CamPath[0], &m_CamPath[1], NULL );
		}
		break;

	default:
		gEngfuncs.Con_DPrintf( "CHudSpectator::DirectorMessage: unknown command %i.\n", cmd );
	}
}
