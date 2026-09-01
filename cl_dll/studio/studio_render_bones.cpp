//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Skeletal bone transformations, quaternion calculations, and motion blending.
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

#ifndef Q_PI
#define Q_PI 3.14159265358979323846f
#endif

#ifndef EF_ROTATE
#define EF_ROTATE ( 1 << 3 )
#endif

extern engine_studio_api_t IEngineStudio;
extern int m_nPlayerGaitSequences[MAX_CLIENTS];

/*
====================
StudioCalcBoneAdj

====================
*/
void CStudioModelRenderer::StudioCalcBoneAdj( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen )
{
	int i, j;
	float value;
	mstudiobonecontroller_t *pbonecontroller;

	pbonecontroller = (mstudiobonecontroller_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->bonecontrollerindex );

	for ( j = 0; j < m_pStudioHeader->numbonecontrollers; j++ )
	{
		i = pbonecontroller[j].index;
		if ( i <= 3 )
		{
			// check for 360 wrap?
			if ( pbonecontroller[j].type & STUDIO_RLOOP )
			{
				if ( abs( pcontroller1[i] - pcontroller2[i] ) > 128 )
				{
					int a, b;
					a = ( pcontroller1[i] + 128 ) % 256;
					b = ( pcontroller2[i] + 128 ) % 256;
					value = ( ( a * ( 1.0 - dadt ) + b * dadt ) - 128 ) * ( 360.0 / 256.0 ) + pbonecontroller[j].start;
				}
				else
				{
					value = ( pcontroller1[i] * ( 1.0 - dadt ) + pcontroller2[i] * dadt ) * ( 360.0 / 256.0 ) + pbonecontroller[j].start;
				}
			}
			else
			{
				value = ( pcontroller1[i] * ( 1.0 - dadt ) + pcontroller2[i] * dadt ) / 255.0;
				if ( value < 0 )
					value = 0;
				if ( value > 1.0 )
					value = 1.0;
				value = ( 1.0 - value ) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}
		}
		else
		{
			value = mouthopen / 64.0;
			if ( value > 1.0 )
				value = 1.0;
			value = ( 1.0 - value ) * pbonecontroller[j].start + value * pbonecontroller[j].end;
		}
		switch ( pbonecontroller[j].type & STUDIO_TYPES )
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			adj[j] = value * ( Q_PI / 180.0 );
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			adj[j] = value;
			break;
		}
	}
}

/*
====================
StudioCalcBoneQuaterion

====================
*/
void CStudioModelRenderer::StudioCalcBoneQuaterion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q )
{
	int j, k;
	vec4_t q1, q2;
	vec3_t angle1, angle2;
	mstudioanimvalue_t *panimvalue;

	for ( j = 0; j < 3; j++ )
	{
		if ( panim->offset[j + 3] == 0 )
		{
			angle2[j] = angle1[j] = pbone->value[j + 3]; // default value
		}
		else
		{
			panimvalue = (mstudioanimvalue_t *)( (byte *)panim + panim->offset[j + 3] );
			k          = frame;
			// DEBUG
			if ( panimvalue->num.total < panimvalue->num.valid )
				k = 0;
			while ( panimvalue->num.total <= k )
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				// DEBUG
				if ( panimvalue->num.total < panimvalue->num.valid )
					k = 0;
			}
			// Bah, just use last frame
			if ( panimvalue->num.valid > k )
			{
				angle1[j] = panimvalue[k + 1].value;

				if ( panimvalue->num.valid > k + 1 )
				{
					angle2[j] = panimvalue[k + 2].value;
				}
				else
				{
					if ( panimvalue->num.total > k + 1 )
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if ( panimvalue->num.total > k + 1 )
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j + 3] + angle1[j] * pbone->scale[j + 3];
			angle2[j] = pbone->value[j + 3] + angle2[j] * pbone->scale[j + 3];
		}

		if ( pbone->bonecontroller[j + 3] != -1 )
		{
			angle1[j] += adj[pbone->bonecontroller[j + 3]];
			angle2[j] += adj[pbone->bonecontroller[j + 3]];
		}
	}

	if ( !VectorCompare( angle1, angle2 ) )
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionSlerp( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}
}

/*
====================
StudioCalcBonePosition

====================
*/
void CStudioModelRenderer::StudioCalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos )
{
	int j, k;
	mstudioanimvalue_t *panimvalue;

	for ( j = 0; j < 3; j++ )
	{
		pos[j] = pbone->value[j]; // default value
		if ( panim->offset[j] != 0 )
		{
			panimvalue = (mstudioanimvalue_t *)( (byte *)panim + panim->offset[j] );

			k = frame;
			// DEBUG
			if ( panimvalue->num.total < panimvalue->num.valid )
				k = 0;
			// find span of frames that includes, kissed, pays at least some attention to the current frame number
			while ( panimvalue->num.total <= k )
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				// DEBUG
				if ( panimvalue->num.total < panimvalue->num.valid )
					k = 0;
			}
			// if we're inside the span
			if ( panimvalue->num.valid > k )
			{
				// short to float conversion
				pos[j] += panimvalue[k + 1].value * pbone->scale[j];

				if ( panimvalue->num.valid > k + 1 )
				{
					pos[j] += ( panimvalue[k + 2].value - panimvalue[k + 1].value ) * s * pbone->scale[j];
				}
				else
				{
					if ( panimvalue->num.total > k + 1 )
					{
						// In-between span
					}
					else
					{
						// Advance to next span
						pos[j] += ( panimvalue[panimvalue->num.valid + 2].value - panimvalue[k + 1].value ) * s * pbone->scale[j];
					}
				}
			}
			else
			{
				// In-between span
				pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				if ( panimvalue->num.total > k + 1 )
				{
					// In-between span
				}
				else
				{
					// Advance to next span
					pos[j] += ( panimvalue[panimvalue->num.valid + 2].value - panimvalue[panimvalue->num.valid].value ) * s * pbone->scale[j];
				}
			}
		}
		if ( pbone->bonecontroller[j] != -1 )
		{
			pos[j] += adj[pbone->bonecontroller[j]];
		}
	}
}

/*
====================
StudioSlerpBones

====================
*/
void CStudioModelRenderer::StudioSlerpBones( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s )
{
	int i;
	vec4_t q3;
	float c;

	if ( s < 0 )
		s = 0;
	else if ( s > 1.0 )
		s = 1.0;

	c = 1.0 - s;

	for ( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		QuaternionSlerp( q1[i], q2[i], s, q3 );
		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];

		pos1[i][0] = pos1[i][0] * c + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * c + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * c + pos2[i][2] * s;
	}
}

/*
====================
StudioGetAnim

====================
*/
mstudioanim_t *CStudioModelRenderer::StudioGetAnim( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc )
{
	mstudioseqgroup_t *pseqgroup;
	cache_user_t *paSequences;

	pseqgroup = (mstudioseqgroup_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqgroupindex ) + pseqdesc->seqgroup;

	if ( pseqdesc->seqgroup == 0 )
	{
		return (mstudioanim_t *)( (byte *)m_pStudioHeader + pseqdesc->animindex );
	}

	paSequences = (cache_user_t *)m_pSubModel->submodels;

	if ( paSequences == NULL )
	{
		paSequences            = (cache_user_t *)IEngineStudio.Mem_Calloc( 16, sizeof( cache_user_t ) ); // UNDONE: leak!
		m_pSubModel->submodels = (dmodel_t *)paSequences;
	}

	if ( !IEngineStudio.Cache_Check( (struct cache_user_s *)&( paSequences[pseqdesc->seqgroup] ) ) )
	{
		gEngfuncs.Con_DPrintf( "loading %s\n", pseqgroup->name );
		IEngineStudio.LoadCacheFile( pseqgroup->name, (struct cache_user_s *)&paSequences[pseqdesc->seqgroup] );
	}
	return (mstudioanim_t *)paSequences[pseqdesc->seqgroup].data;
}

/*
====================
StudioPlayerBlend

====================
*/
void CStudioModelRenderer::StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch )
{
	// calc blending items
	*pBlend = ( *pPitch * 3 );
	if ( *pBlend < pseqdesc->blendstart[0] )
	{
		*pPitch = ( *pPitch - pseqdesc->blendstart[0] / 3.0 );
		*pBlend = 0;
	}
	else if ( *pBlend > pseqdesc->blendend[0] )
	{
		*pPitch = ( *pPitch - pseqdesc->blendend[0] / 3.0 );
		*pBlend = 255;
	}
	else
	{
		if ( pseqdesc->blendend[0] - pseqdesc->blendstart[0] < 0.1 )
			*pBlend = 0;
		else
			*pBlend = (int)( ( *pBlend - pseqdesc->blendstart[0] ) * 255.0 / ( pseqdesc->blendend[0] - pseqdesc->blendstart[0] ) );
		*pPitch = 0;
	}
}

/*
====================
StudioSetUpTransform

====================
*/
void CStudioModelRenderer::StudioSetUpTransform( int trivial_accept )
{
	int i;
	vec3_t angles;
	vec3_t origin;

	// VectorCopy( m_pCurrentEntity->origin, origin );
	// VectorCopy( m_pCurrentEntity->angles, angles );

	// TODO: should really be stored in the entity
	//  interpolate origin and angles
	if ( !trivial_accept && m_fDoInterp )
	{
		for ( i = 0; i < 3; i++ )
		{
			origin[i] = m_pCurrentEntity->curstate.origin[i] * ( 1.0 - m_clTime ) + m_pCurrentEntity->latched.prevorigin[i] * m_clTime;
			angles[i] = m_pCurrentEntity->curstate.angles[i] * ( 1.0 - m_clTime ) + m_pCurrentEntity->latched.prevangles[i] * m_clTime;
		}
	}
	else
	{
		VectorCopy( m_pCurrentEntity->curstate.origin, origin );
		VectorCopy( m_pCurrentEntity->curstate.angles, angles );
	}

	// Con_DPrintf( "%f %f\n", origin[0], m_pCurrentEntity->curstate.origin[0] );

	// Auto-rotate the TFC flag
	if ( m_pCurrentEntity->curstate.effects & EF_ROTATE )
	{
		angles[1] = fmod( 100 * (float)m_clTime, 360.0 );
	}

	if ( m_pCurrentEntity->curstate.effects & EF_BRIGHTFIELD )
	{
		angles[1] = 0;
	}

	angles[0] = -angles[0];
	angles[2] = -angles[2];

	// gEngfuncs.Con_DPrintf( "angles %f %f %f\n", angles[0], angles[1], angles[2] );

	AngleMatrix( angles, ( *m_protationmatrix ) );

	( *m_protationmatrix )[0][3] = origin[0];
	( *m_protationmatrix )[1][3] = origin[1];
	( *m_protationmatrix )[2][3] = origin[2];

	// m_pCurrentEntity->curstate.scale = 0.5;

	// Scale the model, if requested
	if ( m_pCurrentEntity->curstate.scale > 0.001 )
	{
		int j;
		for ( i = 0; i < 3; i++ )
		{
			for ( j = 0; j < 3; j++ )
			{
				( *m_protationmatrix )[i][j] *= m_pCurrentEntity->curstate.scale;
			}
		}
	}

	if ( m_pCurrentEntity->curstate.rendermode == kRenderTransAdd )
	{
	}
	else if ( m_pCurrentEntity->curstate.rendermode != kRenderNormal )
	{
		// Set custom blend amounts
		//
		//
	}

	if ( !IEngineStudio.IsHardware() )
	{
		static float viewmatrix[3][4];

		VectorCopy( m_vRight, viewmatrix[0] );
		VectorCopy( m_vUp, viewmatrix[1] );
		VectorInverse( viewmatrix[1] );
		VectorCopy( m_vNormal, viewmatrix[2] );

		viewmatrix[0][3] = -DotProduct( m_vRenderOrigin, viewmatrix[0] );
		viewmatrix[1][3] = -DotProduct( m_vRenderOrigin, viewmatrix[1] );
		viewmatrix[2][3] = -DotProduct( m_vRenderOrigin, viewmatrix[2] );

		ConcatTransforms( viewmatrix, ( *m_protationmatrix ), ( *m_paliastransform ) );

		// do the scaling up front
		for ( i = 0; i < 3; i++ )
		{
			( *m_paliastransform )[0][i] *= m_fSoftwareXScale * ( 1.0 / ( ZISCALE * 0x10000 ) );
			( *m_paliastransform )[1][i] *= m_fSoftwareYScale * ( 1.0 / ( ZISCALE * 0x10000 ) );
			( *m_paliastransform )[2][i] *= ( 1.0 / ( ZISCALE * 0x10000 ) );
		}
		( *m_paliastransform )[0][3] *= m_fSoftwareXScale * ( 1.0 / ( ZISCALE * 0x10000 ) );
		( *m_paliastransform )[0][3] += ( (float)ScreenWidth / 2.0 ) * ( 1.0 / ( ZISCALE * 0x10000 ) );
		( *m_paliastransform )[1][3] *= m_fSoftwareYScale * ( 1.0 / ( ZISCALE * 0x10000 ) );
		( *m_paliastransform )[1][3] += ( (float)ScreenHeight / 2.0 ) * ( 1.0 / ( ZISCALE * 0x10000 ) );
		( *m_paliastransform )[2][3] *= ( 1.0 / ( ZISCALE * 0x10000 ) );
	}
}

/*
====================
StudioEstimateInterpolant

====================
*/
float CStudioModelRenderer::StudioEstimateInterpolant( void )
{
	float dadt = 1.0;

	if ( m_fDoInterp && ( m_pCurrentEntity->curstate.animtime >= m_pCurrentEntity->latched.prevanimtime + 0.01 ) )
	{
		dadt = ( m_clTime - m_pCurrentEntity->curstate.animtime ) / 0.1;
		if ( dadt > 2.0 )
		{
			dadt = 2.0;
		}
	}
	return dadt;
}

/*
====================
StudioCalcRotations

====================
*/
void CStudioModelRenderer::StudioCalcRotations( float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f )
{
	int i;
	int frame;
	mstudiobone_t *pbone;

	float s;
	float adj[MAXSTUDIOCONTROLLERS];
	float dadt;

	if ( f > pseqdesc->numframes - 1 )
	{
		f = 0; // wrap to first frame
	}
	else if ( f < -0.01 )
	{
		f = -0.01;
	}

	frame = (int)f;

	// Con_DPrintf("%d %.4f %.4f\n", frame, f, s );

	dadt = StudioEstimateInterpolant();
	// dadt = 1.0;
	s = ( f - frame );

	// add in programatic controllers
	pbone = (mstudiobone_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->boneindex );

	StudioCalcBoneAdj( dadt, adj, m_pCurrentEntity->curstate.controller, m_pCurrentEntity->latched.prevcontroller, m_pCurrentEntity->mouth.mouthopen );

	for ( i = 0; i < m_pStudioHeader->numbones; i++, pbone++, panim++ )
	{
		StudioCalcBoneQuaterion( frame, s, pbone, panim, adj, q[i] );

		StudioCalcBonePosition( frame, s, pbone, panim, adj, pos[i] );
		// if (0 && i == 0)
		//	Con_DPrintf("%d %d %d %d\n", m_pCurrentEntity->curstate.sequence, frame, j, k );
	}

	if ( pseqdesc->motiontype & STUDIO_X )
		pos[pseqdesc->motionbone][0] = 0.0;
	if ( pseqdesc->motiontype & STUDIO_Y )
		pos[pseqdesc->motionbone][1] = 0.0;
	if ( pseqdesc->motiontype & STUDIO_Z )
		pos[pseqdesc->motionbone][2] = 0.0;

	s = 0 * ( ( 1.0 - ( f - (int)f ) ) );

	if ( pseqdesc->motiontype & STUDIO_LX )
		pos[pseqdesc->motionbone][0] += s * pseqdesc->linearmovement[0];
	if ( pseqdesc->motiontype & STUDIO_LY )
		pos[pseqdesc->motionbone][1] += s * pseqdesc->linearmovement[1];
	if ( pseqdesc->motiontype & STUDIO_LZ )
		pos[pseqdesc->motionbone][2] += s * pseqdesc->linearmovement[2];
}

/*
====================
StudioFxTransform

====================
*/
void CStudioModelRenderer::StudioFxTransform( cl_entity_t *ent, float transform[3][4] )
{
	switch ( ent->curstate.renderfx )
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
		if ( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			int axis = gEngfuncs.pfnRandomLong( 0, 1 );
			if ( axis == 1 ) // Choose between axis
				axis = 2;
			VectorScale( transform[axis], gEngfuncs.pfnRandomFloat( 1, 1.484 ), transform[axis] );
		}
		else if ( gEngfuncs.pfnRandomLong( 0, 49 ) == 0 )
		{
			float offset;
			int axis = gEngfuncs.pfnRandomLong( 0, 1 );
			if ( axis == 1 ) // Choose between axis
				axis = 2;
			offset               = gEngfuncs.pfnRandomFloat( -10, 10 );
			transform[axis][3] += offset;
		}
		break;
	case kRenderFxExplode:
	{
		float flTimeDelta = m_clTime - ent->curstate.animtime;
		if ( flTimeDelta > 0.0 )
		{
			float flScale = 1.0 + flTimeDelta * 2.0;
			// Scale the model
			int i, j;
			for ( i = 0; i < 3; i++ )
			{
				for ( j = 0; j < 3; j++ )
				{
					transform[i][j] *= flScale;
				}
			}
		}
	}
	break;
	default:
		break;
	}
}

/*
====================
StudioEstimateFrame

====================
*/
float CStudioModelRenderer::StudioEstimateFrame( mstudioseqdesc_t *pseqdesc )
{
	double dfdt, f;

	if ( m_fDoInterp )
	{
		if ( m_clTime < m_pCurrentEntity->curstate.animtime )
		{
			dfdt = 0;
		}
		else
		{
			dfdt = ( m_clTime - m_pCurrentEntity->curstate.animtime ) * m_pCurrentEntity->curstate.framerate * pseqdesc->fps;
		}
	}
	else
	{
		dfdt = 0;
	}

	if ( pseqdesc->numframes <= 1 )
	{
		f = 0;
	}
	else
	{
		f = ( m_pCurrentEntity->curstate.frame * ( pseqdesc->numframes - 1 ) ) / 256.0;
	}

	f += dfdt;

	if ( pseqdesc->flags & STUDIO_LOOPING )
	{
		if ( pseqdesc->numframes > 1 )
		{
			f -= (int)( f / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
		}
		if ( f < 0 )
		{
			f += ( pseqdesc->numframes - 1 );
		}
	}
	else
	{
		if ( f >= pseqdesc->numframes - 1.001 )
		{
			f = pseqdesc->numframes - 1.001;
		}
		if ( f < 0.0 )
		{
			f = 0.0;
		}
	}
	return f;
}

/*
====================
StudioSetupBones

====================
*/
void CStudioModelRenderer::StudioSetupBones( void )
{
	int i;
	double f;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	static vec4_t q[MAXSTUDIOBONES];
	float bonematrix[3][4];

	static float pos2[MAXSTUDIOBONES][3];
	static vec4_t q2[MAXSTUDIOBONES];
	static float pos3[MAXSTUDIOBONES][3];
	static vec4_t q3[MAXSTUDIOBONES];
	static float pos4[MAXSTUDIOBONES][3];
	static vec4_t q4[MAXSTUDIOBONES];

	if ( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq )
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	f     = StudioEstimateFrame( pseqdesc );
	panim = StudioGetAnim( m_pRenderModel, pseqdesc );
	StudioCalcRotations( pos, q, pseqdesc, panim, f );

	if ( pseqdesc->numblends > 1 )
	{
		float s;
		float dadt;

		panim += m_pStudioHeader->numbones;
		StudioCalcRotations( pos2, q2, pseqdesc, panim, f );

		dadt = StudioEstimateInterpolant();
		s    = ( m_pCurrentEntity->curstate.blending[0] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[0] * dadt ) / 255.0;

		StudioSlerpBones( q, pos, q2, pos2, s );

		if ( pseqdesc->numblends == 4 )
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos3, q3, pseqdesc, panim, f );

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos4, q4, pseqdesc, panim, f );

			s = ( m_pCurrentEntity->curstate.blending[0] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[0] * dadt ) / 255.0;
			StudioSlerpBones( q3, pos3, q4, pos4, s );

			s = ( m_pCurrentEntity->curstate.blending[1] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[1] * dadt ) / 255.0;
			StudioSlerpBones( q, pos, q3, pos3, s );
		}
	}

	if ( m_fDoInterp &&
	     m_pCurrentEntity->latched.sequencetime &&
	     ( m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime ) &&
	     ( m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq ) )
	{
		// blend from last sequence
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s;

		pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->latched.prevsequence;
		panim    = StudioGetAnim( m_pRenderModel, pseqdesc );
		// clip from last sequence
		StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

		if ( pseqdesc->numblends > 1 )
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

			s = ( m_pCurrentEntity->latched.prevseqblending[0] ) / 255.0;
			StudioSlerpBones( q1b, pos1b, q2, pos2, s );

			if ( pseqdesc->numblends == 4 )
			{
				panim += m_pStudioHeader->numbones;
				StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				panim += m_pStudioHeader->numbones;
				StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				s = ( m_pCurrentEntity->latched.prevseqblending[0] ) / 255.0;
				StudioSlerpBones( q3, pos3, q4, pos4, s );

				s = ( m_pCurrentEntity->latched.prevseqblending[1] ) / 255.0;
				StudioSlerpBones( q1b, pos1b, q3, pos3, s );
			}
		}

		s = 1.0 - ( m_clTime - m_pCurrentEntity->latched.sequencetime ) / 0.2;
		StudioSlerpBones( q, pos, q1b, pos1b, s );
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->boneindex );

	//	if (m_pPlayerInfo)
	//	{
	//		StudioPlayerGait( m_pPlayerInfo );
	//	}

	for ( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		QuaternionMatrix( q[i], bonematrix );

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if ( pbones[i].parent == -1 )
		{
			if ( IEngineStudio.IsHardware() )
			{
				ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_pbonetransform )[i] );
				ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_plighttransform )[i] );
			}
			else
			{
				ConcatTransforms( ( *m_paliastransform ), bonematrix, ( *m_pbonetransform )[i] );
				ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_plighttransform )[i] );
			}

			// Apply client-side effects to the transformation matrix
			StudioFxTransform( m_pCurrentEntity, ( *m_pbonetransform )[i] );
		}
		else
		{
			ConcatTransforms( ( *m_pbonetransform )[pbones[i].parent], bonematrix, ( *m_pbonetransform )[i] );
			ConcatTransforms( ( *m_plighttransform )[pbones[i].parent], bonematrix, ( *m_plighttransform )[i] );
		}
	}
}

/*
====================
StudioSaveBones

====================
*/
void CStudioModelRenderer::StudioSaveBones( void )
{
	int i;

	mstudiobone_t *pbones;
	pbones = (mstudiobone_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->boneindex );

	m_nCachedBones = m_pStudioHeader->numbones;

	for ( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		strcpy( m_nCachedBoneNames[i], pbones[i].name );
		MatrixCopy( ( *m_pbonetransform )[i], m_rgCachedBoneTransform[i] );
		MatrixCopy( ( *m_plighttransform )[i], m_rgCachedLightTransform[i] );
	}
}

/*
====================
StudioMergeBones

====================
*/
void CStudioModelRenderer::StudioMergeBones( model_t *m_pSubModel )
{
	int i, j;
	double f;
	int hitch = 0;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static float pos[MAXSTUDIOBONES][3];
	static vec4_t q[MAXSTUDIOBONES];
	float bonematrix[3][4];

	static float pos2[MAXSTUDIOBONES][3];
	static vec4_t q2[MAXSTUDIOBONES];
	static float pos3[MAXSTUDIOBONES][3];
	static vec4_t q3[MAXSTUDIOBONES];
	static float pos4[MAXSTUDIOBONES][3];
	static vec4_t q4[MAXSTUDIOBONES];

	if ( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq )
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->curstate.sequence;

	f     = StudioEstimateFrame( pseqdesc );
	panim = StudioGetAnim( m_pSubModel, pseqdesc );
	StudioCalcRotations( pos, q, pseqdesc, panim, f );

	if ( pseqdesc->numblends > 1 )
	{
		float s;
		float dadt;

		panim += m_pStudioHeader->numbones;
		StudioCalcRotations( pos2, q2, pseqdesc, panim, f );

		dadt = StudioEstimateInterpolant();
		s    = ( m_pCurrentEntity->curstate.blending[0] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[0] * dadt ) / 255.0;

		StudioSlerpBones( q, pos, q2, pos2, s );

		if ( pseqdesc->numblends == 4 )
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos3, q3, pseqdesc, panim, f );

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos4, q4, pseqdesc, panim, f );

			s = ( m_pCurrentEntity->curstate.blending[0] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[0] * dadt ) / 255.0;
			StudioSlerpBones( q3, pos3, q4, pos4, s );

			s = ( m_pCurrentEntity->curstate.blending[1] * ( 1.0 - dadt ) + m_pCurrentEntity->latched.prevblending[1] * dadt ) / 255.0;
			StudioSlerpBones( q, pos, q3, pos3, s );
		}
	}

	if ( m_fDoInterp &&
	     m_pCurrentEntity->latched.sequencetime &&
	     ( m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime ) &&
	     ( m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq ) )
	{
		// blend from last sequence
		static float pos1b[MAXSTUDIOBONES][3];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s;

		pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pCurrentEntity->latched.prevsequence;
		panim    = StudioGetAnim( m_pSubModel, pseqdesc );
		// clip from last sequence
		StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

		if ( pseqdesc->numblends > 1 )
		{
			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

			s = ( m_pCurrentEntity->latched.prevseqblending[0] ) / 255.0;
			StudioSlerpBones( q1b, pos1b, q2, pos2, s );

			if ( pseqdesc->numblends == 4 )
			{
				panim += m_pStudioHeader->numbones;
				StudioCalcRotations( pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				panim += m_pStudioHeader->numbones;
				StudioCalcRotations( pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				s = ( m_pCurrentEntity->latched.prevseqblending[0] ) / 255.0;
				StudioSlerpBones( q3, pos3, q4, pos4, s );

				s = ( m_pCurrentEntity->latched.prevseqblending[1] ) / 255.0;
				StudioSlerpBones( q1b, pos1b, q3, pos3, s );
			}
		}

		s = 1.0 - ( m_clTime - m_pCurrentEntity->latched.sequencetime ) / 0.2;
		StudioSlerpBones( q, pos, q1b, pos1b, s );
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->boneindex );

	for ( i = 0; i < m_pStudioHeader->numbones; i++ )
	{
		for ( j = 0; j < m_nCachedBones; j++ )
		{
			if ( stricmp( pbones[i].name, m_nCachedBoneNames[j] ) == 0 )
			{
				MatrixCopy( m_rgCachedBoneTransform[j], ( *m_pbonetransform )[i] );
				MatrixCopy( m_rgCachedLightTransform[j], ( *m_plighttransform )[i] );
				break;
			}
		}
		if ( j >= m_nCachedBones )
		{
			QuaternionMatrix( q[i], bonematrix );

			bonematrix[0][3] = pos[i][0];
			bonematrix[1][3] = pos[i][1];
			bonematrix[2][3] = pos[i][2];

			if ( pbones[i].parent == -1 )
			{
				if ( IEngineStudio.IsHardware() )
				{
					ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_pbonetransform )[i] );
					ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_plighttransform )[i] );
				}
				else
				{
					ConcatTransforms( ( *m_paliastransform ), bonematrix, ( *m_pbonetransform )[i] );
					ConcatTransforms( ( *m_protationmatrix ), bonematrix, ( *m_plighttransform )[i] );
				}

				// Apply client-side effects to the transformation matrix
				StudioFxTransform( m_pCurrentEntity, ( *m_pbonetransform )[i] );
				hitch = 1;
			}
			else
			{
				ConcatTransforms( ( *m_pbonetransform )[pbones[i].parent], bonematrix, ( *m_pbonetransform )[i] );
				ConcatTransforms( ( *m_plighttransform )[pbones[i].parent], bonematrix, ( *m_plighttransform )[i] );
			}
		}
	}
}

/*
====================
StudioEstimateGait

====================
*/
void CStudioModelRenderer::StudioEstimateGait( entity_state_t *pplayer )
{
	float dt;
	vec3_t est_velocity;

	dt = ( m_clTime - m_clOldTime );
	if ( dt < 0 )
		dt = 0;
	else if ( dt > 1.0 )
		dt = 1;

	if ( dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount )
	{
		m_flGaitMovement = 0;
		return;
	}

	// if (length == 0)
	if ( ( pplayer->sequence >= 23 && pplayer->sequence <= 39 ) ||
	     ( pplayer->sequence >= 50 && pplayer->sequence <= 68 ) )
	{
		m_flGaitMovement = 0;
		// m_pPlayerInfo->gaitsequence = 0;
		return;
	}

	VectorSubtract( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity );
	VectorCopy( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin );

	m_flGaitMovement = Length( est_velocity );
	if ( dt > 0 )
	{
		m_flGaitMovement = m_flGaitMovement / dt;
	}
	else
	{
		m_flGaitMovement = 0;
	}
}

/*
====================
StudioProcessGait

====================
*/
void CStudioModelRenderer::StudioProcessGait( entity_state_t *pplayer )
{
	mstudioseqdesc_t *pseqdesc;
	float dt;

	if ( pplayer->sequence >= m_pStudioHeader->numseq )
	{
		pplayer->sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + pplayer->sequence;

	dt = ( m_clTime - m_clOldTime );
	if ( dt < 0 )
		dt = 0;
	else if ( dt > 1.0 )
		dt = 1;

	StudioEstimateGait( pplayer );

	// Calc gait frame
	if ( pplayer->gaitsequence >= m_pStudioHeader->numseq )
	{
		pplayer->gaitsequence = 0;
	}

	// TFC gait sequence hack
	if ( pplayer->gaitsequence == 0 )
	{
		pplayer->gaitsequence = m_nPlayerGaitSequences[m_nPlayerIndex - 1];
	}

	pseqdesc = (mstudioseqdesc_t *)( (byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + pplayer->gaitsequence;

	// Reset gait frame if player changed sequences
	if ( pplayer->gaitsequence != m_pPlayerInfo->gaitsequence )
	{
		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo->gaitframe    = 0;
	}

	// Set gait frame
	if ( pseqdesc->linearmovement[0] > 0 )
	{
		m_pPlayerInfo->gaitframe += ( m_flGaitMovement * dt ) / pseqdesc->linearmovement[0];
		m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)( m_pPlayerInfo->gaitframe );

		if ( m_pPlayerInfo->gaitframe < 0 )
			m_pPlayerInfo->gaitframe += 1.0;
	}
	else
	{
		m_pPlayerInfo->gaitframe = 0;
	}

	// Clear out interpolated frame
	m_pPlayerInfo->renderframe = m_nFrameCount;
}
