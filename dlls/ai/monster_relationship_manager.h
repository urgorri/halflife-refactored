/***
 *
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

// Forward declarations
class CBaseEntity;

// Ensure relationship constants are available
#ifndef R_AL
#define R_AL -2 // (ALLY) pals
#endif
#ifndef R_FR
#define R_FR -1 // (FEAR) will run
#endif
#ifndef R_NO
#define R_NO 0  // (NO RELATIONSHIP) disregard
#endif
#ifndef R_DL
#define R_DL 1  // (DISLIKE) will attack
#endif
#ifndef R_HT
#define R_HT 2  // (HATE) will attack this character instead of any visible DISLIKEd characters
#endif
#ifndef R_NM
#define R_NM 3  // (NEMESIS) monster will ALWAYS attack its nemesis
#endif

class MonsterRelationshipManager
{
  public:
	static constexpr int NUM_DEFAULT_CLASSES = 14;

	// Singleton instance access
	static MonsterRelationshipManager &GetInstance();

	// Query relationship between two monster classes with bounds checking
	int GetRelationship( int iClassFrom, int iClassTo ) const;

	// Query relationship between two entity instances (with pointer validation and entity overrides)
	int GetRelationship( CBaseEntity *pActor, CBaseEntity *pTarget ) const;

	// Class-to-class runtime relationship overrides
	void SetRelationship( int iClassFrom, int iClassTo, int iRelationship );
	void RemoveRelationship( int iClassFrom, int iClassTo );
	bool HasRelationshipOverride( int iClassFrom, int iClassTo ) const;

	// Entity-specific runtime relationship overrides
	void SetEntityRelationship( CBaseEntity *pActor, CBaseEntity *pTarget, int iRelationship );
	void SetEntityClassRelationship( CBaseEntity *pActor, int iTargetClass, int iRelationship );
	void ClearEntityRelationships( CBaseEntity *pActor );
	void ClearAllEntityRelationships();

	// Clear all overrides and restore default matrix
	void ClearOverrides();
	void ResetToDefaults();

	// Class validation
	static bool IsValidClass( int iClass );

	// Static facade convenience methods
	static int QueryRelationship( int iClassFrom, int iClassTo );
	static int QueryRelationship( CBaseEntity *pActor, CBaseEntity *pTarget );
	static void OverrideRelationship( int iClassFrom, int iClassTo, int iRelationship );
	static void Reset();

  private:
	MonsterRelationshipManager();
	void InitDefaultTable();

	static uint64_t MakeClassKey( int iClassFrom, int iClassTo )
	{
		return ( static_cast<uint64_t>( static_cast<uint32_t>( iClassFrom ) ) << 32 ) |
		       static_cast<uint32_t>( iClassTo );
	}

	static uint64_t MakeEntityKey( const CBaseEntity *pActor, const CBaseEntity *pTarget )
	{
		return ( static_cast<uint64_t>( reinterpret_cast<uintptr_t>( pActor ) ) << 32 ) |
		       static_cast<uint64_t>( reinterpret_cast<uintptr_t>( pTarget ) );
	}

	static uint64_t MakeEntityClassKey( const CBaseEntity *pActor, int iTargetClass )
	{
		return ( static_cast<uint64_t>( reinterpret_cast<uintptr_t>( pActor ) ) << 32 ) |
		       static_cast<uint32_t>( iTargetClass );
	}

	// Canonical 14x14 baseline relationship matrix
	int m_defaultTable[NUM_DEFAULT_CLASSES][NUM_DEFAULT_CLASSES];

	// Dynamic class-level relationship overrides
	std::unordered_map<uint64_t, int> m_classOverrides;

	// Dynamic entity-level relationship overrides
	std::unordered_map<uint64_t, int> m_entityOverrides;
	std::unordered_map<uint64_t, int> m_entityClassOverrides;
};
