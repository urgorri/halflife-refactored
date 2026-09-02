//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Submodel attachments and bone attachment point calculation.
//
// $NoKeywords: $
//=============================================================================

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

extern engine_studio_api_t IEngineStudio;

/*
====================
StudioCalcAttachments

====================
*/
void CStudioModelRenderer::StudioCalcAttachments( void )
{
	int i;
	mstudioattachment_t *pattachment;

	if ( m_pStudioHeader->numattachments > 4 )
	{
		gEngfuncs.Con_DPrintf( "Too many attachments on %s\n", m_pCurrentEntity->model->name );
		exit( -1 );
	}

	// calculate attachment points
	pattachment = (mstudioattachment_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->attachmentindex );
	for ( i = 0; i < m_pStudioHeader->numattachments; i++ )
	{
		VectorTransform( pattachment[i].org, ( *m_plighttransform )[pattachment[i].bone], m_pCurrentEntity->attachment[i] );
	}
}
