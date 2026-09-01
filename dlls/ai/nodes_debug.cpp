/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   This source code contains proprietary and confidential information of
 *   Valve LLC and its suppliers.  Access to this code is restricted to
 *   persons who have executed a written SDK license with Valve.  Any access,
 *   use or distribution of this code by or to any unlicensed person is illegal.
 *
 ****/
//=========================================================
// nodes_debug.cpp - AI node graph validation, debugging, and visualization tools.
//=========================================================

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "ai/monsters.h"
#include "ai/nodes.h"
#include "core/animation.h"

//=========================================================
// CGraph - ShowNodeConnections - draws a line from the given node
// to all connected nodes
//=========================================================
void CGraph ::ShowNodeConnections( int iNode )
{
	Vector vecSpot;
	CNode *pNode;
	CNode *pLinkNode;
	int i;

	if ( !m_fGraphPresent || !m_fGraphPointersSet )
	{
		ALERT( at_aiconsole, "Graph not ready!\n" );
		return;
	}

	if ( iNode < 0 )
	{
		ALERT( at_aiconsole, "Can't show connections for node %d\n", iNode );
		return;
	}

	pNode = &m_pNodes[iNode];

	UTIL_ParticleEffect( pNode->m_vecOrigin, g_vecZero, 255, 20 );

	if ( pNode->m_cNumLinks <= 0 )
	{
		ALERT( at_aiconsole, "**No Connections!\n" );
	}

	for ( i = 0; i < pNode->m_cNumLinks; i++ )
	{
		pLinkNode = &Node( NodeLink( iNode, i ).m_iDestNode );
		vecSpot   = pLinkNode->m_vecOrigin;

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SHOWLINE );

		WRITE_COORD( m_pNodes[iNode].m_vecOrigin.x );
		WRITE_COORD( m_pNodes[iNode].m_vecOrigin.y );
		WRITE_COORD( m_pNodes[iNode].m_vecOrigin.z + NODE_HEIGHT );

		WRITE_COORD( vecSpot.x );
		WRITE_COORD( vecSpot.y );
		WRITE_COORD( vecSpot.z + NODE_HEIGHT );
		MESSAGE_END();
	}
}

LINK_ENTITY_TO_CLASS( testhull, CTestHull );

//=========================================================
// CTestHull::Spawn
//=========================================================
void CTestHull ::Spawn( entvars_t *pevMasterNode )
{
	SET_MODEL( ENT( pev ), "models/player.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid     = SOLID_SLIDEBOX;
	pev->movetype  = MOVETYPE_STEP;
	pev->effects   = 0;
	pev->health    = 50;
	pev->yaw_speed = 8;

	if ( WorldGraph.m_fGraphPresent )
	{
		SetThink( &CTestHull::SUB_Remove );
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		SetThink( &CTestHull::DropDelay );
		pev->nextthink = gpGlobals->time + 1;
	}

	pev->rendermode = kRenderTransTexture;
	pev->renderamt  = 0;
}

//=========================================================
// TestHull::DropDelay - spawns TestHull on top of
// the 0th node and drops it to the ground.
//=========================================================
void CTestHull::DropDelay( void )
{
	UTIL_SetOrigin( VARS( pev ), WorldGraph.m_pNodes[0].m_vecOrigin );

	SetThink( &CTestHull::CallBuildNodeGraph );

	pev->nextthink = gpGlobals->time + 1;
}

//=========================================================
// CTestHull - ShowBadNode - makes a bad node fizzle. When
// there's a problem with node graph generation, the test
// hull will be placed up the bad node's location and will generate
// particles
//=========================================================
void CTestHull ::ShowBadNode( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->angles.y = pev->angles.y + 4;

	UTIL_MakeVectors( pev->angles );

	UTIL_ParticleEffect( pev->origin, g_vecZero, 255, 25 );
	UTIL_ParticleEffect( pev->origin + gpGlobals->v_forward * 64, g_vecZero, 255, 25 );
	UTIL_ParticleEffect( pev->origin - gpGlobals->v_forward * 64, g_vecZero, 255, 25 );
	UTIL_ParticleEffect( pev->origin + gpGlobals->v_right * 64, g_vecZero, 255, 25 );
	UTIL_ParticleEffect( pev->origin - gpGlobals->v_right * 64, g_vecZero, 255, 25 );

	pev->nextthink = gpGlobals->time + 0.1;
}

extern BOOL gTouchDisabled;
void CTestHull::CallBuildNodeGraph( void )
{
	gTouchDisabled = TRUE;
	BuildNodeGraph();
	gTouchDisabled = FALSE;
}

//=========================================================
// BuildNodeGraph - think function called by the empty walk
// hull that is spawned by the first node to spawn.
//=========================================================
void CTestHull ::BuildNodeGraph( void )
{
	TraceResult tr;
	FILE *file;

	char szNrpFilename[MAX_PATH];

	CLink *pTempPool;

	CNode *pSrcNode;
	CNode *pDestNode;

	BOOL fSkipRemainingHulls;
	BOOL fPairsValid;

	int i, j, hull;

	int iBadNode;

	int cMaxInitialLinks = 0;
	int cMaxValidLinks   = 0;

	int iPoolIndex = 0;
	int cPoolLinks;

	Vector vecDirToCheckNode;
	Vector vecDirToTestNode;
	Vector vecStepCheckDir;
	Vector vecTraceSpot;
	Vector vecSpot;

	Vector2D vec2DirToCheckNode;
	Vector2D vec2DirToTestNode;
	Vector2D vec2StepCheckDir;
	Vector2D vec2TraceSpot;
	Vector2D vec2Spot;

	float flYaw;
	float flDist;
	int step;

	SetThink( &CTestHull::SUB_Remove );
	pev->nextthink = gpGlobals->time;

	pTempPool = (CLink *)calloc( sizeof( CLink ), ( WorldGraph.m_cNodes * MAX_NODE_INITIAL_LINKS ) );
	if ( !pTempPool )
	{
		ALERT( at_aiconsole, "**Could not malloc TempPool!\n" );
		return;
	}

	strcpy( szNrpFilename, "maps/" );
	strcat( szNrpFilename, STRING( gpGlobals->mapname ) );
	strcat( szNrpFilename, ".nrp" );

	file = fopen( szNrpFilename, "w" );

	if ( !file )
	{
		ALERT( at_aiconsole, "NRP file could not be created\n" );
	}
	else
	{
		fprintf( file, "Node Table for map: %s.bsp\n", STRING( gpGlobals->mapname ) );
		fprintf( file, "%d Nodes recorded\n", WorldGraph.m_cNodes );
		fprintf( file, "----------------------------------------------------------------------------\n\n" );
	}

	ALERT( at_aiconsole, "Graph: %s.bsp ( %d nodes )\n", STRING( gpGlobals->mapname ), WorldGraph.m_cNodes );

	// nudge all the nodes down so that they are closer to the ground
	for ( i = 0; i < WorldGraph.m_cNodes; i++ )
	{
		if ( WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_LAND )
		{
			UTIL_SetOrigin( VARS( pev ), WorldGraph.m_pNodes[i].m_vecOrigin );

			DROP_TO_FLOOR( ENT( pev ) );

			WorldGraph.m_pNodes[i].m_vecOrigin = pev->origin;

			WorldGraph.m_pNodes[i].m_vecOriginPeek = WorldGraph.m_pNodes[i].m_vecOrigin;
			WorldGraph.m_pNodes[i].m_vecOrigin.z += NODE_HEIGHT;
			WorldGraph.m_pNodes[i].m_vecOriginPeek.z += ( NODE_HEIGHT * 2 );
		}
	}

	cPoolLinks = WorldGraph.LinkVisibleNodes( pTempPool, file, &iBadNode );

	if ( !cPoolLinks )
	{
		ALERT( at_aiconsole, "LinkVisibleNodes Failed!\n" );

		if ( file )
		{
			fprintf( file, "\n**LinkVisibleNodes Failed on Node %d!**\n", iBadNode );
			fclose( file );
		}

		if ( iBadNode >= 0 && iBadNode < WorldGraph.m_cNodes )
		{
			UTIL_SetOrigin( VARS( pev ), WorldGraph.m_pNodes[iBadNode].m_vecOrigin );
			vecBadNodeOrigin = pev->origin;

			SetThink( &CTestHull::ShowBadNode );
			pev->nextthink = gpGlobals->time + 0.1;
		}

		free( pTempPool );
		return;
	}

	WorldGraph.RejectInlineLinks( pTempPool, file );

	if ( file )
	{
		fprintf( file, "----------------------------------------------------------------------------\n" );
		fprintf( file, "Testing Hulls:\n" );
		fprintf( file, "----------------------------------------------------------------------------\n" );
	}

	// now, for each node, walk human hull to each of it's connections and record which hulls can fit through
	for ( i = 0; i < WorldGraph.m_cNodes; i++ )
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		if ( file )
		{
			fprintf( file, "Node %4d:\n", i );
		}

		if ( pSrcNode->m_cNumLinks <= 0 )
		{
			if ( file )
			{
				fprintf( file, "**No Links**\n" );
			}
			continue;
		}

		for ( j = 0; j < pSrcNode->m_cNumLinks; j++ )
		{
			pDestNode = &WorldGraph.m_pNodes[pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

			fSkipRemainingHulls = FALSE;

			for ( hull = 0; hull < MAX_NODE_HULLS; hull++ )
			{
				if ( pSrcNode->m_afNodeInfo & bits_NODE_AIR )
				{
					pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo |= bits_LINK_FLY_HULL;
					continue;
				}

				if ( fSkipRemainingHulls )
				{
					continue;
				}

				switch ( hull )
				{
				case NODE_SMALL_HULL:
					UTIL_SetSize( pev, Vector( -12, -12, 0 ), Vector( 12, 12, 24 ) );
					break;
				case NODE_HUMAN_HULL:
					UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
					break;
				case NODE_LARGE_HULL:
					UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 64 ) );
					break;
				case NODE_FLY_HULL:
					break;
				}

				UTIL_SetOrigin( VARS( pev ), pSrcNode->m_vecOrigin );

				if ( hull != NODE_FLY_HULL )
				{
					vecDirToCheckNode  = ( pDestNode->m_vecOrigin - pev->origin );
					flDist             = vecDirToCheckNode.Length();
					vecDirToCheckNode  = vecDirToCheckNode.Normalize();
					vec2DirToCheckNode = vecDirToCheckNode.Make2D();

					flYaw = UTIL_VecToYaw( vecDirToCheckNode );

					for ( step = 0; step < flDist; step += HULL_STEP_SIZE )
					{
						pev->angles.y = flYaw;
						pev->ideal_yaw = flYaw;

						if ( !WALK_MOVE( ENT( pev ), flYaw, HULL_STEP_SIZE, WALKMOVE_CHECKONLY ) )
						{
							fSkipRemainingHulls = TRUE;
							break;
						}
					}
				}

				if ( !fSkipRemainingHulls )
				{
					switch ( hull )
					{
					case NODE_SMALL_HULL:
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo |= bits_LINK_SMALL_HULL;
						break;
					case NODE_HUMAN_HULL:
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo |= bits_LINK_HUMAN_HULL;
						break;
					case NODE_LARGE_HULL:
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo |= bits_LINK_LARGE_HULL;
						break;
					case NODE_FLY_HULL:
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo |= bits_LINK_FLY_HULL;
						break;
					}
				}
			}

			if ( !pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo )
			{
				pTempPool[pSrcNode->m_iFirstLink + j] = pTempPool[pSrcNode->m_iFirstLink + ( pSrcNode->m_cNumLinks - 1 )];
				pSrcNode->m_cNumLinks--;
				j--;
			}
			else
			{
				if ( file )
				{
					fprintf( file, "Link to: %4d Valid. Weight: %4d\n", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode, (int)pTempPool[pSrcNode->m_iFirstLink + j].m_flWeight );
				}
			}
		}

		if ( file )
		{
			fprintf( file, "----------------------------------------------------------------------------\n" );
		}
	}

	if ( file )
	{
		fprintf( file, "\nGraph Link Pairing Matrix:\n" );
	}

	fPairsValid = TRUE;
	for ( i = 0; i < WorldGraph.m_cNodes; i++ )
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		if ( file )
		{
			fprintf( file, "\nNode %4d ( %2d links ):", i, pSrcNode->m_cNumLinks );
		}

		for ( j = 0; j < pSrcNode->m_cNumLinks; j++ )
		{
			pDestNode = &WorldGraph.m_pNodes[pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

			int k;
			for ( k = 0; k < pDestNode->m_cNumLinks; k++ )
			{
				if ( pTempPool[pDestNode->m_iFirstLink + k].m_iDestNode == i )
				{
					if ( pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo != pTempPool[pDestNode->m_iFirstLink + k].m_afLinkInfo )
					{
						if ( file )
						{
							fprintf( file, "%4d! ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode );
						}
					}
					else
					{
						if ( file )
						{
							fprintf( file, "%4d  ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode );
						}
					}
					break;
				}
			}

			if ( k == pDestNode->m_cNumLinks )
			{
				fPairsValid = FALSE;
				if ( file )
				{
					fprintf( file, "%4d* ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode );
				}
			}
		}
	}

	if ( file )
	{
		fprintf( file, "\n\nKey:\n*  = Unmatched connection.\n!  = Match is asymmetrical\n" );
	}

	if ( !fPairsValid )
	{
		ALERT( at_aiconsole, "Graph contains unmatched connections!\n" );
	}

	cPoolLinks = 0;
	for ( i = 0; i < WorldGraph.m_cNodes; i++ )
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		if ( pSrcNode->m_cNumLinks > cMaxValidLinks )
		{
			cMaxValidLinks = pSrcNode->m_cNumLinks;
		}

		cPoolLinks += pSrcNode->m_cNumLinks;
	}

	if ( file )
	{
		fprintf( file, "\n\n%4d Total Valid Connections - %4d Maximum connections for a single node.\n", cPoolLinks, cMaxValidLinks );
		fprintf( file, "----------------------------------------------------------------------------\n" );
	}

	WorldGraph.m_pLinkPool = (CLink *)calloc( sizeof( CLink ), cPoolLinks );

	if ( !WorldGraph.m_pLinkPool )
	{
		ALERT( at_aiconsole, "Could not malloc LinkPool!\n" );
		if ( file )
		{
			fclose( file );
		}
		free( pTempPool );
		return;
	}

	for ( i = 0; i < WorldGraph.m_cNodes; i++ )
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		for ( j = 0; j < pSrcNode->m_cNumLinks; j++ )
		{
			WorldGraph.m_pLinkPool[iPoolIndex] = pTempPool[pSrcNode->m_iFirstLink + j];
			iPoolIndex++;
		}

		pSrcNode->m_iFirstLink = iPoolIndex - pSrcNode->m_cNumLinks;
	}

	WorldGraph.m_cLinks = cPoolLinks;

	WorldGraph.m_fGraphPresent     = TRUE;
	WorldGraph.m_fGraphPointersSet = TRUE;

	free( pTempPool );

	if ( file )
	{
		fclose( file );
	}

	WorldGraph.SortNodes();
	WorldGraph.BuildLinkLookups();
	WorldGraph.BuildRegionTables();
	WorldGraph.ComputeStaticRoutingTables();

	WorldGraph.FSaveGraph( (char *)STRING( gpGlobals->mapname ) );

	ALERT( at_aiconsole, "Node Graph Complete (%d Nodes, %d Links)\n", WorldGraph.m_cNodes, WorldGraph.m_cLinks );
}

//=========================================================
// returns a hardcoded path.
//=========================================================
void CTestHull ::PathFind( void )
{
	int iPath[50];
	int iPathSize;
	int i;
	CNode *pNode, *pNextNode;

	if ( !WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet )
	{
		ALERT( at_aiconsole, "Graph not ready!\n" );
		return;
	}

	iPathSize = WorldGraph.FindShortestPath( iPath, 0, 19, 0, 0 );

	if ( !iPathSize )
	{
		ALERT( at_aiconsole, "No Path!\n" );
		return;
	}

	ALERT( at_aiconsole, "%d\n", iPathSize );

	pNode = &WorldGraph.m_pNodes[iPath[0]];

	for ( i = 0; i < iPathSize - 1; i++ )
	{
		pNextNode = &WorldGraph.m_pNodes[iPath[i + 1]];

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SHOWLINE );

		WRITE_COORD( pNode->m_vecOrigin.x );
		WRITE_COORD( pNode->m_vecOrigin.y );
		WRITE_COORD( pNode->m_vecOrigin.z + NODE_HEIGHT );

		WRITE_COORD( pNextNode->m_vecOrigin.x );
		WRITE_COORD( pNextNode->m_vecOrigin.y );
		WRITE_COORD( pNextNode->m_vecOrigin.z + NODE_HEIGHT );
		MESSAGE_END();

		pNode = pNextNode;
	}
}

//=========================================================
// CNodeViewer - Draws a graph of the shorted path from all nodes
// to current location (typically the player).  It then draws
// as many connects as it can per frame, trying not to overflow the buffer
//=========================================================
class CNodeViewer : public CBaseEntity
{
  public:
	void Spawn( void );

	int m_iBaseNode;
	int m_iDraw;
	int m_nVisited;
	int m_aFrom[128];
	int m_aTo[128];
	int m_iHull;
	int m_afNodeType;
	Vector m_vecColor;

	void FindNodeConnections( int iNode );
	void AddNode( int iFrom, int iTo );
	void EXPORT DrawThink( void );
};
LINK_ENTITY_TO_CLASS( node_viewer, CNodeViewer );
LINK_ENTITY_TO_CLASS( node_viewer_human, CNodeViewer );
LINK_ENTITY_TO_CLASS( node_viewer_fly, CNodeViewer );
LINK_ENTITY_TO_CLASS( node_viewer_large, CNodeViewer );

void CNodeViewer::Spawn()
{
	if ( !WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet )
	{
		ALERT( at_console, "Graph not ready!\n" );
		UTIL_Remove( this );
		return;
	}

	if ( FClassnameIs( pev, "node_viewer_fly" ) )
	{
		m_iHull      = NODE_FLY_HULL;
		m_afNodeType = bits_NODE_AIR;
		m_vecColor   = Vector( 160, 100, 255 );
	}
	else if ( FClassnameIs( pev, "node_viewer_large" ) )
	{
		m_iHull      = NODE_LARGE_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor   = Vector( 100, 255, 160 );
	}
	else
	{
		m_iHull      = NODE_HUMAN_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor   = Vector( 255, 160, 100 );
	}

	m_iBaseNode = WorldGraph.FindNearestNode( pev->origin, m_afNodeType );

	if ( m_iBaseNode < 0 )
	{
		ALERT( at_console, "No nearby node\n" );
		return;
	}

	m_nVisited = 0;

	ALERT( at_aiconsole, "basenode %d\n", m_iBaseNode );

	if ( WorldGraph.m_cNodes < 128 )
	{
		for ( int i = 0; i < WorldGraph.m_cNodes; i++ )
		{
			AddNode( i, WorldGraph.NextNodeInRoute( i, m_iBaseNode, m_iHull, 0 ) );
		}
	}
	else
	{
		FindNodeConnections( m_iBaseNode );

		int start = 0;
		int end;
		do
		{
			end = m_nVisited;
			for ( end = m_nVisited; start < end; start++ )
			{
				FindNodeConnections( m_aFrom[start] );
				FindNodeConnections( m_aTo[start] );
			}
		} while ( end != m_nVisited );
	}

	ALERT( at_aiconsole, "%d nodes\n", m_nVisited );

	m_iDraw = 0;
	SetThink( &CNodeViewer::DrawThink );
	pev->nextthink = gpGlobals->time;
}

void CNodeViewer ::FindNodeConnections( int iNode )
{
	AddNode( iNode, WorldGraph.NextNodeInRoute( iNode, m_iBaseNode, m_iHull, 0 ) );
	for ( int i = 0; i < WorldGraph.m_pNodes[iNode].m_cNumLinks; i++ )
	{
		CLink *pToLink = &WorldGraph.NodeLink( iNode, i );
		AddNode( pToLink->m_iDestNode, WorldGraph.NextNodeInRoute( pToLink->m_iDestNode, m_iBaseNode, m_iHull, 0 ) );
	}
}

void CNodeViewer::AddNode( int iFrom, int iTo )
{
	if ( m_nVisited >= 128 )
	{
		return;
	}
	else
	{
		if ( iFrom == iTo )
			return;

		for ( int i = 0; i < m_nVisited; i++ )
		{
			if ( m_aFrom[i] == iFrom && m_aTo[i] == iTo )
				return;
			if ( m_aFrom[i] == iTo && m_aTo[i] == iFrom )
				return;
		}
		m_aFrom[m_nVisited] = iFrom;
		m_aTo[m_nVisited]   = iTo;
		m_nVisited++;
	}
}

void CNodeViewer ::DrawThink( void )
{
	pev->nextthink = gpGlobals->time;

	for ( int i = 0; i < 10; i++ )
	{
		if ( m_iDraw == m_nVisited )
		{
			UTIL_Remove( this );
			return;
		}

		extern short g_sModelIndexLaser;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS );
		WRITE_COORD( WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.x );
		WRITE_COORD( WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.y );
		WRITE_COORD( WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.z + NODE_HEIGHT );

		WRITE_COORD( WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.x );
		WRITE_COORD( WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.y );
		WRITE_COORD( WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.z + NODE_HEIGHT );
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 );            // framerate
		WRITE_BYTE( 0 );            // framerate
		WRITE_BYTE( 250 );          // life
		WRITE_BYTE( 40 );           // width
		WRITE_BYTE( 0 );            // noise
		WRITE_BYTE( m_vecColor.x ); // r, g, b
		WRITE_BYTE( m_vecColor.y ); // r, g, b
		WRITE_BYTE( m_vecColor.z ); // r, g, b
		WRITE_BYTE( 128 );          // brightness
		WRITE_BYTE( 0 );            // speed
		MESSAGE_END();

		m_iDraw++;
	}
}
