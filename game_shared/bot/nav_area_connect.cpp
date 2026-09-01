// nav_area.cpp
// AI Navigation areas
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#pragma warning( disable : 4530 ) // STL uses exceptions, but we are not compiling with them - ignore warning
#pragma warning( disable : 4786 ) // long STL names get truncated in browse info.

#include <list>
#include <vector>
#include <algorithm>

#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef _WIN32
#include <io.h>

#else
#include <unistd.h>
#define _write write
#define _close close
#define MAX_OSPATH PATH_MAX
#endif

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "core/player.h"
#include "gamerules.h"
#include "bot_util.h"

/// @todo Abstract hostages and cs-bots out of here
#include "cs_bot.h"
#include "cs_bot_manager.h"
#include "hostage.h"

#include "nav.h"
#include "nav_node.h"
#include "nav_area.h"

#include "pm_shared.h" // for OBS_ROAMING

extern void HintMessageToAllPlayers( const char *message );

unsigned int CNavArea::m_nextID = 1;
NavAreaList TheNavAreaList;

void CNavArea::ConnectTo( CNavArea *area, NavDirType dir )
{
	// check if already connected
	for ( NavConnectList::iterator iter = m_connect[dir].begin(); iter != m_connect[dir].end(); ++iter )
		if ( ( *iter ).area == area )
			return;

	NavConnect con;
	con.area = area;
	m_connect[dir].push_back( con );

	// static char *dirName[] = { "NORTH", "EAST", "SOUTH", "WEST" };
	// CONSOLE_ECHO( "  Connected area #%d to #%d, %s\n", m_id, area->m_id, dirName[ dir ] );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Disconnect this area from given area
 */
void CNavArea::Disconnect( CNavArea *area )
{
	NavConnect connect;
	connect.area = area;

	for ( int dir = 0; dir < NUM_DIRECTIONS; dir++ )
		m_connect[dir].remove( connect );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recompute internal data once nodes have been adjusted during merge
 * Destroy adjArea.
 */
void CNavArea::FinishMerge( CNavArea *adjArea )
{
	// update extent
	m_extent.lo = *m_node[NORTH_WEST]->GetPosition();
	m_extent.hi = *m_node[SOUTH_EAST]->GetPosition();

	m_center.x = ( m_extent.lo.x + m_extent.hi.x ) / 2.0f;
	m_center.y = ( m_extent.lo.y + m_extent.hi.y ) / 2.0f;
	m_center.z = ( m_extent.lo.z + m_extent.hi.z ) / 2.0f;

	m_neZ = m_node[NORTH_EAST]->GetPosition()->z;
	m_swZ = m_node[SOUTH_WEST]->GetPosition()->z;

	// reassign the adjacent area's internal nodes to the final area
	adjArea->AssignNodes( this );

	// merge adjacency links - we gain all the connections that adjArea had
	MergeAdjacentConnections( adjArea );

	// remove subsumed adjacent area
	TheNavAreaList.remove( adjArea );
	delete adjArea;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * For merging with "adjArea" - pick up all of "adjArea"s connections
 */
void CNavArea::MergeAdjacentConnections( CNavArea *adjArea )
{
	// merge adjacency links - we gain all the connections that adjArea had
	NavConnectList::iterator iter;
	int dir;
	for ( dir = 0; dir < NUM_DIRECTIONS; dir++ )
	{
		for ( iter = adjArea->m_connect[dir].begin(); iter != adjArea->m_connect[dir].end(); ++iter )
		{
			NavConnect connect = *iter;

			if ( connect.area != adjArea && connect.area != this )
				ConnectTo( connect.area, (NavDirType)dir );
		}
	}

	// remove any references from this area to the adjacent area, since it is now part of us
	for ( dir = 0; dir < NUM_DIRECTIONS; dir++ )
	{
		NavConnect connect;
		connect.area = adjArea;

		m_connect[dir].remove( connect );
	}

	// Change other references to adjArea to refer instead to us
	// We can't just replace existing connections, as several adjacent areas may have been merged into one,
	// resulting in a large area adjacent to all of them ending up with multiple redunandant connections
	// into the merged area, one for each of the adjacent subsumed smaller ones.
	// If an area has a connection to the merged area, we must remove all references to adjArea, and add
	// a single connection to us.
	for ( NavAreaList::iterator areaIter = TheNavAreaList.begin(); areaIter != TheNavAreaList.end(); ++areaIter )
	{
		CNavArea *area = *areaIter;

		if ( area == this || area == adjArea )
			continue;

		for ( dir = 0; dir < NUM_DIRECTIONS; dir++ )
		{
			// check if there are any references to adjArea in this direction
			bool connected = false;
			for ( iter = area->m_connect[dir].begin(); iter != area->m_connect[dir].end(); ++iter )
			{
				NavConnect connect = *iter;

				if ( connect.area == adjArea )
				{
					connected = true;
					break;
				}
			}

			if ( connected )
			{
				// remove all references to adjArea
				NavConnect connect;
				connect.area = adjArea;
				area->m_connect[dir].remove( connect );

				// remove all references to the new area
				connect.area = this;
				area->m_connect[dir].remove( connect );

				// add a single connection to the new area
				connect.area = this;
				area->m_connect[dir].push_back( connect );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Assign internal nodes to the given area
 * NOTE: "internal" nodes do not include the east or south border nodes
 */
void CNavArea::AssignNodes( CNavArea *area )
{
	CNavNode *horizLast = m_node[NORTH_EAST];

	for ( CNavNode *vertNode = m_node[NORTH_WEST]; vertNode != m_node[SOUTH_WEST]; vertNode = vertNode->GetConnectedNode( SOUTH ) )
	{
		for ( CNavNode *horizNode = vertNode; horizNode != horizLast; horizNode = horizNode->GetConnectedNode( EAST ) )
		{
			horizNode->AssignArea( area );
		}

		horizLast = horizLast->GetConnectedNode( SOUTH );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Split this area into two areas at the given edge.
 * Preserve all adjacency connections.
 * NOTE: This does not update node connections, only areas.
 */
bool CNavArea::SplitEdit( bool splitAlongX, float splitEdge, CNavArea **outAlpha, CNavArea **outBeta )
{
	CNavArea *alpha = NULL;
	CNavArea *beta  = NULL;

	if ( splitAlongX )
	{
		// +-----+->X
		// |  A  |
		// +-----+
		// |  B  |
		// +-----+
		// |
		// Y

		// don't do split if at edge of area
		if ( splitEdge <= m_extent.lo.y + 1.0f )
			return false;

		if ( splitEdge >= m_extent.hi.y - 1.0f )
			return false;

		alpha              = new CNavArea;
		alpha->m_extent.lo = m_extent.lo;

		alpha->m_extent.hi.x = m_extent.hi.x;
		alpha->m_extent.hi.y = splitEdge;
		alpha->m_extent.hi.z = GetZ( &alpha->m_extent.hi );

		beta                = new CNavArea;
		beta->m_extent.lo.x = m_extent.lo.x;
		beta->m_extent.lo.y = splitEdge;
		beta->m_extent.lo.z = GetZ( &beta->m_extent.lo );

		beta->m_extent.hi = m_extent.hi;

		alpha->ConnectTo( beta, SOUTH );
		beta->ConnectTo( alpha, NORTH );

		FinishSplitEdit( alpha, SOUTH );
		FinishSplitEdit( beta, NORTH );
	}
	else
	{
		// +--+--+->X
		// |  |  |
		// | A|B |
		// |  |  |
		// +--+--+
		// |
		// Y

		// don't do split if at edge of area
		if ( splitEdge <= m_extent.lo.x + 1.0f )
			return false;

		if ( splitEdge >= m_extent.hi.x - 1.0f )
			return false;

		alpha              = new CNavArea;
		alpha->m_extent.lo = m_extent.lo;

		alpha->m_extent.hi.x = splitEdge;
		alpha->m_extent.hi.y = m_extent.hi.y;
		alpha->m_extent.hi.z = GetZ( &alpha->m_extent.hi );

		beta                = new CNavArea;
		beta->m_extent.lo.x = splitEdge;
		beta->m_extent.lo.y = m_extent.lo.y;
		beta->m_extent.lo.z = GetZ( &beta->m_extent.lo );

		beta->m_extent.hi = m_extent.hi;

		alpha->ConnectTo( beta, EAST );
		beta->ConnectTo( alpha, WEST );

		FinishSplitEdit( alpha, EAST );
		FinishSplitEdit( beta, WEST );
	}

	// new areas inherit attributes from original area
	alpha->SetAttributes( GetAttributes() );
	beta->SetAttributes( GetAttributes() );

	// new areas inherit place from original area
	alpha->SetPlace( GetPlace() );
	beta->SetPlace( GetPlace() );

	// return new areas
	if ( outAlpha )
		*outAlpha = alpha;

	if ( outBeta )
		*outBeta = beta;

	// remove original area
	TheNavAreaList.remove( this );
	delete this;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given area is connected in given direction
 * if dir == NUM_DIRECTIONS, check all directions (direction is unknown)
 * @todo Formalize "asymmetric" flag on connections
 */
bool CNavArea::IsConnected( const CNavArea *area, NavDirType dir ) const
{
	// we are connected to ourself
	if ( area == this )
		return true;

	NavConnectList::const_iterator iter;

	if ( dir == NUM_DIRECTIONS )
	{
		// search all directions
		for ( int d = 0; d < NUM_DIRECTIONS; ++d )
		{
			for ( iter = m_connect[d].begin(); iter != m_connect[d].end(); ++iter )
			{
				if ( area == ( *iter ).area )
					return true;
			}
		}

		// check ladder connections
		NavLadderList::const_iterator liter;
		for ( liter = m_ladder[LADDER_UP].begin(); liter != m_ladder[LADDER_UP].end(); ++liter )
		{
			CNavLadder *ladder = *liter;

			if ( ladder->m_topBehindArea == area ||
			     ladder->m_topForwardArea == area ||
			     ladder->m_topLeftArea == area ||
			     ladder->m_topRightArea == area )
				return true;
		}

		for ( liter = m_ladder[LADDER_DOWN].begin(); liter != m_ladder[LADDER_DOWN].end(); ++liter )
		{
			CNavLadder *ladder = *liter;

			if ( ladder->m_bottomArea == area )
				return true;
		}
	}
	else
	{
		// check specific direction
		for ( iter = m_connect[dir].begin(); iter != m_connect[dir].end(); ++iter )
		{
			if ( area == ( *iter ).area )
				return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute change in height from this area to given area
 * @todo This is approximate for now
 */
float CNavArea::ComputeHeightChange( const CNavArea *area )
{
	float ourZ  = GetZ( GetCenter() );
	float areaZ = area->GetZ( area->GetCenter() );

	return areaZ - ourZ;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given the portion of the original area, update its internal data
 * The "ignoreEdge" direction defines the side of the original area that the new area does not include
 */
void CNavArea::FinishSplitEdit( CNavArea *newArea, NavDirType ignoreEdge )
{
	newArea->m_center.x = ( newArea->m_extent.lo.x + newArea->m_extent.hi.x ) / 2.0f;
	newArea->m_center.y = ( newArea->m_extent.lo.y + newArea->m_extent.hi.y ) / 2.0f;
	newArea->m_center.z = ( newArea->m_extent.lo.z + newArea->m_extent.hi.z ) / 2.0f;

	newArea->m_neZ = GetZ( newArea->m_extent.hi.x, newArea->m_extent.lo.y );
	newArea->m_swZ = GetZ( newArea->m_extent.lo.x, newArea->m_extent.hi.y );

	// connect to adjacent areas
	for ( int d = 0; d < NUM_DIRECTIONS; ++d )
	{
		if ( d == ignoreEdge )
			continue;

		int count = GetAdjacentCount( (NavDirType)d );

		for ( int a = 0; a < count; ++a )
		{
			CNavArea *adj = GetAdjacentArea( (NavDirType)d, a );

			switch ( d )
			{
			case NORTH:
			case SOUTH:
				if ( newArea->IsOverlappingX( adj ) )
				{
					newArea->ConnectTo( adj, (NavDirType)d );

					// add reciprocal connection if needed
					if ( adj->IsConnected( this, OppositeDirection( (NavDirType)d ) ) )
						adj->ConnectTo( newArea, OppositeDirection( (NavDirType)d ) );
				}
				break;

			case EAST:
			case WEST:
				if ( newArea->IsOverlappingY( adj ) )
				{
					newArea->ConnectTo( adj, (NavDirType)d );

					// add reciprocal connection if needed
					if ( adj->IsConnected( this, OppositeDirection( (NavDirType)d ) ) )
						adj->ConnectTo( newArea, OppositeDirection( (NavDirType)d ) );
				}
				break;
			}
		}
	}

	TheNavAreaList.push_back( newArea );
	TheNavAreaGrid.AddNavArea( newArea );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Create a new area between this area and given area
 */
bool CNavArea::SpliceEdit( CNavArea *other )
{
	CNavArea *newArea = NULL;
	Vector nw, ne, se, sw;

	if ( m_extent.lo.x > other->m_extent.hi.x )
	{
		// 'this' is east of 'other'
		float top    = max( m_extent.lo.y, other->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, other->m_extent.hi.y );

		nw.x = other->m_extent.hi.x;
		nw.y = top;
		nw.z = other->GetZ( &nw );

		se.x = m_extent.lo.x;
		se.y = bottom;
		se.z = GetZ( &se );

		ne.x = se.x;
		ne.y = nw.y;
		ne.z = GetZ( &ne );

		sw.x = nw.x;
		sw.y = se.y;
		sw.z = other->GetZ( &sw );

		newArea = new CNavArea( &nw, &ne, &se, &sw );

		this->ConnectTo( newArea, WEST );
		newArea->ConnectTo( this, EAST );

		other->ConnectTo( newArea, EAST );
		newArea->ConnectTo( other, WEST );
	}
	else if ( m_extent.hi.x < other->m_extent.lo.x )
	{
		// 'this' is west of 'other'
		float top    = max( m_extent.lo.y, other->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, other->m_extent.hi.y );

		nw.x = m_extent.hi.x;
		nw.y = top;
		nw.z = GetZ( &nw );

		se.x = other->m_extent.lo.x;
		se.y = bottom;
		se.z = other->GetZ( &se );

		ne.x = se.x;
		ne.y = nw.y;
		ne.z = other->GetZ( &ne );

		sw.x = nw.x;
		sw.y = se.y;
		sw.z = GetZ( &sw );

		newArea = new CNavArea( &nw, &ne, &se, &sw );

		this->ConnectTo( newArea, EAST );
		newArea->ConnectTo( this, WEST );

		other->ConnectTo( newArea, WEST );
		newArea->ConnectTo( other, EAST );
	}
	else // 'this' overlaps in X
	{
		if ( m_extent.lo.y > other->m_extent.hi.y )
		{
			// 'this' is south of 'other'
			float left  = max( m_extent.lo.x, other->m_extent.lo.x );
			float right = min( m_extent.hi.x, other->m_extent.hi.x );

			nw.x = left;
			nw.y = other->m_extent.hi.y;
			nw.z = other->GetZ( &nw );

			se.x = right;
			se.y = m_extent.lo.y;
			se.z = GetZ( &se );

			ne.x = se.x;
			ne.y = nw.y;
			ne.z = other->GetZ( &ne );

			sw.x = nw.x;
			sw.y = se.y;
			sw.z = GetZ( &sw );

			newArea = new CNavArea( &nw, &ne, &se, &sw );

			this->ConnectTo( newArea, NORTH );
			newArea->ConnectTo( this, SOUTH );

			other->ConnectTo( newArea, SOUTH );
			newArea->ConnectTo( other, NORTH );
		}
		else if ( m_extent.hi.y < other->m_extent.lo.y )
		{
			// 'this' is north of 'other'
			float left  = max( m_extent.lo.x, other->m_extent.lo.x );
			float right = min( m_extent.hi.x, other->m_extent.hi.x );

			nw.x = left;
			nw.y = m_extent.hi.y;
			nw.z = GetZ( &nw );

			se.x = right;
			se.y = other->m_extent.lo.y;
			se.z = other->GetZ( &se );

			ne.x = se.x;
			ne.y = nw.y;
			ne.z = GetZ( &ne );

			sw.x = nw.x;
			sw.y = se.y;
			sw.z = other->GetZ( &sw );

			newArea = new CNavArea( &nw, &ne, &se, &sw );

			this->ConnectTo( newArea, SOUTH );
			newArea->ConnectTo( this, NORTH );

			other->ConnectTo( newArea, NORTH );
			newArea->ConnectTo( other, SOUTH );
		}
		else
		{
			// areas overlap
			return false;
		}
	}

	// if both areas have the same place, the new area inherits it
	if ( GetPlace() == other->GetPlace() )
	{
		newArea->SetPlace( GetPlace() );
	}
	else if ( GetPlace() == UNDEFINED_PLACE )
	{
		newArea->SetPlace( other->GetPlace() );
	}
	else if ( other->GetPlace() == UNDEFINED_PLACE )
	{
		newArea->SetPlace( GetPlace() );
	}
	else
	{
		// both have valid, but different places - pick on at random
		if ( RANDOM_LONG( 0, 100 ) < 50 )
			newArea->SetPlace( GetPlace() );
		else
			newArea->SetPlace( other->GetPlace() );
	}

	TheNavAreaList.push_back( newArea );
	TheNavAreaGrid.AddNavArea( newArea );

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Merge this area and given adjacent area
 */
bool CNavArea::MergeEdit( CNavArea *adj )
{
	// can only merge if attributes of both areas match

	// check that these areas can be merged
	const float tolerance = 1.0f;
	bool merge            = false;
	if ( ABS( m_extent.lo.x - adj->m_extent.lo.x ) < tolerance &&
	     ABS( m_extent.hi.x - adj->m_extent.hi.x ) < tolerance )
		merge = true;

	if ( ABS( m_extent.lo.y - adj->m_extent.lo.y ) < tolerance &&
	     ABS( m_extent.hi.y - adj->m_extent.hi.y ) < tolerance )
		merge = true;

	if ( merge == false )
		return false;

	Extent origExtent = m_extent;

	// update extent
	if ( m_extent.lo.x > adj->m_extent.lo.x || m_extent.lo.y > adj->m_extent.lo.y )
		m_extent.lo = adj->m_extent.lo;

	if ( m_extent.hi.x < adj->m_extent.hi.x || m_extent.hi.y < adj->m_extent.hi.y )
		m_extent.hi = adj->m_extent.hi;

	m_center.x = ( m_extent.lo.x + m_extent.hi.x ) / 2.0f;
	m_center.y = ( m_extent.lo.y + m_extent.hi.y ) / 2.0f;
	m_center.z = ( m_extent.lo.z + m_extent.hi.z ) / 2.0f;

	if ( m_extent.hi.x > origExtent.hi.x || m_extent.lo.y < origExtent.lo.y )
		m_neZ = adj->GetZ( m_extent.hi.x, m_extent.lo.y );
	else
		m_neZ = GetZ( m_extent.hi.x, m_extent.lo.y );

	if ( m_extent.lo.x < origExtent.lo.x || m_extent.hi.y > origExtent.hi.y )
		m_swZ = adj->GetZ( m_extent.lo.x, m_extent.hi.y );
	else
		m_swZ = GetZ( m_extent.lo.x, m_extent.hi.y );

	// merge adjacency links - we gain all the connections that adjArea had
	MergeAdjacentConnections( adj );

	// remove subsumed adjacent area
	TheNavAreaList.remove( adj );
	delete adj;

	return true;
}

//--------------------------------------------------------------------------------------------------------------

/**
 * Start at given position and find first area in given direction
 */
inline CNavArea *FindFirstAreaInDirection( const Vector *start, NavDirType dir, float range, float beneathLimit, CBaseEntity *traceIgnore = NULL, Vector *closePos = NULL )
{
	CNavArea *area = NULL;

	Vector pos = *start;

	int end = (int)( ( range / GenerationStepSize ) + 0.5f );

	for ( int i = 1; i <= end; i++ )
	{
		AddDirectionVector( &pos, dir, GenerationStepSize );

		// make sure we dont look thru the wall
		TraceResult result;

		if ( traceIgnore )
			UTIL_TraceLine( *start, pos, ignore_monsters, ENT( traceIgnore->pev ), &result );
		else
			UTIL_TraceLine( *start, pos, ignore_monsters, NULL, &result );

		if ( result.flFraction != 1.0f )
			break;

		area = TheNavAreaGrid.GetNavArea( &pos, beneathLimit );
		if ( area )
		{
			if ( closePos )
			{
				closePos->x = pos.x;
				closePos->y = pos.y;
				closePos->z = area->GetZ( pos.x, pos.y );
			}

			break;
		}
	}

	return area;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Determine if we can "jump down" from given point
 */
inline bool testJumpDown( const Vector *fromPos, const Vector *toPos )
{
	float dz = fromPos->z - toPos->z;

	// drop can't be too far, or too short (or nonexistant)
	if ( dz <= JumpCrouchHeight || dz >= DeathDrop )
		return false;

	//
	// Check LOS out and down
	//
	// ------+
	//       |
	// F     |
	//       |
	//       T
	//

	Vector from( fromPos->x, fromPos->y, fromPos->z + HumanHeight );
	Vector to( toPos->x, toPos->y, from.z );

	TraceResult result;
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if ( result.flFraction != 1.0f || result.fStartSolid )
		return false;

	from = to;
	to.z = toPos->z + 2.0f;
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if ( result.flFraction != 1.0f || result.fStartSolid )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
inline CNavArea *findJumpDownArea( const Vector *fromPos, NavDirType dir )
{
	Vector start( fromPos->x, fromPos->y, fromPos->z + HalfHumanHeight );
	AddDirectionVector( &start, dir, GenerationStepSize / 2.0f );

	Vector toPos;
	CNavArea *downArea = FindFirstAreaInDirection( &start, dir, 4.0f * GenerationStepSize, DeathDrop, NULL, &toPos );

	if ( downArea && testJumpDown( fromPos, &toPos ) )
		return downArea;

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Define connections between adjacent generated areas
 */
void ConnectGeneratedAreas( void )
{
	CONSOLE_ECHO( "  Connecting navigation areas...\n" );

	for ( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		// scan along edge nodes, stepping one node over into the next area
		// for now, only use bi-directional connections

		// north edge
		CNavNode *node;
		for ( node = area->m_node[NORTH_WEST]; node != area->m_node[NORTH_EAST]; node = node->GetConnectedNode( EAST ) )
		{
			CNavNode *adj = node->GetConnectedNode( NORTH );

			if ( adj && adj->GetArea() && adj->GetConnectedNode( SOUTH ) == node )
			{
				area->ConnectTo( adj->GetArea(), NORTH );
			}
			else
			{
				CNavArea *downArea = findJumpDownArea( node->GetPosition(), NORTH );
				if ( downArea && downArea != area )
					area->ConnectTo( downArea, NORTH );
			}
		}

		// west edge
		for ( node = area->m_node[NORTH_WEST]; node != area->m_node[SOUTH_WEST]; node = node->GetConnectedNode( SOUTH ) )
		{
			CNavNode *adj = node->GetConnectedNode( WEST );

			if ( adj && adj->GetArea() && adj->GetConnectedNode( EAST ) == node )
			{
				area->ConnectTo( adj->GetArea(), WEST );
			}
			else
			{
				CNavArea *downArea = findJumpDownArea( node->GetPosition(), WEST );
				if ( downArea && downArea != area )
					area->ConnectTo( downArea, WEST );
			}
		}

		// south edge - this edge's nodes are actually part of adjacent areas
		// move one node north, and scan west to east
		/// @todo This allows one-node-wide areas - do we want this?
		node = area->m_node[SOUTH_WEST];
		node = node->GetConnectedNode( NORTH );
		if ( node )
		{
			CNavNode *end = area->m_node[SOUTH_EAST]->GetConnectedNode( NORTH );
			/// @todo Figure out why cs_backalley gets a NULL node in here...
			for ( ; node && node != end; node = node->GetConnectedNode( EAST ) )
			{
				CNavNode *adj = node->GetConnectedNode( SOUTH );

				if ( adj && adj->GetArea() && adj->GetConnectedNode( NORTH ) == node )
				{
					area->ConnectTo( adj->GetArea(), SOUTH );
				}
				else
				{
					CNavArea *downArea = findJumpDownArea( node->GetPosition(), SOUTH );
					if ( downArea && downArea != area )
						area->ConnectTo( downArea, SOUTH );
				}
			}
		}

		// east edge - this edge's nodes are actually part of adjacent areas
		node = area->m_node[NORTH_EAST];
		node = node->GetConnectedNode( WEST );
		if ( node )
		{
			CNavNode *end = area->m_node[SOUTH_EAST]->GetConnectedNode( WEST );
			for ( ; node && node != end; node = node->GetConnectedNode( SOUTH ) )
			{
				CNavNode *adj = node->GetConnectedNode( EAST );

				if ( adj && adj->GetArea() && adj->GetConnectedNode( WEST ) == node )
				{
					area->ConnectTo( adj->GetArea(), EAST );
				}
				else
				{
					CNavArea *downArea = findJumpDownArea( node->GetPosition(), EAST );
					if ( downArea && downArea != area )
						area->ConnectTo( downArea, EAST );
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Merge areas together to make larger ones (must remain rectangular - convex).
 * Areas can only be merged if their attributes match.
 */
void MergeGeneratedAreas( void )
{
	CONSOLE_ECHO( "  Merging navigation areas...\n" );

	bool merged;

	do
	{
		merged = false;

		for ( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
		{
			CNavArea *area = *iter;

			// north edge
			NavConnectList::iterator citer;
			for ( citer = area->m_connect[NORTH].begin(); citer != area->m_connect[NORTH].end(); ++citer )
			{
				CNavArea *adjArea = ( *citer ).area;

				if ( area->m_node[NORTH_WEST] == adjArea->m_node[SOUTH_WEST] &&
				     area->m_node[NORTH_EAST] == adjArea->m_node[SOUTH_EAST] &&
				     area->GetAttributes() == adjArea->GetAttributes() &&
				     area->IsCoplanar( adjArea ) )
				{
					// merge vertical
					area->m_node[NORTH_WEST] = adjArea->m_node[NORTH_WEST];
					area->m_node[NORTH_EAST] = adjArea->m_node[NORTH_EAST];

					merged = true;
					// CONSOLE_ECHO( "  Merged (north) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if ( merged )
				break;

			// south edge
			for ( citer = area->m_connect[SOUTH].begin(); citer != area->m_connect[SOUTH].end(); ++citer )
			{
				CNavArea *adjArea = ( *citer ).area;

				if ( adjArea->m_node[NORTH_WEST] == area->m_node[SOUTH_WEST] &&
				     adjArea->m_node[NORTH_EAST] == area->m_node[SOUTH_EAST] &&
				     area->GetAttributes() == adjArea->GetAttributes() &&
				     area->IsCoplanar( adjArea ) )
				{
					// merge vertical
					area->m_node[SOUTH_WEST] = adjArea->m_node[SOUTH_WEST];
					area->m_node[SOUTH_EAST] = adjArea->m_node[SOUTH_EAST];

					merged = true;
					// CONSOLE_ECHO( "  Merged (south) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if ( merged )
				break;

			// west edge
			for ( citer = area->m_connect[WEST].begin(); citer != area->m_connect[WEST].end(); ++citer )
			{
				CNavArea *adjArea = ( *citer ).area;

				if ( area->m_node[NORTH_WEST] == adjArea->m_node[NORTH_EAST] &&
				     area->m_node[SOUTH_WEST] == adjArea->m_node[SOUTH_EAST] &&
				     area->GetAttributes() == adjArea->GetAttributes() &&
				     area->IsCoplanar( adjArea ) )
				{
					// merge horizontal
					area->m_node[NORTH_WEST] = adjArea->m_node[NORTH_WEST];
					area->m_node[SOUTH_WEST] = adjArea->m_node[SOUTH_WEST];

					merged = true;
					// CONSOLE_ECHO( "  Merged (west) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if ( merged )
				break;

			// east edge
			for ( citer = area->m_connect[EAST].begin(); citer != area->m_connect[EAST].end(); ++citer )
			{
				CNavArea *adjArea = ( *citer ).area;

				if ( adjArea->m_node[NORTH_WEST] == area->m_node[NORTH_EAST] &&
				     adjArea->m_node[SOUTH_WEST] == area->m_node[SOUTH_EAST] &&
				     area->GetAttributes() == adjArea->GetAttributes() &&
				     area->IsCoplanar( adjArea ) )
				{
					// merge horizontal
					area->m_node[NORTH_EAST] = adjArea->m_node[NORTH_EAST];
					area->m_node[SOUTH_EAST] = adjArea->m_node[SOUTH_EAST];

					merged = true;
					// CONSOLE_ECHO( "  Merged (east) areas #%d and #%d\n", area->m_id, adjArea->m_id );

					area->FinishMerge( adjArea );

					// restart scan - iterator is invalidated
					break;
				}
			}

			if ( merged )
				break;
		}
	} while ( merged );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if area is more or less square.
 * This is used when merging to prevent long, thin, areas being created.
 */
inline bool IsAreaRoughlySquare( const CNavArea *area )
{
	float aspect = area->GetSizeX() / area->GetSizeY();

	const float maxAspect = 3.01;
	const float minAspect = 1.0f / maxAspect;
	if ( aspect < minAspect || aspect > maxAspect )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recursively chop area in half along X until child areas are roughly square
 */
void SplitX( CNavArea *area )
{
	if ( IsAreaRoughlySquare( area ) )
		return;

	float split = area->GetSizeX();
	split /= 2.0f;
	split += area->GetExtent()->lo.x;

	SnapToGrid( &split );

	const float epsilon = 0.1f;
	if ( abs( split - area->GetExtent()->lo.x ) < epsilon ||
	     abs( split - area->GetExtent()->hi.x ) < epsilon )
	{
		// too small to subdivide
		return;
	}

	CNavArea *alpha, *beta;
	if ( area->SplitEdit( false, split, &alpha, &beta ) )
	{
		// split each new area until square
		SplitX( alpha );
		SplitX( beta );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Recursively chop area in half along Y until child areas are roughly square
 */
void SplitY( CNavArea *area )
{
	if ( IsAreaRoughlySquare( area ) )
		return;

	float split = area->GetSizeY();
	split /= 2.0f;
	split += area->GetExtent()->lo.y;

	SnapToGrid( &split );

	const float epsilon = 0.1f;
	if ( abs( split - area->GetExtent()->lo.y ) < epsilon ||
	     abs( split - area->GetExtent()->hi.y ) < epsilon )
	{
		// too small to subdivide
		return;
	}

	CNavArea *alpha, *beta;
	if ( area->SplitEdit( true, split, &alpha, &beta ) )
	{
		// split each new area until square
		SplitY( alpha );
		SplitY( beta );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Split any long, thin, areas into roughly square chunks.
 */
void SquareUpAreas( void )
{
	NavAreaList::iterator iter = TheNavAreaList.begin();

	while ( iter != TheNavAreaList.end() )
	{
		CNavArea *area = *iter;
		++iter;

		if ( !IsAreaRoughlySquare( area ) )
		{
			// chop this area into square pieces
			if ( area->GetSizeX() > area->GetSizeY() )
				SplitX( area );
			else
				SplitY( area );
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Check if an rectangular area of the given size can be
 * made starting from the given node as the NW corner.
 * Only consider fully connected nodes for this check.
 * All of the nodes within the test area must have the same attributes.
 * All of the nodes must be approximately co-planar w.r.t the NW node's normal, with the
 * exception of 1x1 areas which can be any angle.
 */
bool TestArea( CNavNode *node, int width, int height )
{
	Vector normal = *node->GetNormal();
	float d       = -DotProduct( normal, *node->GetPosition() );

	const float offPlaneTolerance = 5.0f;

	CNavNode *vertNode, *horizNode;

	vertNode = node;
	for ( int y = 0; y < height; y++ )
	{
		horizNode = vertNode;

		for ( int x = 0; x < width; x++ )
		{
			// all nodes must have the same attributes
			if ( horizNode->GetAttributes() != node->GetAttributes() )
				return false;

			if ( horizNode->IsCovered() )
				return false;

			if ( !horizNode->IsClosedCell() )
				return false;

			horizNode = horizNode->GetConnectedNode( EAST );
			if ( horizNode == NULL )
				return false;

			// nodes must lie on/near the plane
			if ( width > 1 || height > 1 )
			{
				float dist = abs( DotProduct( *horizNode->GetPosition(), normal ) + d );
				if ( dist > offPlaneTolerance )
					return false;
			}
		}

		vertNode = vertNode->GetConnectedNode( SOUTH );
		if ( vertNode == NULL )
			return false;

		// nodes must lie on/near the plane
		if ( width > 1 || height > 1 )
		{
			float dist = abs( DotProduct( *vertNode->GetPosition(), normal ) + d );
			if ( dist > offPlaneTolerance )
				return false;
		}
	}

	// check planarity of southern edge
	if ( width > 1 || height > 1 )
	{
		horizNode = vertNode;

		for ( int x = 0; x < width; x++ )
		{
			horizNode = horizNode->GetConnectedNode( EAST );
			if ( horizNode == NULL )
				return false;

			// nodes must lie on/near the plane
			float dist = abs( DotProduct( *horizNode->GetPosition(), normal ) + d );
			if ( dist > offPlaneTolerance )
				return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Create a nav area, and mark all nodes it overlaps as "covered"
 * NOTE: Nodes on the east and south edges are not included.
 * Returns number of nodes covered by this area, or -1 for error;
 */
int BuildArea( CNavNode *node, int width, int height )
{
	// CONSOLE_ECHO( "BuildArea( #%d, %d, %d )\n", node->GetID(), width, height );

	CNavNode *nwNode = node;
	CNavNode *neNode = NULL;
	CNavNode *swNode = NULL;
	CNavNode *seNode = NULL;

	CNavNode *vertNode = node;
	CNavNode *horizNode;

	int coveredNodes = 0;

	for ( int y = 0; y < height; y++ )
	{
		horizNode = vertNode;

		for ( int x = 0; x < width; x++ )
		{
			horizNode->Cover();
			++coveredNodes;

			horizNode = horizNode->GetConnectedNode( EAST );
		}

		if ( y == 0 )
			neNode = horizNode;

		vertNode = vertNode->GetConnectedNode( SOUTH );
	}

	swNode = vertNode;

	horizNode = vertNode;
	for ( int x = 0; x < width; x++ )
	{
		horizNode = horizNode->GetConnectedNode( EAST );
	}
	seNode = horizNode;

	if ( !nwNode || !neNode || !seNode || !swNode )
	{
		CONSOLE_ECHO( "ERROR: BuildArea - NULL node.\n" );
		return -1;
	}

	CNavArea *area = new CNavArea( nwNode, neNode, seNode, swNode );
	TheNavAreaList.push_back( area );

	// since all internal nodes have the same attributes, set this area's attributes
	area->SetAttributes( node->GetAttributes() );

	//	fprintf( fp, "f %d %d %d %d\n", nwNode->m_id, neNode->m_id, seNode->m_id, swNode->m_id );

	return coveredNodes;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * For each ladder in the map, create a navigation representation of it.
 */
void BuildLadders( void )
{
	// remove any left-over ladders
	DestroyLadders();

	TraceResult result;
	CBaseEntity *entity = UTIL_FindEntityByClassname( NULL, "func_ladder" );
	while ( entity && !FNullEnt( entity->edict() ) )
	{
		CNavLadder *ladder = new CNavLadder;

		// compute top & bottom of ladder
		ladder->m_top.x = ( entity->pev->absmin.x + entity->pev->absmax.x ) / 2.0f;
		ladder->m_top.y = ( entity->pev->absmin.y + entity->pev->absmax.y ) / 2.0f;
		ladder->m_top.z = entity->pev->absmax.z;

		ladder->m_bottom.x = ladder->m_top.x;
		ladder->m_bottom.y = ladder->m_top.y;
		ladder->m_bottom.z = entity->pev->absmin.z;

		// determine facing - assumes "normal" runged ladder
		float xSize = entity->pev->absmax.x - entity->pev->absmin.x;
		float ySize = entity->pev->absmax.y - entity->pev->absmin.y;
		if ( xSize > ySize )
		{
			// ladder is facing north or south - determine which way
			// "pull in" traceline from bottom and top in case ladder abuts floor and/or ceiling
			Vector from = ladder->m_bottom + Vector( 0.0f, GenerationStepSize, GenerationStepSize );
			Vector to   = ladder->m_top + Vector( 0.0f, GenerationStepSize, -GenerationStepSize );

			UTIL_TraceLine( from, to, ignore_monsters, ENT( entity->pev ), &result );

			if ( result.flFraction != 1.0f || result.fStartSolid )
				ladder->m_dir = NORTH;
			else
				ladder->m_dir = SOUTH;
		}
		else
		{
			// ladder is facing east or west - determine which way
			Vector from = ladder->m_bottom + Vector( GenerationStepSize, 0.0f, GenerationStepSize );
			Vector to   = ladder->m_top + Vector( GenerationStepSize, 0.0f, -GenerationStepSize );

			UTIL_TraceLine( from, to, ignore_monsters, ENT( entity->pev ), &result );

			if ( result.flFraction != 1.0f || result.fStartSolid )
				ladder->m_dir = WEST;
			else
				ladder->m_dir = EAST;
		}

		// adjust top and bottom of ladder to make sure they are reachable
		// (cs_office has a crate right in front of the base of a ladder)
		Vector along = ladder->m_top - ladder->m_bottom;
		float length = along.NormalizeInPlace();
		Vector on, out;
		const float minLadderClearance = 32.0f;

		// adjust bottom to bypass blockages
		const float inc = 10.0f;
		float t;
		for ( t = 0.0f; t <= length; t += inc )
		{
			on = ladder->m_bottom + t * along;

			out = on;
			AddDirectionVector( &out, ladder->m_dir, minLadderClearance );

			UTIL_TraceLine( on, out, ignore_monsters, ENT( entity->pev ), &result );

			if ( result.flFraction == 1.0f && !result.fStartSolid )
			{
				// found viable ladder bottom
				ladder->m_bottom = on;
				break;
			}
		}

		// adjust top to bypass blockages
		for ( t = 0.0f; t <= length; t += inc )
		{
			on = ladder->m_top - t * along;

			out = on;
			AddDirectionVector( &out, ladder->m_dir, minLadderClearance );

			UTIL_TraceLine( on, out, ignore_monsters, ENT( entity->pev ), &result );

			if ( result.flFraction == 1.0f && !result.fStartSolid )
			{
				// found viable ladder top
				ladder->m_top = on;
				break;
			}
		}

		ladder->m_length = ( ladder->m_top - ladder->m_bottom ).Length();

		DirectionToVector2D( ladder->m_dir, &ladder->m_dirVector );

		ladder->m_entity            = entity;
		const float nearLadderRange = 75.0f; // 50

		//
		// Find naviagtion area at bottom of ladder
		//

		// get approximate postion of player on ladder
		Vector center = ladder->m_bottom + Vector( 0, 0, GenerationStepSize );
		AddDirectionVector( &center, ladder->m_dir, HalfHumanWidth );

		ladder->m_bottomArea = TheNavAreaGrid.GetNearestNavArea( &center, true );
		if ( !ladder->m_bottomArea )
		{
			ALERT( at_console, "ERROR: Unconnected ladder bottom at ( %g, %g, %g )\n", ladder->m_bottom.x, ladder->m_bottom.y, ladder->m_bottom.z );
		}
		else
		{
			// store reference to ladder in the area
			ladder->m_bottomArea->AddLadderUp( ladder );
		}

		//
		// Find adjacent navigation areas at the top of the ladder
		//

		// get approximate postion of player on ladder
		center = ladder->m_top + Vector( 0, 0, GenerationStepSize );
		AddDirectionVector( &center, ladder->m_dir, HalfHumanWidth );

		// find "ahead" area
		ladder->m_topForwardArea = FindFirstAreaInDirection( &center, OppositeDirection( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if ( ladder->m_topForwardArea == ladder->m_bottomArea )
			ladder->m_topForwardArea = NULL;

		// find "left" area
		ladder->m_topLeftArea = FindFirstAreaInDirection( &center, DirectionLeft( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if ( ladder->m_topLeftArea == ladder->m_bottomArea )
			ladder->m_topLeftArea = NULL;

		// find "right" area
		ladder->m_topRightArea = FindFirstAreaInDirection( &center, DirectionRight( ladder->m_dir ), nearLadderRange, 120.0f, entity );
		if ( ladder->m_topRightArea == ladder->m_bottomArea )
			ladder->m_topRightArea = NULL;

		// find "behind" area - must look farther, since ladder is against the wall away from this area
		ladder->m_topBehindArea = FindFirstAreaInDirection( &center, ladder->m_dir, 2.0f * nearLadderRange, 120.0f, entity );
		if ( ladder->m_topBehindArea == ladder->m_bottomArea )
			ladder->m_topBehindArea = NULL;

		// can't include behind area, since it is not used when going up a ladder
		if ( !ladder->m_topForwardArea && !ladder->m_topLeftArea && !ladder->m_topRightArea )
			ALERT( at_console, "ERROR: Unconnected ladder top at ( %g, %g, %g )\n", ladder->m_top.x, ladder->m_top.y, ladder->m_top.z );

		// store reference to ladder in the area(s)
		if ( ladder->m_topForwardArea )
			ladder->m_topForwardArea->AddLadderDown( ladder );

		if ( ladder->m_topLeftArea )
			ladder->m_topLeftArea->AddLadderDown( ladder );

		if ( ladder->m_topRightArea )
			ladder->m_topRightArea->AddLadderDown( ladder );

		if ( ladder->m_topBehindArea )
			ladder->m_topBehindArea->AddLadderDown( ladder );

		// adjust top of ladder to highest connected area
		float topZ       = -99999.9f;
		bool topAdjusted = false;
		CNavArea *topAreaList[4];
		topAreaList[0] = ladder->m_topForwardArea;
		topAreaList[1] = ladder->m_topLeftArea;
		topAreaList[2] = ladder->m_topRightArea;
		topAreaList[3] = ladder->m_topBehindArea;

		for ( int a = 0; a < 4; ++a )
		{
			CNavArea *topArea = topAreaList[a];
			if ( topArea == NULL )
				continue;

			Vector close;
			topArea->GetClosestPointOnArea( &ladder->m_top, &close );
			if ( topZ < close.z )
			{
				topZ        = close.z;
				topAdjusted = true;
			}
		}

		if ( topAdjusted )
			ladder->m_top.z = topZ;

		//
		// Determine whether this ladder is "dangling" or not
		// "Dangling" ladders are too high to go up
		//
		ladder->m_isDangling = false;
		if ( ladder->m_bottomArea )
		{
			Vector bottomSpot;
			ladder->m_bottomArea->GetClosestPointOnArea( &ladder->m_bottom, &bottomSpot );
			if ( ladder->m_bottom.z - bottomSpot.z > HumanHeight )
				ladder->m_isDangling = true;
		}

		// add ladder to global list
		TheNavLadderList.push_back( ladder );

		entity = UTIL_FindEntityByClassname( entity, "func_ladder" );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Mark all areas that require a jump to get through them.
 * This currently relies on jump areas having extreme slope.
 */

bool CNavArea::IsEdge( NavDirType dir ) const
{
	for ( NavConnectList::const_iterator it = m_connect[dir].begin(); it != m_connect[dir].end(); ++it )
	{
		const NavConnect connect = *it;

		if ( connect.area->IsConnected( this, OppositeDirection( dir ) ) )
			return false;
	}


void CNavArea::RaiseCorner( NavCornerType corner, int amount )
{
	if ( corner == NUM_CORNERS )
	{
		m_extent.lo.z += amount;
		m_extent.hi.z += amount;
		m_neZ += amount;
		m_swZ += amount;
	}
	else
	{
		switch ( corner )
		{
		case NORTH_WEST:
			m_extent.lo.z += amount;
			break;
		case NORTH_EAST:
			m_neZ += amount;
			break;
		case SOUTH_WEST:
			m_swZ += amount;
			break;
		case SOUTH_EAST:
			m_extent.hi.z += amount;
			break;
		}
	}

	m_center.x = ( m_extent.lo.x + m_extent.hi.x ) / 2.0f;
	m_center.y = ( m_extent.lo.y + m_extent.hi.y ) / 2.0f;
	m_center.z = ( m_extent.lo.z + m_extent.hi.z ) / 2.0f;
}

/**
 * Flood fills all areas with current place

