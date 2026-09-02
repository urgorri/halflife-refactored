//========= Copyright ? 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Spectator map overview parser, map sprites, radar layer, and overview entity rendering.
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

extern extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];

extern void V_GetInEyePos( int entity, float *origin, float *angles );
extern void V_GetChasePos( int target, float *cl_angles, float *origin, float *angles );
extern float *GetClientColor( int clientIndex );

extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_cl_angles;
extern vec3_t v_sim_org;

bool CHudSpectator::ParseOverviewFile()
{
	char filename[255];
	char levelname[255];
	char token[1024];
	float height;

	char *pfile = NULL;

	memset( &m_OverviewData, 0, sizeof( m_OverviewData ) );

	// fill in standrd values
	m_OverviewData.insetWindowX      = 4; // upper left corner
	m_OverviewData.insetWindowY      = 4;
	m_OverviewData.insetWindowHeight = 180;
	m_OverviewData.insetWindowWidth  = 240;
	m_OverviewData.origin[0]         = 0.0f;
	m_OverviewData.origin[1]         = 0.0f;
	m_OverviewData.origin[2]         = 0.0f;
	m_OverviewData.zoom              = 1.0f;
	m_OverviewData.layers            = 0;
	m_OverviewData.layersHeights[0]  = 0.0f;
	strcpy( m_OverviewData.map, gEngfuncs.pfnGetLevelName() );

	if ( strlen( m_OverviewData.map ) == 0 )
		return false; // not active yet

	strcpy( levelname, m_OverviewData.map + 5 );
	levelname[strlen( levelname ) - 4] = 0;

	sprintf( filename, "overviews/%s.txt", levelname );

	pfile = (char *)gEngfuncs.COM_LoadFile( filename, 5, NULL );

	if ( !pfile )
	{
		gEngfuncs.Con_DPrintf( "Couldn't open file %s. Using default values for overiew mode.\n", filename );
		return false;
	}

	while ( true )
	{
		pfile = gEngfuncs.COM_ParseFile( pfile, token );

		if ( !pfile )
			break;

		if ( !stricmp( token, "global" ) )
		{
			// parse the global data
			pfile = gEngfuncs.COM_ParseFile( pfile, token );
			if ( stricmp( token, "{" ) )
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename );
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			while ( stricmp( token, "}" ) )
			{
				if ( !stricmp( token, "zoom" ) )
				{
					pfile               = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.zoom = atof( token );
				}
				else if ( !stricmp( token, "origin" ) )
				{
					pfile                    = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.origin[0] = atof( token );
					pfile                    = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.origin[1] = atof( token );
					pfile                    = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.origin[2] = atof( token );
				}
				else if ( !stricmp( token, "rotated" ) )
				{
					pfile                  = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.rotated = atoi( token );
				}
				else if ( !stricmp( token, "inset" ) )
				{
					pfile                            = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowX      = atof( token );
					pfile                            = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowY      = atof( token );
					pfile                            = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowWidth  = atof( token );
					pfile                            = gEngfuncs.COM_ParseFile( pfile, token );
					m_OverviewData.insetWindowHeight = atof( token );
				}
				else
				{
					gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename, token );
					return false;
				}

				pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
			}
		}
		else if ( !stricmp( token, "layer" ) )
		{
			// parse a layer data

			if ( m_OverviewData.layers == OVERVIEW_MAX_LAYERS )
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. ( too many layers )\n", filename );
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			if ( stricmp( token, "{" ) )
			{
				gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename );
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );

			while ( stricmp( token, "}" ) )
			{
				if ( !stricmp( token, "image" ) )
				{
					pfile = gEngfuncs.COM_ParseFile( pfile, token );
					strcpy( m_OverviewData.layersImages[m_OverviewData.layers], token );
				}
				else if ( !stricmp( token, "height" ) )
				{
					pfile                                               = gEngfuncs.COM_ParseFile( pfile, token );
					height                                              = atof( token );
					m_OverviewData.layersHeights[m_OverviewData.layers] = height;
				}
				else
				{
					gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename, token );
					return false;
				}

				pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
			}

			m_OverviewData.layers++;
		}
	}

	gEngfuncs.COM_FreeFile( pfile );

	m_mapZoom   = m_OverviewData.zoom;
	m_mapOrigin = m_OverviewData.origin;

	return true;
}

void CHudSpectator::LoadMapSprites()
{
	if ( m_OverviewData.layers > 0 )
	{
		m_MapSprite = gEngfuncs.LoadMapSprite( m_OverviewData.layersImages[0] );
	}
	else
		m_MapSprite = NULL;
}

void CHudSpectator::DrawOverviewLayer()
{
	float screenaspect, xs, ys, xStep, yStep, x, y, z;
	int ix, iy, i, xTiles, yTiles, frame;

	qboolean maybeCanDoTriAPI = true;

	if ( !m_MapSprite )
		return;

	z = m_OverviewData.layersHeights[0];

	i = m_MapSprite->numframes / ( 4 * 3 );

	i = sqrt( (double)i );

	xTiles = i * 4;
	yTiles = i * 3;

	screenaspect = 4.0f / 3.0f;

	xs = m_OverviewData.origin[0];
	ys = m_OverviewData.origin[1];

	xStep = ( 4096.0f / m_OverviewData.zoom ) / xTiles;
	yStep = -( ( 4096.0f / m_OverviewData.zoom ) / screenaspect ) / yTiles;

	xs -= ( ( 4096.0f / m_OverviewData.zoom ) / 2 );
	ys += ( ( ( 4096.0f / m_OverviewData.zoom ) / screenaspect ) / 2 );

	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );

	if ( m_OverviewData.rotated )
	{
		for ( ix = 0; ix < yTiles; ix++ )
		{
			x = xs + ( ix * ( 4096.0f / m_OverviewData.zoom ) / (float)yTiles );

			for ( iy = 0; iy < xTiles; iy++ )
			{
				y = ys + ( ( iy + 1 ) * ( -( 4096.0f / m_OverviewData.zoom ) / screenaspect ) / (float)xTiles );

				frame = iy * yTiles + ( ( yTiles - 1 ) - ix );

				gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );

				gEngfuncs.pTriAPI->Begin( TRI_QUADS );
				gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
				gEngfuncs.pTriAPI->Vertex3f( x, y, z );

				gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
				gEngfuncs.pTriAPI->Vertex3f( x, y - yStep, z );

				gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
				gEngfuncs.pTriAPI->Vertex3f( x + xStep, y - yStep, z );

				gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
				gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z );
				gEngfuncs.pTriAPI->End();
			}
		}
	}
	else
	{
		for ( iy = 0; iy < yTiles; iy++ )
		{
			y = ys + ( iy * yStep );

			for ( ix = 0; ix < xTiles; ix++ )
			{
				x = xs + ( ix * xStep );

				frame = iy * xTiles + ix;

				gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );

				gEngfuncs.pTriAPI->Begin( TRI_QUADS );
				gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
				gEngfuncs.pTriAPI->Vertex3f( x, y, z );

				gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
				gEngfuncs.pTriAPI->Vertex3f( x, y + yStep, z );

				gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
				gEngfuncs.pTriAPI->Vertex3f( x + xStep, y + yStep, z );

				gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
				gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z );
				gEngfuncs.pTriAPI->End();
			}
		}
	}
}

void CHudSpectator::DrawOverviewEntities()
{
	int i, ir, ig, ib;
	struct model_s *hSpriteModel;
	vec3_t origin, angles, point, forward, right, up, left, offset, screen;
	float x, y, z, r, g, b, sizeScale = 1.0;
	cl_entity_t *ent;
	float rmatrix[3][4]; // rotation matrix

	float zScale = ( 90.0f - v_angles[0] ) / 90.0f;

	z = m_OverviewData.layersHeights[0] * zScale;
	UnpackRGB( ir, ig, ib, RGB_YELLOWISH );
	r = (float)ir / 255.0f;
	g = (float)ig / 255.0f;
	b = (float)ib / 255.0f;

	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	for ( i = 0; i < MAX_PLAYERS; i++ )
		m_vPlayerPos[i][2] = -1;

	for ( i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if ( !m_OverviewEntities[i].hSprite )
			continue;

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_OverviewEntities[i].hSprite );
		ent          = m_OverviewEntities[i].entity;

		gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );
		gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );

		AngleVectors( ent->angles, right, up, NULL );

		VectorCopy( ent->origin, origin );

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );

		gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
		VectorMA( origin, 16.0f * sizeScale, up, point );
		VectorMA( point, 16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

		VectorMA( origin, 16.0f * sizeScale, up, point );
		VectorMA( point, -16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
		VectorMA( origin, -16.0f * sizeScale, up, point );
		VectorMA( point, -16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
		VectorMA( origin, -16.0f * sizeScale, up, point );
		VectorMA( point, 16.0f * sizeScale, right, point );
		point[2] *= zScale;
		gEngfuncs.pTriAPI->Vertex3fv( point );

		gEngfuncs.pTriAPI->End();

		if ( !ent->player )
			continue;

		origin[2] *= zScale;

		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );

		hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_hsprBeam );
		gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );

		gEngfuncs.pTriAPI->Color4f( r, g, b, 0.3 );

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] + 4, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] - 4, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] - 4, z );
		gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] + 4, z );
		gEngfuncs.pTriAPI->End();

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] + 4, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] - 4, origin[2] - zScale );
		gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] - 4, z );
		gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
		gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] + 4, z );
		gEngfuncs.pTriAPI->End();

		if ( gEngfuncs.pTriAPI->WorldToScreen( origin, screen ) )
			continue;

		screen[0] = XPROJECT( screen[0] );
		screen[1] = YPROJECT( screen[1] );
		screen[2] = 0.0f;

		origin[0] += 32.0f;
		origin[1] += 32.0f;

		gEngfuncs.pTriAPI->WorldToScreen( origin, offset );

		offset[0] = XPROJECT( offset[0] );
		offset[1] = YPROJECT( offset[1] );
		offset[2] = 0.0f;

		VectorSubtract( offset, screen, offset );

		int playerNum = ent->index - 1;

		m_vPlayerPos[playerNum][0] = screen[0];
		m_vPlayerPos[playerNum][1] = screen[1] + Length( offset );
		m_vPlayerPos[playerNum][2] = 1;
	}

	if ( !m_pip->value || !m_drawcone->value )
		return;

	if ( m_pip->value == INSET_IN_EYE || g_iUser1 == OBS_IN_EYE )
	{
		V_GetInEyePos( g_iUser2, origin, angles );
	}
	else if ( m_pip->value == INSET_CHASE_FREE || g_iUser1 == OBS_CHASE_FREE )
	{
		V_GetChasePos( g_iUser2, v_cl_angles, origin, angles );
	}
	else if ( g_iUser1 == OBS_ROAMING )
	{
		VectorCopy( v_sim_org, origin );
		VectorCopy( v_cl_angles, angles );
	}
	else
		V_GetChasePos( g_iUser2, NULL, origin, angles );

	x = origin[0];
	y = origin[1];
	z = origin[2];

	angles[0] = 0;

	hSpriteModel = (struct model_s *)gEngfuncs.GetSpritePointer( m_hsprCamera );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );

	gEngfuncs.pTriAPI->Color4f( r, g, b, 1.0 );

	AngleVectors( angles, forward, NULL, NULL );
	VectorScale( forward, 512.0f, forward );

	offset[0] = 0.0f;
	offset[1] = 45.0f;
	offset[2] = 0.0f;

	AngleMatrix( offset, rmatrix );
	VectorTransform( forward, rmatrix, right );

	offset[1] = -45.0f;
	AngleMatrix( offset, rmatrix );
	VectorTransform( forward, rmatrix, left );

	gEngfuncs.pTriAPI->Begin( TRI_TRIANGLES );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( x + right[0], y + right[1], ( z + right[2] ) * zScale );

	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( x, y, z * zScale );

	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( x + left[0], y + left[1], ( z + left[2] ) * zScale );
	gEngfuncs.pTriAPI->End();
}

void CHudSpectator::DrawOverview()
{
	if ( !g_iUser1 )
		return;

	if ( m_iDrawCycle == 0 && ( ( g_iUser1 != OBS_MAP_FREE ) && ( g_iUser1 != OBS_MAP_CHASE ) ) )
		return;

	if ( m_iDrawCycle == 1 && m_pip->value < INSET_MAP_FREE )
		return;

	DrawOverviewLayer();
	DrawOverviewEntities();
	CheckOverviewEntities();
}

void CHudSpectator::CheckOverviewEntities()
{
	double time = gEngfuncs.GetClientTime();

	for ( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if ( m_OverviewEntities[i].killTime < time )
		{
			memset( &m_OverviewEntities[i], 0, sizeof( overviewEntity_t ) );
		}
	}
}

bool CHudSpectator::AddOverviewEntity( int type, struct cl_entity_s *ent, const char *modelname )
{
	HSPRITE hSprite = 0;
	double duration = -1.0f;

	if ( !ent )
		return false;

	if ( type == ET_PLAYER )
	{
		if ( ent->curstate.solid != SOLID_NOT )
		{
			switch ( g_PlayerExtraInfo[ent->index].teamnumber )
			{
			case 1:
				hSprite = m_hsprPlayerBlue;
				break;
			case 2:
				hSprite = m_hsprPlayerRed;
				break;
			default:
				hSprite = m_hsprPlayer;
				break;
			}
		}
		else
			return false;
	}
	else if ( type == ET_NORMAL )
	{
		return false;
	}
	else
		return false;

	return AddOverviewEntityToList( hSprite, ent, gEngfuncs.GetClientTime() + duration );
}

void CHudSpectator::DeathMessage( int victim )
{
	cl_entity_t *pl = gEngfuncs.GetEntityByIndex( victim );

	if ( pl && pl->player )
		AddOverviewEntityToList( m_hsprPlayerDead, pl, gEngfuncs.GetClientTime() + 2.0f );
}

bool CHudSpectator::AddOverviewEntityToList( HSPRITE sprite, cl_entity_t *ent, double killTime )
{
	for ( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
	{
		if ( m_OverviewEntities[i].entity == NULL )
		{
			m_OverviewEntities[i].entity   = ent;
			m_OverviewEntities[i].hSprite  = sprite;
			m_OverviewEntities[i].killTime = killTime;
			return true;
		}
	}

	return false;
}
