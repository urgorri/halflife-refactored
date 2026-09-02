// nav_area_geometry.cpp
// AI Navigation areas geometry and elevation calculations
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )

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

#include "cs_bot.h"
#include "cs_bot_manager.h"
#include "hostage.h"

#include "nav.h"
#include "nav_node.h"
#include "nav_area.h"

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given position is within 2D extents of this area
 */
bool CNavArea::IsOverlapping( const Vector *pos ) const
{
	if ( pos->x >= m_extent.lo.x && pos->x <= m_extent.hi.x &&
	     pos->y >= m_extent.lo.y && pos->y <= m_extent.hi.y )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our 2D extents
 */
bool CNavArea::IsOverlapping( const CNavArea *area ) const
{
	if ( area->m_extent.lo.x < m_extent.hi.x && area->m_extent.hi.x > m_extent.lo.x &&
	     area->m_extent.lo.y < m_extent.hi.y && area->m_extent.hi.y > m_extent.lo.y )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our X extent
 */
bool CNavArea::IsOverlappingX( const CNavArea *area ) const
{
	if ( area->m_extent.lo.x < m_extent.hi.x && area->m_extent.hi.x > m_extent.lo.x )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if 'area' overlaps our Y extent
 */
bool CNavArea::IsOverlappingY( const CNavArea *area ) const
{
	if ( area->m_extent.lo.y < m_extent.hi.y && area->m_extent.hi.y > m_extent.lo.y )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if given point is on or above this area, but no others
 */
bool CNavArea::Contains( const Vector *pos ) const
{
	if ( !IsOverlapping( pos ) )
		return false;

	float ourZ = GetZ( pos );

	if ( ourZ > pos->z )
		return false;

	for ( NavAreaList::const_iterator iter = m_overlapList.begin(); iter != m_overlapList.end(); ++iter )
	{
		const CNavArea *area = *iter;

		if ( area == this )
			continue;

		if ( !area->IsOverlapping( pos ) )
			continue;

		float theirZ = area->GetZ( pos );
		if ( theirZ > pos->z )
		{
			continue;
		}

		if ( theirZ > ourZ )
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this area and given area are approximately co-planar
 */
bool CNavArea::IsCoplanar( const CNavArea *area ) const
{
	Vector u, v;

	u.x = m_extent.hi.x - m_extent.lo.x;
	u.y = 0.0f;
	u.z = m_neZ - m_extent.lo.z;

	v.x = 0.0f;
	v.y = m_extent.hi.y - m_extent.lo.y;
	v.z = m_swZ - m_extent.lo.z;

	Vector normal = CrossProduct( u, v );
	normal.NormalizeInPlace();

	u.x = area->m_extent.hi.x - area->m_extent.lo.x;
	u.y = 0.0f;
	u.z = area->m_neZ - area->m_extent.lo.z;

	v.x = 0.0f;
	v.y = area->m_extent.hi.y - area->m_extent.lo.y;
	v.z = area->m_swZ - area->m_extent.lo.z;

	Vector otherNormal = CrossProduct( u, v );
	otherNormal.NormalizeInPlace();

	const float tolerance = 0.99f;
	if ( DotProduct( normal, otherNormal ) > tolerance )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return Z of area at (x,y) of 'pos'
 * Trilinear interpolation of Z values at quad edges.
 */
float CNavArea::GetZ( const Vector *pos ) const
{
	float dx = m_extent.hi.x - m_extent.lo.x;
	float dy = m_extent.hi.y - m_extent.lo.y;

	if ( dx == 0.0f || dy == 0.0f )
		return m_neZ;

	float u = ( pos->x - m_extent.lo.x ) / dx;
	float v = ( pos->y - m_extent.lo.y ) / dy;

	if ( u < 0.0f )
		u = 0.0f;
	else if ( u > 1.0f )
		u = 1.0f;

	if ( v < 0.0f )
		v = 0.0f;
	else if ( v > 1.0f )
		v = 1.0f;

	float northZ = m_extent.lo.z + u * ( m_neZ - m_extent.lo.z );
	float southZ = m_swZ + u * ( m_extent.hi.z - m_swZ );

	return northZ + v * ( southZ - northZ );
}

float CNavArea::GetZ( float x, float y ) const
{
	Vector pos( x, y, 0.0f );
	return GetZ( &pos );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return closest point to 'pos' on 'area'.
 */
void CNavArea::GetClosestPointOnArea( const Vector *pos, Vector *close ) const
{
	const Extent *extent = GetExtent();

	if ( pos->x < extent->lo.x )
	{
		if ( pos->y < extent->lo.y )
		{
			*close = extent->lo;
		}
		else if ( pos->y > extent->hi.y )
		{
			close->x = extent->lo.x;
			close->y = extent->hi.y;
		}
		else
		{
			close->x = extent->lo.x;
			close->y = pos->y;
		}
	}
	else if ( pos->x > extent->hi.x )
	{
		if ( pos->y < extent->lo.y )
		{
			close->x = extent->hi.x;
			close->y = extent->lo.y;
		}
		else if ( pos->y > extent->hi.y )
		{
			*close = extent->hi;
		}
		else
		{
			close->x = extent->hi.x;
			close->y = pos->y;
		}
	}
	else if ( pos->y < extent->lo.y )
	{
		close->x = pos->x;
		close->y = extent->lo.y;
	}
	else if ( pos->y > extent->hi.y )
	{
		close->x = pos->x;
		close->y = extent->hi.y;
	}
	else
	{
		*close = *pos;
	}

	close->z = GetZ( close );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return shortest distance squared between point and this area
 */
float CNavArea::GetDistanceSquaredToPoint( const Vector *pos ) const
{
	const Extent *extent = GetExtent();

	if ( pos->x < extent->lo.x )
	{
		if ( pos->y < extent->lo.y )
		{
			return ( extent->lo - *pos ).LengthSquared();
		}
		else if ( pos->y > extent->hi.y )
		{
			Vector d;
			d.x = extent->lo.x - pos->x;
			d.y = extent->hi.y - pos->y;
			d.z = m_swZ - pos->z;
			return d.LengthSquared();
		}
		else
		{
			float d = extent->lo.x - pos->x;
			return d * d;
		}
	}
	else if ( pos->x > extent->hi.x )
	{
		if ( pos->y < extent->lo.y )
		{
			Vector d;
			d.x = extent->hi.x - pos->x;
			d.y = extent->lo.y - pos->y;
			d.z = m_neZ - pos->z;
			return d.LengthSquared();
		}
		else if ( pos->y > extent->hi.y )
		{
			return ( extent->hi - *pos ).LengthSquared();
		}
		else
		{
			float d = pos->z - extent->hi.x;
			return d * d;
		}
	}
	else if ( pos->y < extent->lo.y )
	{
		float d = extent->lo.y - pos->y;
		return d * d;
	}
	else if ( pos->y > extent->hi.y )
	{
		float d = pos->y - extent->hi.y;
		return d * d;
	}
	else
	{
		float z = GetZ( pos );
		float d = z - pos->z;
		return d * d;
	}
}

//--------------------------------------------------------------------------------------------------------------
CNavArea *CNavArea::GetRandomAdjacentArea( NavDirType dir ) const
{
	int count = m_connect[dir].size();
	int which = RANDOM_LONG( 0, count - 1 );

	int i = 0;
	NavConnectList::const_iterator iter;
	for ( iter = m_connect[dir].begin(); iter != m_connect[dir].end(); ++iter )
	{
		if ( i == which )
			return ( *iter ).area;

		i++;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute "portal" between to adjacent areas.
 */
void CNavArea::ComputePortal( const CNavArea *to, NavDirType dir, Vector *center, float *halfWidth ) const
{
	if ( dir == NORTH || dir == SOUTH )
	{
		if ( dir == NORTH )
			center->y = m_extent.lo.y;
		else
			center->y = m_extent.hi.y;

		float left  = max( m_extent.lo.x, to->m_extent.lo.x );
		float right = min( m_extent.hi.x, to->m_extent.hi.x );

		if ( left < m_extent.lo.x )
			left = m_extent.lo.x;
		else if ( left > m_extent.hi.x )
			left = m_extent.hi.x;

		if ( right < m_extent.lo.x )
			right = m_extent.lo.x;
		else if ( right > m_extent.hi.x )
			right = m_extent.hi.x;

		center->x  = ( left + right ) / 2.0f;
		*halfWidth = ( right - left ) / 2.0f;
	}
	else
	{
		if ( dir == WEST )
			center->x = m_extent.lo.x;
		else
			center->x = m_extent.hi.x;

		float top    = max( m_extent.lo.y, to->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, to->m_extent.hi.y );

		if ( top < m_extent.lo.y )
			top = m_extent.lo.y;
		else if ( top > m_extent.hi.y )
			top = m_extent.hi.y;

		if ( bottom < m_extent.lo.y )
			bottom = m_extent.lo.y;
		else if ( bottom > m_extent.hi.y )
			bottom = m_extent.hi.y;

		center->y  = ( top + bottom ) / 2.0f;
		*halfWidth = ( bottom - top ) / 2.0f;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute closest point within the "portal" between to adjacent areas.
 */
void CNavArea::ComputeClosestPointInPortal( const CNavArea *to, NavDirType dir, const Vector *fromPos, Vector *closePos ) const
{
	const float margin = GenerationStepSize / 2.0f;

	if ( dir == NORTH || dir == SOUTH )
	{
		if ( dir == NORTH )
			closePos->y = m_extent.lo.y;
		else
			closePos->y = m_extent.hi.y;

		float left  = max( m_extent.lo.x, to->m_extent.lo.x );
		float right = min( m_extent.hi.x, to->m_extent.hi.x );

		if ( left < m_extent.lo.x )
			left = m_extent.lo.x;
		else if ( left > m_extent.hi.x )
			left = m_extent.hi.x;

		if ( right < m_extent.lo.x )
			right = m_extent.lo.x;
		else if ( right > m_extent.hi.x )
			right = m_extent.hi.x;

		const float leftMargin  = ( to->IsEdge( WEST ) ) ? ( left + margin ) : left;
		const float rightMargin = ( to->IsEdge( EAST ) ) ? ( right - margin ) : right;

		if ( fromPos->x < leftMargin )
			closePos->x = leftMargin;
		else if ( fromPos->x > rightMargin )
			closePos->x = rightMargin;
		else
			closePos->x = fromPos->x;
	}
	else
	{
		if ( dir == WEST )
			closePos->x = m_extent.lo.x;
		else
			closePos->x = m_extent.hi.x;

		float top    = max( m_extent.lo.y, to->m_extent.lo.y );
		float bottom = min( m_extent.hi.y, to->m_extent.hi.y );

		if ( top < m_extent.lo.y )
			top = m_extent.lo.y;
		else if ( top > m_extent.hi.y )
			top = m_extent.hi.y;

		if ( bottom < m_extent.lo.y )
			bottom = m_extent.lo.y;
		else if ( bottom > m_extent.hi.y )
			bottom = m_extent.hi.y;

		const float topMargin    = ( to->IsEdge( NORTH ) ) ? ( top + margin ) : top;
		const float bottomMargin = ( to->IsEdge( SOUTH ) ) ? ( bottom - margin ) : bottom;

		if ( fromPos->y < topMargin )
			closePos->y = topMargin;
		else if ( fromPos->y > bottomMargin )
			closePos->y = bottomMargin;
		else
			closePos->y = fromPos->y;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the coordinates of the area's corner.
 */
const Vector *CNavArea::GetCorner( NavCornerType corner ) const
{
	static Vector pos;

	switch ( corner )
	{
	case NORTH_WEST:
		return &m_extent.lo;

	case NORTH_EAST:
		pos.x = m_extent.hi.x;
		pos.y = m_extent.lo.y;
		pos.z = m_neZ;
		return &pos;

	case SOUTH_WEST:
		pos.x = m_extent.lo.x;
		pos.y = m_extent.hi.y;
		pos.z = m_swZ;
		return &pos;

	case SOUTH_EAST:
		return &m_extent.hi;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the ground height below this point in "height".
 */
bool GetGroundHeight( const Vector *pos, float *height, Vector *normal )
{
	Vector to;
	to.x = pos->x;
	to.y = pos->y;
	to.z = pos->z - 9999.9f;

	float offset;
	Vector from;
	TraceResult result;
	edict_t *ignore = NULL;
	float ground    = 0.0f;

	const float maxOffset = 100.0f;
	const float inc       = 10.0f;

#define MAX_GROUND_LAYERS 16
	struct GroundLayerInfo
	{
		float ground;
		Vector normal;
	} layer[MAX_GROUND_LAYERS];
	int layerCount = 0;

	for ( offset = 1.0f; offset < maxOffset; offset += inc )
	{
		from = *pos + Vector( 0, 0, offset );

		UTIL_TraceLine( from, to, ignore_monsters, dont_ignore_glass, ignore, &result );

		if ( result.pHit )
		{
			if ( FClassnameIs( VARS( result.pHit ), "func_door" ) ||
			     FClassnameIs( VARS( result.pHit ), "func_door_rotating" ) ||
			     ( FClassnameIs( VARS( result.pHit ), "func_breakable" ) && VARS( result.pHit )->takedamage == DAMAGE_YES ) )
			{
				ignore = result.pHit;
				continue;
			}
		}

		if ( result.fStartSolid == false )
		{
			if ( layerCount == 0 || result.vecEndPos.z > layer[layerCount - 1].ground )
			{
				layer[layerCount].ground = result.vecEndPos.z;
				layer[layerCount].normal = result.vecPlaneNormal;
				++layerCount;

				if ( layerCount == MAX_GROUND_LAYERS )
					break;
			}
		}
	}

	if ( layerCount == 0 )
		return false;

	int i;
	for ( i = 0; i < layerCount - 1; ++i )
	{
		if ( layer[i + 1].ground - layer[i].ground >= HalfHumanHeight )
			break;
	}

	*height = layer[i].ground;

	if ( normal )
		*normal = layer[i].normal;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the "simple" ground height below this point in "height".
 */
bool GetSimpleGroundHeight( const Vector *pos, float *height, Vector *normal )
{
	Vector to;
	to.x = pos->x;
	to.y = pos->y;
	to.z = pos->z - 9999.9f;

	TraceResult result;

	UTIL_TraceLine( *pos, to, ignore_monsters, dont_ignore_glass, NULL, &result );

	if ( result.fStartSolid )
		return false;

	*height = result.vecEndPos.z;

	if ( normal )
		*normal = result.vecPlaneNormal;

	return true;
}
