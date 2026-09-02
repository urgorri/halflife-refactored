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

static NavAreaList goodSizedAreaList;

static void buildGoodSizedList( void )
{
	const float minSize = 200.0f; // 150

	NavAreaList::iterator iter;
	for ( iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		// skip the small areas
		const Extent *extent = area->GetExtent();
		if ( extent->SizeX() < minSize || extent->SizeY() < minSize )
			continue;

		goodSizedAreaList.push_back( area );
	}
}

//--------------------------------------------------------------------------------------------------------------

HidingSpotList TheHidingSpotList;
unsigned int HidingSpot::m_nextID       = 1;
unsigned int HidingSpot::m_masterMarker = 0;

void DestroyHidingSpots( void )
{
	// remove all hiding spot references from the nav areas
	for ( NavAreaList::iterator areaIter = TheNavAreaList.begin(); areaIter != TheNavAreaList.end(); ++areaIter )
	{
		CNavArea *area = *areaIter;

		area->m_hidingSpotList.clear();
	}

	HidingSpot::m_nextID = 0;

	// free all the HidingSpots
	for ( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
		delete *iter;

	TheHidingSpotList.clear();
}

/**
 * For use when loading from a file
 */
HidingSpot::HidingSpot( void )
{
	m_pos   = Vector( 0, 0, 0 );
	m_id    = 0;
	m_flags = 0;

	TheHidingSpotList.push_back( this );
}

/**
 * For use when generating - assigns unique ID
 */
HidingSpot::HidingSpot( const Vector *pos, unsigned char flags )
{
	m_pos   = *pos;
	m_id    = m_nextID++;
	m_flags = flags;

	TheHidingSpotList.push_back( this );
}

void HidingSpot::Save( int fd, unsigned int version ) const
{
	_write( fd, &m_id, sizeof( unsigned int ) );
	_write( fd, &m_pos, 3 * sizeof( float ) );
	_write( fd, &m_flags, sizeof( unsigned char ) );
}

void HidingSpot::Load( SteamFile *file, unsigned int version )
{
	file->Read( &m_id, sizeof( unsigned int ) );
	file->Read( &m_pos, 3 * sizeof( float ) );
	file->Read( &m_flags, sizeof( unsigned char ) );

	// update next ID to avoid ID collisions by later spots
	if ( m_id >= m_nextID )
		m_nextID = m_id + 1;
}

/**
 * Given a HidingSpot ID, return the associated HidingSpot
 */
HidingSpot *GetHidingSpotByID( unsigned int id )
{
	for ( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
	{
		HidingSpot *spot = *iter;

		if ( spot->GetID() == id )
			return spot;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * To keep constructors consistent
 */

void ApproachAreaAnalysisPrep( void )
{
	// collect "good-sized" areas for computing approach areas
	buildGoodSizedList();
}

//--------------------------------------------------------------------------------------------------------------
void CleanupApproachAreaAnalysisPrep( void )
{
	goodSizedAreaList.clear();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Destroy ladder representations
 */

bool CNavArea::IsHidingSpotCollision( const Vector *pos ) const
{
	const float collisionRange = 30.0f;

	for ( HidingSpotList::const_iterator iter = m_hidingSpotList.begin(); iter != m_hidingSpotList.end(); ++iter )
	{
		const HidingSpot *spot = *iter;

		if ( ( *spot->GetPosition() - *pos ).IsLengthLessThan( collisionRange ) )
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
bool IsHidingSpotInCover( const Vector *spot )
{
	int coverCount = 0;
	TraceResult result;

	Vector from = *spot;
	from.z += HalfHumanHeight;

	Vector to;

	// if we are crouched underneath something, that counts as good cover
	to = from + Vector( 0, 0, 20.0f );
	UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );
	if ( result.flFraction != 1.0f )
		return true;

	const float coverRange = 100.0f;
	const float inc        = M_PI / 8.0f;

	for ( float angle = 0.0f; angle < 2.0f * M_PI; angle += inc )
	{
		to = from + Vector( coverRange * cos( angle ), coverRange * sin( angle ), HalfHumanHeight );

		UTIL_TraceLine( from, to, ignore_monsters, NULL, &result );

		// if traceline hit something, it hit "cover"
		if ( result.flFraction != 1.0f )
			++coverCount;
	}

	// if more than half of the circle has no cover, the spot is not "in cover"
	const int halfCover = 8;
	if ( coverCount < halfCover )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Analyze local area neighborhood to find "hiding spots" for this area
 */
void CNavArea::ComputeHidingSpots( void )
{
	struct
	{
		float lo, hi;
	} extent;

	// "jump areas" cannot have hiding spots
	if ( GetAttributes() & NAV_JUMP )
		return;

	int cornerCount[NUM_CORNERS];
	for ( int i = 0; i < NUM_CORNERS; ++i )
		cornerCount[i] = 0;

	const float cornerSize = 20.0f;

	// for each direction, find extents of adjacent areas along the wall
	for ( int d = 0; d < NUM_DIRECTIONS; ++d )
	{
		extent.lo = 999999.9f;
		extent.hi = -999999.9f;

		bool isHoriz = ( d == NORTH || d == SOUTH ) ? true : false;

		for ( NavConnectList::iterator iter = m_connect[d].begin(); iter != m_connect[d].end(); ++iter )
		{
			NavConnect connect = *iter;

			// if connection is only one-way, it's a "jump down" connection (ie: a discontinuity that may mean cover)
			// ignore it
			if ( connect.area->IsConnected( this, OppositeDirection( static_cast< NavDirType >( d ) ) ) == false )
				continue;

			// ignore jump areas
			if ( connect.area->GetAttributes() & NAV_JUMP )
				continue;

			if ( isHoriz )
			{
				if ( connect.area->m_extent.lo.x < extent.lo )
					extent.lo = connect.area->m_extent.lo.x;

				if ( connect.area->m_extent.hi.x > extent.hi )
					extent.hi = connect.area->m_extent.hi.x;
			}
			else
			{
				if ( connect.area->m_extent.lo.y < extent.lo )
					extent.lo = connect.area->m_extent.lo.y;

				if ( connect.area->m_extent.hi.y > extent.hi )
					extent.hi = connect.area->m_extent.hi.y;
			}
		}

		switch ( d )
		{
		case NORTH:
			if ( extent.lo - m_extent.lo.x >= cornerSize )
				++cornerCount[NORTH_WEST];

			if ( m_extent.hi.x - extent.hi >= cornerSize )
				++cornerCount[NORTH_EAST];
			break;

		case SOUTH:
			if ( extent.lo - m_extent.lo.x >= cornerSize )
				++cornerCount[SOUTH_WEST];

			if ( m_extent.hi.x - extent.hi >= cornerSize )
				++cornerCount[SOUTH_EAST];
			break;

		case EAST:
			if ( extent.lo - m_extent.lo.y >= cornerSize )
				++cornerCount[NORTH_EAST];

			if ( m_extent.hi.y - extent.hi >= cornerSize )
				++cornerCount[SOUTH_EAST];
			break;

		case WEST:
			if ( extent.lo - m_extent.lo.y >= cornerSize )
				++cornerCount[NORTH_WEST];

			if ( m_extent.hi.y - extent.hi >= cornerSize )
				++cornerCount[SOUTH_WEST];
			break;
		}
	}

	// if a corner count is 2, then it really is a corner (walls on both sides)
	float offset = 12.5f;

	if ( cornerCount[NORTH_WEST] == 2 )
	{
		Vector pos = *GetCorner( NORTH_WEST ) + Vector( offset, offset, 0.0f );

		m_hidingSpotList.push_back( new HidingSpot( &pos, ( IsHidingSpotInCover( &pos ) ) ? HidingSpot::IN_COVER : 0 ) );
	}

	if ( cornerCount[NORTH_EAST] == 2 )
	{
		Vector pos = *GetCorner( NORTH_EAST ) + Vector( -offset, offset, 0.0f );
		if ( !IsHidingSpotCollision( &pos ) )
			m_hidingSpotList.push_back( new HidingSpot( &pos, ( IsHidingSpotInCover( &pos ) ) ? HidingSpot::IN_COVER : 0 ) );
	}

	if ( cornerCount[SOUTH_WEST] == 2 )
	{
		Vector pos = *GetCorner( SOUTH_WEST ) + Vector( offset, -offset, 0.0f );
		if ( !IsHidingSpotCollision( &pos ) )
			m_hidingSpotList.push_back( new HidingSpot( &pos, ( IsHidingSpotInCover( &pos ) ) ? HidingSpot::IN_COVER : 0 ) );
	}

	if ( cornerCount[SOUTH_EAST] == 2 )
	{
		Vector pos = *GetCorner( SOUTH_EAST ) + Vector( -offset, -offset, 0.0f );
		if ( !IsHidingSpotCollision( &pos ) )
			m_hidingSpotList.push_back( new HidingSpot( &pos, ( IsHidingSpotInCover( &pos ) ) ? HidingSpot::IN_COVER : 0 ) );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Determine how much walkable area we can see from the spot, and how far away we can see.
 */
void ClassifySniperSpot( HidingSpot *spot )
{
	Vector eye = *spot->GetPosition() + Vector( 0, 0, HalfHumanHeight ); // assume we are crouching
	Vector walkable;
	TraceResult result;

	Extent sniperExtent;
	float farthestRangeSq        = 0.0f;
	const float minSniperRangeSq = 1000.0f * 1000.0f;
	bool found                   = false;

	for ( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		const Extent *extent = area->GetExtent();

		// scan this area
		for ( walkable.y = extent->lo.y + GenerationStepSize / 2.0f; walkable.y < extent->hi.y; walkable.y += GenerationStepSize )
		{
			for ( walkable.x = extent->lo.x + GenerationStepSize / 2.0f; walkable.x < extent->hi.x; walkable.x += GenerationStepSize )
			{
				walkable.z = area->GetZ( &walkable ) + HalfHumanHeight;

				// check line of sight
				UTIL_TraceLine( eye, walkable, ignore_monsters, ignore_glass, NULL, &result );

				if ( result.flFraction == 1.0f && !result.fStartSolid )
				{
					// can see this spot

					// keep track of how far we can see
					float rangeSq = ( eye - walkable ).LengthSquared();
					if ( rangeSq > farthestRangeSq )
					{
						farthestRangeSq = rangeSq;

						if ( rangeSq >= minSniperRangeSq )
						{
							// this is a sniper spot
							// determine how good of a sniper spot it is by keeping track of the snipable area
							if ( found )
							{
								if ( walkable.x < sniperExtent.lo.x )
									sniperExtent.lo.x = walkable.x;
								if ( walkable.x > sniperExtent.hi.x )
									sniperExtent.hi.x = walkable.x;

								if ( walkable.y < sniperExtent.lo.y )
									sniperExtent.lo.y = walkable.y;
								if ( walkable.y > sniperExtent.hi.y )
									sniperExtent.hi.y = walkable.y;
							}
							else
							{
								sniperExtent.lo = walkable;
								sniperExtent.hi = walkable;
								found           = true;
							}
						}
					}
				}
			}
		}
	}

	if ( found )
	{
		// if we can see a large snipable area, it is an "ideal" spot
		float snipableArea = sniperExtent.Area();

		const float minIdealSniperArea = 200.0f * 200.0f;
		const float longSniperRangeSq  = 1500.0f * 1500.0f;

		if ( snipableArea >= minIdealSniperArea || farthestRangeSq >= longSniperRangeSq )
			spot->SetFlags( HidingSpot::IDEAL_SNIPER_SPOT );
		else
			spot->SetFlags( HidingSpot::GOOD_SNIPER_SPOT );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Analyze local area neighborhood to find "sniper spots" for this area
 */
void CNavArea::ComputeSniperSpots( void )
{
	if ( cv_bot_quicksave.value > 0.0f )
		return;

	for ( HidingSpotList::iterator iter = m_hidingSpotList.begin(); iter != m_hidingSpotList.end(); ++iter )
	{
		HidingSpot *spot = *iter;

		ClassifySniperSpot( spot );
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Given the areas we are moving between, return the spots we will encounter
 */
SpotEncounter *CNavArea::GetSpotEncounter( const CNavArea *from, const CNavArea *to )
{
	if ( from && to )
	{
		SpotEncounter *e;

		for ( SpotEncounterList::iterator iter = m_spotEncounterList.begin(); iter != m_spotEncounterList.end(); ++iter )
		{
			e = &( *iter );

			if ( e->from.area == from && e->to.area == to )
				return e;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Add spot encounter data when moving from area to area
 */
void CNavArea::AddSpotEncounters( const CNavArea *from, NavDirType fromDir, const CNavArea *to, NavDirType toDir )
{
	SpotEncounter e;

	e.from.area = const_cast< CNavArea * >( from );
	e.fromDir   = fromDir;

	e.to.area = const_cast< CNavArea * >( to );
	e.toDir   = toDir;

	float halfWidth;
	ComputePortal( to, toDir, &e.path.to, &halfWidth );
	ComputePortal( from, fromDir, &e.path.from, &halfWidth );

	const float eyeHeight = HalfHumanHeight;
	e.path.from.z         = from->GetZ( &e.path.from ) + eyeHeight;
	e.path.to.z           = to->GetZ( &e.path.to ) + eyeHeight;

	// step along ray and track which spots can be seen
	Vector dir   = e.path.to - e.path.from;
	float length = dir.NormalizeInPlace();

	// create unique marker to flag used spots
	HidingSpot::ChangeMasterMarker();

	const float stepSize     = 25.0f;   // 50
	const float seeSpotRange = 2000.0f; // 3000
	TraceResult result;

	Vector eye, delta;
	HidingSpot *spot;
	SpotOrder spotOrder;

	// step along path thru this area
	bool done = false;
	for ( float along = 0.0f; !done; along += stepSize )
	{
		// make sure we check the endpoint of the path segment
		if ( along >= length )
		{
			along = length;
			done  = true;
		}

		// move the eyepoint along the path segment
		eye = e.path.from + along * dir;

		// check each hiding spot for visibility
		for ( HidingSpotList::iterator iter = TheHidingSpotList.begin(); iter != TheHidingSpotList.end(); ++iter )
		{
			spot = *iter;

			// only look at spots with cover (others are out in the open and easily seen)
			if ( !spot->HasGoodCover() )
				continue;

			if ( spot->IsMarked() )
				continue;

			const Vector *spotPos = spot->GetPosition();

			delta.x = spotPos->x - eye.x;
			delta.y = spotPos->y - eye.y;
			delta.z = ( spotPos->z + eyeHeight ) - eye.z;

			// check if in range
			if ( delta.IsLengthGreaterThan( seeSpotRange ) )
				continue;

			// check if we have LOS
			UTIL_TraceLine( eye, Vector( spotPos->x, spotPos->y, spotPos->z + HalfHumanHeight ), ignore_monsters, ignore_glass, NULL, &result );
			if ( result.flFraction != 1.0f )
				continue;

			// if spot is in front of us along our path, ignore it
			delta.NormalizeInPlace();
			float dot = DotProduct( dir, delta );
			if ( dot < 0.7071f && dot > -0.7071f )
			{
				// we only want to keep spots that BECOME visible as we walk past them
				// therefore, skip ALL visible spots at the start of the path segment
				if ( along > 0.0f )
				{
					// add spot to encounter
					spotOrder.spot = spot;
					spotOrder.t    = along / length;
					e.spotList.push_back( spotOrder );
				}
			}

			// mark spot as encountered
			spot->Mark();
		}
	}

	// add encounter to list
	m_spotEncounterList.push_back( e );
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Compute "spot encounter" data. This is an ordered list of spots to look at
 * for each possible path thru a nav area.
 */
void CNavArea::ComputeSpotEncounters( void )
{
	m_spotEncounterList.clear();

	if ( cv_bot_quicksave.value > 0.0f )
		return;

	// for each adjacent area
	for ( int fromDir = 0; fromDir < NUM_DIRECTIONS; ++fromDir )
	{
		for ( NavConnectList::iterator fromIter = m_connect[fromDir].begin(); fromIter != m_connect[fromDir].end(); ++fromIter )
		{
			NavConnect *fromCon = &( *fromIter );

			// compute encounter data for path to each adjacent area
			for ( int toDir = 0; toDir < NUM_DIRECTIONS; ++toDir )
			{
				for ( NavConnectList::iterator toIter = m_connect[toDir].begin(); toIter != m_connect[toDir].end(); ++toIter )
				{
					NavConnect *toCon = &( *toIter );

					if ( toCon == fromCon )
						continue;

					// just do our direction, as we'll loop around for other direction
					AddSpotEncounters( fromCon->area, (NavDirType)fromDir, toCon->area, (NavDirType)toDir );
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Decay the danger values
 */
void CNavArea::DecayDanger( void )
{
	// one kill == 1.0, which we will forget about in two minutes
	const float decayRate = 1.0f / 120.0f;

	for ( int i = 0; i < MAX_AREA_TEAMS; ++i )
	{
		float deltaT      = gpGlobals->time - m_dangerTimestamp[i];
		float decayAmount = decayRate * deltaT;

		m_danger[i] -= decayAmount;
		if ( m_danger[i] < 0.0f )
			m_danger[i] = 0.0f;

		// update timestamp
		m_dangerTimestamp[i] = gpGlobals->time;
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Increase the danger of this area for the given team
 */
void CNavArea::IncreaseDanger( int teamID, float amount )
{
	// before we add the new value, decay what's there
	DecayDanger();

	m_danger[teamID] += amount;
	m_dangerTimestamp[teamID] = gpGlobals->time;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the danger of this area (decays over time)
 */
float CNavArea::GetDanger( int teamID )
{
	DecayDanger();
	return m_danger[teamID];
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Increase the danger of nav areas containing and near the given position
 */
void IncreaseDangerNearby( int teamID, float amount, CNavArea *startArea, const Vector *pos, float maxRadius )
{
	if ( startArea == NULL )
		return;

	CNavArea::MakeNewMarker();
	CNavArea::ClearSearchLists();

	startArea->AddToOpenList();
	startArea->SetTotalCost( 0.0f );
	startArea->Mark();
	startArea->IncreaseDanger( teamID, amount );

	while ( !CNavArea::IsOpenListEmpty() )
	{
		// get next area to check
		CNavArea *area = CNavArea::PopOpenList();

		// area has no hiding spots, explore adjacent areas
		for ( int dir = 0; dir < NUM_DIRECTIONS; ++dir )
		{
			int count = area->GetAdjacentCount( (NavDirType)dir );
			for ( int i = 0; i < count; ++i )
			{
				CNavArea *adjArea = area->GetAdjacentArea( (NavDirType)dir, i );

				if ( !adjArea->IsMarked() )
				{
					// compute distance from danger source
					float cost = ( *adjArea->GetCenter() - *pos ).Length();
					if ( cost <= maxRadius )
					{
						adjArea->AddToOpenList();
						adjArea->SetTotalCost( cost );
						adjArea->Mark();
						adjArea->IncreaseDanger( teamID, amount * cost / maxRadius );
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Show danger levels for debugging
 */
void DrawDanger( void )
{
	for ( NavAreaList::iterator iter = TheNavAreaList.begin(); iter != TheNavAreaList.end(); ++iter )
	{
		CNavArea *area = *iter;

		Vector center = *area->GetCenter();
		Vector top;
		center.z = area->GetZ( &center );

		float danger = area->GetDanger( 0 );
		if ( danger > 0.1f )
		{
			top.x = center.x;
			top.y = center.y;
			top.z = center.z + 10.0f * danger;
			UTIL_DrawBeamPoints( center, top, 3.0f, 255, 0, 0 );
		}

		danger = area->GetDanger( 1 );
		if ( danger > 0.1f )
		{
			top.x = center.x;
			top.y = center.y;
			top.z = center.z + 10.0f * danger;
			UTIL_DrawBeamPoints( center, top, 3.0f, 0, 0, 255 );
		}
	}
}

//--------------------------------------------------------------------------------------------------------------
/**
 * If a player is at the given spot, return true
 */
bool IsSpotOccupied( CBaseEntity *me, const Vector *pos )
{
	const float closeRange = 75.0f; // 50

	// is there a player in this spot
	float range;
	CBasePlayer *player = UTIL_GetClosestPlayer( pos, &range );

	if ( player != me )
	{
		if ( player && range < closeRange )
			return true;
	}

	// is there is a hostage in this spot
	if ( g_pHostages )
	{
		CHostage *hostage = g_pHostages->GetClosestHostage( *pos, &range );
		if ( hostage && hostage != me && range < closeRange )
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
class CollectHidingSpotsFunctor
{
  public:
	CollectHidingSpotsFunctor( CBaseEntity *me, const Vector *origin, float range, unsigned char flags, Place place = UNDEFINED_PLACE, bool useCrouchAreas = true )
	{
		m_me             = me;
		m_count          = 0;
		m_origin         = origin;
		m_range          = range;
		m_flags          = flags;
		m_place          = place;
		m_useCrouchAreas = useCrouchAreas;
	}

	enum
	{
		MAX_SPOTS = 256
	};

	bool operator()( CNavArea *area )
	{
		// if a place is specified, only consider hiding spots from areas in that place
		if ( m_place != UNDEFINED_PLACE && area->GetPlace() != m_place )
			return true;

		// collect all the hiding spots in this area
		const HidingSpotList *list = area->GetHidingSpotList();

		for ( HidingSpotList::const_iterator iter = list->begin(); iter != list->end() && m_count < MAX_SPOTS; ++iter )
		{
			const HidingSpot *spot = *iter;

			if ( m_useCrouchAreas == false )
			{
				CNavArea *area = TheNavAreaGrid.GetNavArea( spot->GetPosition() );
				if ( area && ( area->GetAttributes() & NAV_CROUCH ) )
					continue;
			}

			// make sure hiding spot is in range
			if ( m_range > 0.0f )
				if ( ( *spot->GetPosition() - *m_origin ).IsLengthGreaterThan( m_range ) )
					continue;

			// if a Player is using this hiding spot, don't consider it
			if ( IsSpotOccupied( m_me, spot->GetPosition() ) )
			{
				// player is in hiding spot
				/// @todo Check if player is moving or sitting still
				continue;
			}

			// only collect hiding spots with matching flags
			if ( m_flags & spot->GetFlags() )
			{
				m_hidingSpot[m_count++] = spot->GetPosition();
			}
		}

		// if we've filled up, stop searching
		if ( m_count == MAX_SPOTS )
			return false;

		return true;
	}

	/**
	 * Remove the spot at index "i"
	 */
	void RemoveSpot( int i )
	{
		if ( m_count == 0 )
			return;

		for ( int j = i + 1; j < m_count; ++j )
			m_hidingSpot[j - 1] = m_hidingSpot[j];

		--m_count;
	}

	CBaseEntity *m_me;
	const Vector *m_origin;
	float m_range;

	const Vector *m_hidingSpot[MAX_SPOTS];
	int m_count;

	unsigned char m_flags;
	Place m_place;
	bool m_useCrouchAreas;
};

/**
 * Do a breadth-first search to find a nearby hiding spot and return it.
 * Don't pick a hiding spot that a Player is currently occupying.
 * @todo Clean up this mess
 */
const Vector *FindNearbyHidingSpot( CBaseEntity *me, const Vector *pos, CNavArea *startArea, float maxRange, bool isSniper, bool useNearest )
{
	if ( startArea == NULL )
		return NULL;

	// collect set of nearby hiding spots
	if ( isSniper )
	{
		CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::IDEAL_SNIPER_SPOT );
		SearchSurroundingAreas( startArea, pos, collector, maxRange );

		if ( collector.m_count )
		{
			int which = RANDOM_LONG( 0, collector.m_count - 1 );
			return collector.m_hidingSpot[which];
		}
		else
		{
			// no ideal sniping spots, look for "good" sniping spots
			CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::GOOD_SNIPER_SPOT );
			SearchSurroundingAreas( startArea, pos, collector, maxRange );

			if ( collector.m_count )
			{
				int which = RANDOM_LONG( 0, collector.m_count - 1 );
				return collector.m_hidingSpot[which];
			}

			// no sniping spots at all.. fall through and pick a normal hiding spot
		}
	}

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, pos, maxRange, HidingSpot::IN_COVER );
	SearchSurroundingAreas( startArea, pos, collector, maxRange );

	if ( collector.m_count == 0 )
		return NULL;

	if ( useNearest )
	{
		// return closest hiding spot
		const Vector *closest = NULL;
		float closeRangeSq    = 9999999999.9f;
		for ( int i = 0; i < collector.m_count; ++i )
		{
			float rangeSq = ( *collector.m_hidingSpot[i] - *pos ).LengthSquared();
			if ( rangeSq < closeRangeSq )
			{
				closeRangeSq = rangeSq;
				closest      = collector.m_hidingSpot[i];
			}
		}

		return closest;
	}

	// select a hiding spot at random
	int which = RANDOM_LONG( 0, collector.m_count - 1 );
	return collector.m_hidingSpot[which];
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Select a random hiding spot among the nav areas that are tagged with the given place
 */
const Vector *FindRandomHidingSpot( CBaseEntity *me, Place place, bool isSniper )
{
	// collect set of nearby hiding spots
	if ( isSniper )
	{
		CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::IDEAL_SNIPER_SPOT, place );
		ForAllAreas( collector );

		if ( collector.m_count )
		{
			int which = RANDOM_LONG( 0, collector.m_count - 1 );
			return collector.m_hidingSpot[which];
		}
		else
		{
			// no ideal sniping spots, look for "good" sniping spots
			CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::GOOD_SNIPER_SPOT, place );
			ForAllAreas( collector );

			if ( collector.m_count )
			{
				int which = RANDOM_LONG( 0, collector.m_count - 1 );
				return collector.m_hidingSpot[which];
			}

			// no sniping spots at all.. fall through and pick a normal hiding spot
		}
	}

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, NULL, -1.0f, HidingSpot::IN_COVER, place );
	ForAllAreas( collector );

	if ( collector.m_count == 0 )
		return NULL;

	// select a hiding spot at random
	int which = RANDOM_LONG( 0, collector.m_count - 1 );
	return collector.m_hidingSpot[which];
}

//--------------------------------------------------------------------------------------------------------------------
/**
 * Return true if moving from "start" to "finish" will cross a player's line of fire.
 * The path from "start" to "finish" is assumed to be a straight line.
 * "start" and "finish" are assumed to be points on the ground.
 */
bool IsCrossingLineOfFire( const Vector &start, const Vector &finish, CBaseEntity *ignore, int ignoreTeam )
{
	for ( int p = 1; p <= gpGlobals->maxClients; ++p )
	{
		CBasePlayer *player = static_cast< CBasePlayer * >( UTIL_PlayerByIndex( p ) );

		if ( !IsEntityValid( player ) )
			continue;

		if ( player == ignore )
			continue;

		if ( !player->IsAlive() )
			continue;

		if ( ignoreTeam && player->m_iTeam == ignoreTeam )
			continue;

		// compute player's unit aiming vector
		UTIL_MakeVectors( player->pev->v_angle + player->pev->punchangle );

		const float longRange = 5000.0f;
		Vector playerTarget   = player->pev->origin + longRange * gpGlobals->v_forward;

		Vector result;
		if ( IsIntersecting2D( start, finish, player->pev->origin, playerTarget, &result ) )
		{
			// simple check to see if intersection lies in the Z range of the path
			float loZ, hiZ;

			if ( start.z < finish.z )
			{
				loZ = start.z;
				hiZ = finish.z;
			}
			else
			{
				loZ = finish.z;
				hiZ = start.z;
			}

			if ( result.z >= loZ && result.z <= hiZ + HumanHeight )
				return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------
/**
 * Select a nearby retreat spot.
 * Don't pick a hiding spot that a Player is currently occupying.
 * If "avoidTeam" is nonzero, avoid getting close to members of that team.
 */
const Vector *FindNearbyRetreatSpot( CBaseEntity *me, const Vector *start, CNavArea *startArea, float maxRange, int avoidTeam, bool useCrouchAreas )
{
	if ( startArea == NULL )
		return NULL;

	// collect hiding spots with decent "cover"
	CollectHidingSpotsFunctor collector( me, start, maxRange, HidingSpot::IN_COVER, UNDEFINED_PLACE, useCrouchAreas );
	SearchSurroundingAreas( startArea, start, collector, maxRange );

	if ( collector.m_count == 0 )
		return NULL;

	// find the closest unoccupied hiding spot that crosses the least lines of fire and has the best cover
	for ( int i = 0; i < collector.m_count; ++i )
	{
		// check if we would have to cross a line of fire to reach this hiding spot
		if ( IsCrossingLineOfFire( *start, *collector.m_hidingSpot[i], me ) )
		{
			collector.RemoveSpot( i );

			// back up a step, so iteration won't skip a spot
			--i;

			continue;
		}

		// check if there is someone on the avoidTeam near this hiding spot
		if ( avoidTeam )
		{
			float range;
			if ( UTIL_GetClosestPlayer( collector.m_hidingSpot[i], avoidTeam, &range ) )
			{
				const float dangerRange = 150.0f;
				if ( range < dangerRange )
				{
					// there is an avoidable player too near this spot - remove it
					collector.RemoveSpot( i );

					// back up a step, so iteration won't skip a spot
					--i;

					continue;
				}
			}
		}
	}

	if ( collector.m_count <= 0 )
		return NULL;

	// all remaining spots are ok - pick one at random
	int which = RANDOM_LONG( 0, collector.m_count - 1 );
	return collector.m_hidingSpot[which];
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return number of players with given teamID in this area (teamID == 0 means any/all)
 * @todo Keep pointers to contained Players to make this a zero-time query
 */

			else
				dist = ( *area->GetCenter() - *fromArea->GetCenter() ).Length();

			float cost = dist + fromArea->GetCostSoFar();

			return cost;
		}
	}
};

/**
 * Can we see this area?
 * For now, if we can see any corner, we can see the area
 * @todo Need to check LOS to more than the corners for large and/or long areas
 */
inline bool IsAreaVisible( const Vector *pos, const CNavArea *area )
{
	Vector corner;
	TraceResult result;

	for ( int c = 0; c < NUM_CORNERS; ++c )
	{
		corner = *area->GetCorner( (NavCornerType)c );
		corner.z += 0.75f * HumanHeight;

		UTIL_TraceLine( *pos, corner, ignore_monsters, NULL, &result );
		if ( result.flFraction == 1.0f )
		{
			// we can see this area
			return true;
		}
	}

	return false;
}

/**
 * Determine the set of "approach areas".
 * An approach area is an area representing a place where players
 * move into/out of our local neighborhood of areas.
 */
void CNavArea::ComputeApproachAreas( void )
{
	m_approachCount = 0;

	if ( cv_bot_quicksave.value > 0.0f )
		return;

	// use the center of the nav area as the "view" point
	Vector eye = m_center;
	if ( GetGroundHeight( &eye, &eye.z ) == false )
		return;

	// approximate eye position
	if ( GetAttributes() & NAV_CROUCH )
		eye.z += 0.9f * HalfHumanHeight;
	else
		eye.z += 0.9f * HumanHeight;

	enum
	{
		MAX_PATH_LENGTH = 256
	};
	CNavArea *path[MAX_PATH_LENGTH];

	//
	// In order to enumerate all of the approach areas, we need to
	// run the algorithm many times, once for each "far away" area
	// and keep the union of the approach area sets
	//
	NavAreaList::iterator iter;
	for ( iter = goodSizedAreaList.begin(); iter != goodSizedAreaList.end(); ++iter )
	{
		CNavArea *farArea = *iter;

		BlockedIDCount = 0;

		// if we can see 'farArea', try again - the whole point is to go "around the bend", so to speak
		if ( IsAreaVisible( &eye, farArea ) )
			continue;

		// make first path to far away area
		ApproachAreaCost cost;
		if ( NavAreaBuildPath( this, farArea, NULL, cost ) == false )
			continue;

		//
		// Keep building paths to farArea and blocking them off until we
		// cant path there any more.
		// As areas are blocked off, all exits will be enumerated.
		//
		while ( m_approachCount < MAX_APPROACH_AREAS )
		{
			// find number of areas on path
			int count = 0;
			CNavArea *area;
			for ( area = farArea; area; area = area->GetParent() )
				++count;

			if ( count > MAX_PATH_LENGTH )
				count = MAX_PATH_LENGTH;

			// build path in correct order - from eye outwards
			int i = count;
			for ( area = farArea; i && area; area = area->GetParent() )
				path[--i] = area;

			// traverse path to find first area we cannot see (skip the first area)
			for ( i = 1; i < count; ++i )
			{
				// if we see this area, continue on
				if ( IsAreaVisible( &eye, path[i] ) )
					continue;

				// we can't see this area.
				// mark this area as "blocked" and unusable by subsequent approach paths
				if ( BlockedIDCount == MAX_BLOCKED_AREAS )
				{
					CONSOLE_ECHO( "Overflow computing approach areas for area #%d.\n", m_id );
					return;
				}

				// if the area to be blocked is actually farArea, block the one just prior
				// (blocking farArea will cause all subsequent pathfinds to fail)
				int block = ( path[i] == farArea ) ? i - 1 : i;

				BlockedID[BlockedIDCount++] = path[block]->GetID();

				if ( block == 0 )
					break;

				// store new approach area if not already in set
				int a;
				for ( a = 0; a < m_approachCount; ++a )
					if ( m_approach[a].here.area == path[block - 1] )
						break;

				if ( a == m_approachCount )
				{
					m_approach[m_approachCount].prev.area = ( block >= 2 ) ? path[block - 2] : NULL;

					m_approach[m_approachCount].here.area     = path[block - 1];
					m_approach[m_approachCount].prevToHereHow = path[block - 1]->GetParentHow();

					m_approach[m_approachCount].next.area     = path[block];
					m_approach[m_approachCount].hereToNextHow = path[block]->GetParentHow();

					++m_approachCount;
				}

				// we are done with this path
				break;
			}

			// find another path to 'farArea'
			ApproachAreaCost cost;
			if ( NavAreaBuildPath( this, farArea, NULL, cost ) == false )
			{
				// can't find a path to 'farArea' means all exits have been already tested and blocked
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------

/**
 * The singleton for accessing the grid
 */
CNavAreaGrid TheNavAreaGrid;


