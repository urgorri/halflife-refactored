// studio_model.cpp
// routines for setting up to draw 3DStudio models

#include "hud/hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

extern cvar_t *tfc_newmodels;

extern extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS + 1];

// team colors for old TFC models
#define TEAM1_COLOR 150
#define TEAM2_COLOR 250
#define TEAM3_COLOR 45
#define TEAM4_COLOR 100

int m_nPlayerGaitSequences[MAX_CLIENTS];

// Global engine <-> studio model rendering code interface
engine_studio_api_t IEngineStudio;

/////////////////////
// Implementation of CStudioModelRenderer.h

/*
====================
Init

====================
*/
void CStudioModelRenderer::Init( void )
{
	// Set up some variables shared with engine
	m_pCvarHiModels     = IEngineStudio.GetCvar( "cl_himodels" );
	m_pCvarDeveloper    = IEngineStudio.GetCvar( "developer" );
	m_pCvarDrawEntities = IEngineStudio.GetCvar( "r_drawentities" );

	m_pChromeSprite = IEngineStudio.GetChromeSprite();

	IEngineStudio.GetModelCounters( &m_pStudioModelCount, &m_pModelsDrawn );

	// Get pointers to engine data structures
	m_pbonetransform  = (float( * )[MAXSTUDIOBONES][3][4])IEngineStudio.StudioGetBoneTransform();
	m_plighttransform = (float( * )[MAXSTUDIOBONES][3][4])IEngineStudio.StudioGetLightTransform();
	m_paliastransform = (float( * )[3][4])IEngineStudio.StudioGetAliasTransform();
	m_protationmatrix = (float( * )[3][4])IEngineStudio.StudioGetRotationMatrix();
}

/*
====================
CStudioModelRenderer

====================
*/
CStudioModelRenderer::CStudioModelRenderer( void )
{
	m_fDoInterp         = 1;
	m_fGaitEstimation   = 1;
	m_pCurrentEntity    = NULL;
	m_pCvarHiModels     = NULL;
	m_pCvarDeveloper    = NULL;
	m_pCvarDrawEntities = NULL;
	m_pChromeSprite     = NULL;
	m_pStudioModelCount = NULL;
	m_pModelsDrawn      = NULL;
	m_protationmatrix   = NULL;
	m_paliastransform   = NULL;
	m_pbonetransform    = NULL;
	m_plighttransform   = NULL;
	m_pStudioHeader     = NULL;
	m_pBodyPart         = NULL;
	m_pSubModel         = NULL;
	m_pPlayerInfo       = NULL;
	m_pRenderModel      = NULL;
}

/*
====================
~CStudioModelRenderer

====================
*/
CStudioModelRenderer::~CStudioModelRenderer( void )
{
}

/*
====================
StudioDrawModel

====================
*/
int CStudioModelRenderer::StudioDrawModel( int flags )
{
	alight_t lighting;
	vec3_t dir;

	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();
	IEngineStudio.GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	IEngineStudio.GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	IEngineStudio.GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	if ( m_pCurrentEntity->curstate.renderfx == kRenderFxDeadPlayer )
	{
		entity_state_t deadplayer;

		int player = m_pCurrentEntity->curstate.renderamt;

		if ( player < 1 || player > gEngfuncs.GetMaxClients() )
			return 0;

		// get copy of player
		deadplayer = *( IEngineStudio.GetPlayerState( m_pCurrentEntity->curstate.renderamt - 1 ) ); // cl.frames[cl.parsecount & CL_UPDATE_MASK].playerstate[m_pCurrentEntity->curstate.renderamt-1];

		// get player model
		// m_pRenderModel = IEngineStudio.SetupPlayerModel( player - 1 );

		// get dead player sequence
		deadplayer.sequence = m_pCurrentEntity->curstate.sequence;
		deadplayer.frame    = m_pCurrentEntity->curstate.frame;
		VectorCopy( m_pCurrentEntity->curstate.angles, deadplayer.angles );

		// copy monster angles
		// VectorCopy( m_pCurrentEntity->angles, deadplayer.angles );
		// VectorCopy( m_pCurrentEntity->origin, deadplayer.origin );

		// if (deadplayer.angles[0] > 0)
		//	deadplayer.angles[0] = 0;

		return StudioDrawPlayer( flags, &deadplayer );
	}

	m_pRenderModel = m_pCurrentEntity->model;
	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( m_pRenderModel );
	IEngineStudio.StudioSetHeader( m_pStudioHeader );
	IEngineStudio.SetRenderModel( m_pRenderModel );

	StudioSetUpTransform( 0 );

	if ( flags & STUDIO_RENDER )
	{
		// see if the bounding box lets us trivially reject, also sets
		if ( !IEngineStudio.StudioCheckBBox() )
			return 0;

		( *m_pModelsDrawn )++;
		( *m_pStudioModelCount )++; // The number of times StudioDrawModelImp has been called

		if ( m_pStudioHeader->numbodyparts == 0 )
			return 1;
	}

	if ( m_pCurrentEntity->curstate.movetype == MOVETYPE_FOLLOW )
	{
		StudioMergeBones( m_pRenderModel );
	}
	else
	{
		StudioSetupBones();
	}

	StudioSaveBones();

	if ( flags & STUDIO_EVENTS )
	{
		StudioCalcAttachments();
		IEngineStudio.StudioClientEvents();
		// copy attachments back to viewentity
		// VectorCopy( m_pCurrentEntity->attachment[0], g_vViewEntityAttachment );
		// VectorCopy( m_pCurrentEntity->attachment[0], cl.viewent.attachment[0] );
	}

	if ( flags & STUDIO_RENDER )
	{
		lighting.plightvec = dir;

		IEngineStudio.StudioDynamicLight( m_pCurrentEntity, &lighting );

		IEngineStudio.StudioEntityLight( &lighting );

		// model and frame independant
		IEngineStudio.StudioSetupLighting( &lighting );

		// get remap colors
		m_nTopColor    = m_pCurrentEntity->curstate.colormap & 0xFF;
		m_nBottomColor = ( m_pCurrentEntity->curstate.colormap & 0xFF00 ) >> 8;

		// Fixup for model colors
		//
		if ( ( m_nTopColor == 0 ) && ( m_nBottomColor == 0 ) )
		{
			// Need to do this, otherwise, players will appear with black shirts and pants!
			//
			m_nTopColor    = g_PlayerExtraInfo[m_pCurrentEntity->index].playerclass;
			m_nBottomColor = g_PlayerExtraInfo[m_pCurrentEntity->index].teamnumber;

			if ( m_nBottomColor == 1 )
			{
				m_nBottomColor = TEAM1_COLOR;
			}
			else if ( m_nBottomColor == 2 )
			{
				m_nBottomColor = TEAM2_COLOR;
			}
			else if ( m_nBottomColor == 3 )
			{
				m_nBottomColor = TEAM3_COLOR;
			}
			else if ( m_nBottomColor == 4 )
			{
				m_nBottomColor = TEAM4_COLOR;
			}
			else
			{
				m_nBottomColor = 0;
			}
		}

		IEngineStudio.StudioSetRemapColors( m_nTopColor, m_nBottomColor );

		StudioRenderModel();
	}

	return 1;
}

/*
====================
StudioDrawPlayer

====================
*/
int CStudioModelRenderer::StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	alight_t lighting;
	vec3_t dir;

	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();
	IEngineStudio.GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	IEngineStudio.GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	IEngineStudio.GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	m_nPlayerIndex = pplayer->number - 1;

	if ( m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients() )
		return 0;

	m_pRenderModel = IEngineStudio.SetupPlayerModel( m_nPlayerIndex );
	if ( m_pRenderModel == NULL )
		return 0;

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( m_pRenderModel );
	IEngineStudio.StudioSetHeader( m_pStudioHeader );
	IEngineStudio.SetRenderModel( m_pRenderModel );

	StudioSetUpTransform( 0 );

	if ( flags & STUDIO_RENDER )
	{
		// see if the bounding box lets us trivially reject, also sets
		if ( !IEngineStudio.StudioCheckBBox() )
			return 0;

		( *m_pModelsDrawn )++;
		( *m_pStudioModelCount )++; // The number of times StudioDrawModelImp has been called

		if ( m_pStudioHeader->numbodyparts == 0 )
			return 1;
	}

	m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

	StudioProcessGait( pplayer );

	StudioSetupBones();
	StudioSaveBones();

	if ( flags & STUDIO_EVENTS )
	{
		StudioCalcAttachments();
		IEngineStudio.StudioClientEvents();
		// copy attachments back to viewentity
		// VectorCopy( m_pCurrentEntity->attachment[0], g_vViewEntityAttachment );
		// VectorCopy( m_pCurrentEntity->attachment[0], cl.viewent.attachment[0] );
	}

	if ( flags & STUDIO_RENDER )
	{
		lighting.plightvec = dir;

		IEngineStudio.StudioDynamicLight( m_pCurrentEntity, &lighting );

		IEngineStudio.StudioEntityLight( &lighting );

		// model and frame independant
		IEngineStudio.StudioSetupLighting( &lighting );

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		// get remap colors
		m_nTopColor    = pplayer->colormap & 0xFF;
		m_nBottomColor = ( pplayer->colormap & 0xFF00 ) >> 8;

		// Fixup for model colors
		//
		if ( ( m_nTopColor == 0 ) && ( m_nBottomColor == 0 ) )
		{
			// Need to do this, otherwise, players will appear with black shirts and pants!
			//
			m_nTopColor    = g_PlayerExtraInfo[m_nPlayerIndex + 1].playerclass;
			m_nBottomColor = g_PlayerExtraInfo[m_nPlayerIndex + 1].teamnumber;

			if ( m_nBottomColor == 1 )
			{
				m_nBottomColor = TEAM1_COLOR;
			}
			else if ( m_nBottomColor == 2 )
			{
				m_nBottomColor = TEAM2_COLOR;
			}
			else if ( m_nBottomColor == 3 )
			{
				m_nBottomColor = TEAM3_COLOR;
			}
			else if ( m_nBottomColor == 4 )
			{
				m_nBottomColor = TEAM4_COLOR;
			}
			else
			{
				m_nBottomColor = 0;
			}
		}

		IEngineStudio.StudioSetRemapColors( m_nTopColor, m_nBottomColor );

		StudioRenderModel();

		if ( pplayer->weaponmodel )
		{
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex( pplayer->weaponmodel );

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata( pweaponmodel );
			IEngineStudio.StudioSetHeader( m_pStudioHeader );

			StudioMergeBones( pweaponmodel );

			IEngineStudio.StudioSetupLighting( &lighting );

			StudioRenderModel();

			StudioCalcAttachments();

			*m_pCurrentEntity = saveent;
		}
	}

	return 1;
}

/*
====================
StudioRenderModel

====================
*/
void CStudioModelRenderer::StudioRenderModel( void )
{
	IEngineStudio.SetChromeOrigin();
	IEngineStudio.SetForceFaceFlags( 0 );

	if ( m_pCurrentEntity->curstate.renderfx == kRenderFxGlowShell )
	{
		m_pCurrentEntity->curstate.renderfx = kRenderFxNone;

		StudioRenderFinal();

		if ( !IEngineStudio.IsHardware() )
		{
			gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
		}

		IEngineStudio.SetForceFaceFlags( STUDIO_NF_CHROME );

		gEngfuncs.pTriAPI->SpriteTexture( m_pChromeSprite, 0 );
		m_pCurrentEntity->curstate.renderfx = kRenderFxGlowShell;

		StudioRenderFinal();
		if ( !IEngineStudio.IsHardware() )
		{
			gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
		}
	}
	else
	{
		StudioRenderFinal();
	}
}

/*
====================
StudioRenderFinal_Software

====================
*/
void CStudioModelRenderer::StudioRenderFinal_Software( void )
{
	int i;

	IEngineStudio.SetupRenderer( 0 );

	if ( m_pCvarDrawEntities->value == 2 )
	{
		IEngineStudio.StudioDrawBones();
	}
	else if ( m_pCvarDrawEntities->value == 3 )
	{
		IEngineStudio.StudioDrawHulls();
	}
	else
	{
		for ( i = 0; i < m_pStudioHeader->numbodyparts; i++ )
		{
			IEngineStudio.StudioSetupModel( i, (void **)&m_pBodyPart, (void **)&m_pSubModel );
			IEngineStudio.StudioDrawPoints();
		}
	}

	if ( m_pCvarDrawEntities->value == 4 )
	{
		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
		IEngineStudio.StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	}

	if ( m_pCvarDrawEntities->value == 5 )
	{
		IEngineStudio.StudioDrawAbsBBox();
	}

	IEngineStudio.RestoreRenderer();
}

/*
====================
StudioRenderFinal_Hardware

====================
*/
void CStudioModelRenderer::StudioRenderFinal_Hardware( void )
{
	int i;
	int rendermode;

	rendermode = IEngineStudio.GetForceFaceFlags() ? kRenderTransAdd : m_pCurrentEntity->curstate.rendermode;
	IEngineStudio.SetupRenderer( rendermode );

	if ( m_pCvarDrawEntities->value == 2 )
	{
		IEngineStudio.StudioDrawBones();
	}
	else if ( m_pCvarDrawEntities->value == 3 )
	{
		IEngineStudio.StudioDrawHulls();
	}
	else
	{
		for ( i = 0; i < m_pStudioHeader->numbodyparts; i++ )
		{
			IEngineStudio.StudioSetupModel( i, (void **)&m_pBodyPart, (void **)&m_pSubModel );

			if ( m_fDoInterp )
			{
				// IEngineStudio.StudioInterpolateModel( m_pSubModel, &m_pCurrentEntity->latched.prevbody );
			}

			IEngineStudio.GL_SetRenderMode( rendermode );
			IEngineStudio.StudioDrawPoints();
			IEngineStudio.GL_StudioDrawShadow();
		}
	}

	if ( m_pCvarDrawEntities->value == 4 )
	{
		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
		IEngineStudio.StudioDrawHulls();
		gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	}

	IEngineStudio.RestoreRenderer();
}

/*
====================
StudioRenderFinal

====================
*/
void CStudioModelRenderer::StudioRenderFinal( void )
{
	if ( IEngineStudio.IsHardware() )
	{
		StudioRenderFinal_Hardware();
	}
	else
	{
		StudioRenderFinal_Software();
	}
}
