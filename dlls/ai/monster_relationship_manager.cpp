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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "ai/monsters.h"
#include "ai/monster_relationship_manager.h"

MonsterRelationshipManager &MonsterRelationshipManager::GetInstance()
{
	static MonsterRelationshipManager s_instance;
	return s_instance;
}

MonsterRelationshipManager::MonsterRelationshipManager()
{
	InitDefaultTable();
}

void MonsterRelationshipManager::InitDefaultTable()
{
	// Canonical Valve GoldSrc 14x14 monster relationship table
	const int canonicalTable[NUM_DEFAULT_CLASSES][NUM_DEFAULT_CLASSES] = {
		//   NONE   MACH   PLYR   HPASS  HMIL   AMIL   APASS  AMONST APREY  APRED  INSECT PLRALY PBWPN  ABWPN
		/*NONE*/          { R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO },
		/*MACHINE*/       { R_NO, R_NO, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_DL, R_DL },
		/*PLAYER*/        { R_NO, R_DL, R_NO, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_DL, R_DL },
		/*HUMANPASSIVE*/  { R_NO, R_NO, R_AL, R_AL, R_HT, R_FR, R_NO, R_HT, R_DL, R_FR, R_NO, R_AL, R_NO, R_NO },
		/*HUMANMILITARY*/ { R_NO, R_NO, R_HT, R_DL, R_NO, R_HT, R_DL, R_DL, R_DL, R_DL, R_NO, R_HT, R_NO, R_NO },
		/*ALIENMILITARY*/ { R_NO, R_DL, R_HT, R_DL, R_HT, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_DL, R_NO, R_NO },
		/*ALIENPASSIVE*/  { R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO },
		/*ALIENMONSTER*/  { R_NO, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO, R_NO, R_NO, R_DL, R_NO, R_NO },
		/*ALIENPREY*/     { R_NO, R_NO, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO, R_FR, R_NO, R_DL, R_NO, R_NO },
		/*ALIENPREDATOR*/ { R_NO, R_NO, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_HT, R_DL, R_NO, R_DL, R_NO, R_NO },
		/*INSECT*/        { R_FR, R_FR, R_FR, R_FR, R_FR, R_NO, R_FR, R_FR, R_FR, R_FR, R_NO, R_FR, R_NO, R_NO },
		/*PLAYERALLY*/    { R_NO, R_DL, R_AL, R_AL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_NO, R_NO, R_NO },
		/*PBIOWEAPON*/    { R_NO, R_NO, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_DL, R_NO, R_DL, R_NO, R_DL },
		/*ABIOWEAPON*/    { R_NO, R_NO, R_DL, R_DL, R_DL, R_AL, R_NO, R_DL, R_DL, R_NO, R_NO, R_DL, R_DL, R_NO }
	};

	for ( int i = 0; i < NUM_DEFAULT_CLASSES; ++i )
	{
		for ( int j = 0; j < NUM_DEFAULT_CLASSES; ++j )
		{
			m_defaultTable[i][j] = canonicalTable[i][j];
		}
	}
}

bool MonsterRelationshipManager::IsValidClass( int iClass )
{
	return iClass >= 0 && iClass < NUM_DEFAULT_CLASSES;
}

int MonsterRelationshipManager::GetRelationship( int iClassFrom, int iClassTo ) const
{
	// 1. Check for dynamic class override
	auto it = m_classOverrides.find( MakeClassKey( iClassFrom, iClassTo ) );
	if ( it != m_classOverrides.end() )
	{
		return it->second;
	}

	// 2. Validate bounds against canonical table
	if ( IsValidClass( iClassFrom ) && IsValidClass( iClassTo ) )
	{
		return m_defaultTable[iClassFrom][iClassTo];
	}

	// 3. Fallback for unconfigured / extended classes (e.g. CLASS_BARNACLE = 99, CLASS_VEHICLE = 14)
	return R_NO;
}

int MonsterRelationshipManager::GetRelationship( CBaseEntity *pActor, CBaseEntity *pTarget ) const
{
	if ( !pActor || !pTarget )
	{
		return R_NO;
	}

	// 1. Check entity-to-entity override
	auto itEntity = m_entityOverrides.find( MakeEntityKey( pActor, pTarget ) );
	if ( itEntity != m_entityOverrides.end() )
	{
		return itEntity->second;
	}

	// 2. Check entity-to-class override
	auto itClass = m_entityClassOverrides.find( MakeEntityClassKey( pActor, pTarget->Classify() ) );
	if ( itClass != m_entityClassOverrides.end() )
	{
		return itClass->second;
	}

	// 3. Evaluate by class
	return GetRelationship( pActor->Classify(), pTarget->Classify() );
}

void MonsterRelationshipManager::SetRelationship( int iClassFrom, int iClassTo, int iRelationship )
{
	m_classOverrides[MakeClassKey( iClassFrom, iClassTo )] = iRelationship;
}

void MonsterRelationshipManager::RemoveRelationship( int iClassFrom, int iClassTo )
{
	m_classOverrides.erase( MakeClassKey( iClassFrom, iClassTo ) );
}

bool MonsterRelationshipManager::HasRelationshipOverride( int iClassFrom, int iClassTo ) const
{
	return m_classOverrides.find( MakeClassKey( iClassFrom, iClassTo ) ) != m_classOverrides.end();
}

void MonsterRelationshipManager::SetEntityRelationship( CBaseEntity *pActor, CBaseEntity *pTarget, int iRelationship )
{
	if ( !pActor || !pTarget )
		return;

	m_entityOverrides[MakeEntityKey( pActor, pTarget )] = iRelationship;
}

void MonsterRelationshipManager::SetEntityClassRelationship( CBaseEntity *pActor, int iTargetClass, int iRelationship )
{
	if ( !pActor )
		return;

	m_entityClassOverrides[MakeEntityClassKey( pActor, iTargetClass )] = iRelationship;
}

void MonsterRelationshipManager::ClearEntityRelationships( CBaseEntity *pActor )
{
	if ( !pActor )
		return;

	uint64_t actorPrefix = static_cast<uint64_t>( reinterpret_cast<uintptr_t>( pActor ) ) << 32;

	for ( auto it = m_entityOverrides.begin(); it != m_entityOverrides.end(); )
	{
		if ( ( it->first & 0xFFFFFFFF00000000ULL ) == actorPrefix )
			it = m_entityOverrides.erase( it );
		else
			++it;
	}

	for ( auto it = m_entityClassOverrides.begin(); it != m_entityClassOverrides.end(); )
	{
		if ( ( it->first & 0xFFFFFFFF00000000ULL ) == actorPrefix )
			it = m_entityClassOverrides.erase( it );
		else
			++it;
	}
}

void MonsterRelationshipManager::ClearAllEntityRelationships()
{
	m_entityOverrides.clear();
	m_entityClassOverrides.clear();
}

void MonsterRelationshipManager::ClearOverrides()
{
	m_classOverrides.clear();
	ClearAllEntityRelationships();
}

void MonsterRelationshipManager::ResetToDefaults()
{
	ClearOverrides();
	InitDefaultTable();
}

int MonsterRelationshipManager::QueryRelationship( int iClassFrom, int iClassTo )
{
	return GetInstance().GetRelationship( iClassFrom, iClassTo );
}

int MonsterRelationshipManager::QueryRelationship( CBaseEntity *pActor, CBaseEntity *pTarget )
{
	return GetInstance().GetRelationship( pActor, pTarget );
}

void MonsterRelationshipManager::OverrideRelationship( int iClassFrom, int iClassTo, int iRelationship )
{
	GetInstance().SetRelationship( iClassFrom, iClassTo, iRelationship );
}

void MonsterRelationshipManager::Reset()
{
	GetInstance().ResetToDefaults();
}
