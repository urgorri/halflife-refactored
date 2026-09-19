/***
 *
 *	Behavioral Equivalence Verification - Weapon Pipeline Unit Tests (Layer 3)
 *	Verifies WeaponRegistry (#85), ClientWeaponManager (#86), and EventRegistry (#87)
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_engine.h"
#include "dlls/weapons/weapon_registry.h"
#include "cl_dll/hl/client_weapon_manager.h"
#include "cl_dll/events/event_registry.h"
#include "weapons/weapon_defs.h"

#include <vector>

// Static tracking for mock precache order
static std::vector<int> s_precacheCallOrder;
static void MockPrecache100() { s_precacheCallOrder.push_back( 100 ); }
static void MockPrecache200() { s_precacheCallOrder.push_back( 200 ); }
static void MockPrecache300() { s_precacheCallOrder.push_back( 300 ); }

// Static tracking for mock event execution
static int s_mockEvent1Calls = 0;
static int s_mockEvent2Calls = 0;
static void MockEventCallback1( struct event_args_s *args ) { (void)args; s_mockEvent1Calls++; }
static void MockEventCallback2( struct event_args_s *args ) { (void)args; s_mockEvent2Calls++; }

TEST_CASE( "WeaponRegistry: registration, ordering, and lookup (#85)", "[weapons][registry]" )
{
	WeaponRegistry::Clear();
	REQUIRE( WeaponRegistry::GetDescriptors().empty() );

	WeaponDescriptor d1 = { "weapon_9mmhandgun", 300, MockPrecache300 };
	WeaponDescriptor d2 = { "weapon_shotgun", 100, MockPrecache100 };
	WeaponDescriptor d3 = { "weapon_crowbar", 200, MockPrecache200 };

	// Register out of priority order
	WeaponRegistry::Register( d1 );
	WeaponRegistry::Register( d2 );
	WeaponRegistry::Register( d3 );

	REQUIRE( WeaponRegistry::GetDescriptors().size() == 3 );

	SECTION( "Duplicate registration is ignored" )
	{
		WeaponRegistry::Register( d1 );
		REQUIRE( WeaponRegistry::GetDescriptors().size() == 3 );
	}

	SECTION( "FindByClassname finds correct descriptors" )
	{
		const WeaponDescriptor *pGlock = WeaponRegistry::FindByClassname( "weapon_9mmhandgun" );
		REQUIRE( pGlock != nullptr );
		CHECK( pGlock->iPriorityOrder == 300 );

		const WeaponDescriptor *pShotgun = WeaponRegistry::FindByClassname( "weapon_shotgun" );
		REQUIRE( pShotgun != nullptr );
		CHECK( pShotgun->iPriorityOrder == 100 );

		CHECK( WeaponRegistry::FindByClassname( "weapon_nonexistent" ) == nullptr );
		CHECK( WeaponRegistry::FindByClassname( nullptr ) == nullptr );
	}

	SECTION( "PrecacheAll executes in ascending priority order" )
	{
		s_precacheCallOrder.clear();
		WeaponRegistry::PrecacheAll();

		REQUIRE( s_precacheCallOrder.size() == 3 );
		CHECK( s_precacheCallOrder[0] == 100 );
		CHECK( s_precacheCallOrder[1] == 200 );
		CHECK( s_precacheCallOrder[2] == 300 );

		// Descriptors in storage should now be sorted
		const auto &descs = WeaponRegistry::GetDescriptors();
		CHECK( descs[0].iPriorityOrder == 100 );
		CHECK( descs[1].iPriorityOrder == 200 );
		CHECK( descs[2].iPriorityOrder == 300 );
	}
}

TEST_CASE( "ClientWeaponManager: bounds, table lookup, and entity safety (#86)", "[client][weapons][prediction]" )
{
	ClientWeaponManager::Clear();

	SECTION( "Array bounds and default state" )
	{
		REQUIRE( ClientWeaponManager::GetMaxWeapons() == MAX_WEAPONS );
		REQUIRE( ClientWeaponManager::GetEntityCount() == 0 );

		// All uninitialized slots should be null
		for ( int i = 0; i < MAX_WEAPONS; ++i )
		{
			CHECK( ClientWeaponManager::GetWeapon( i ) == nullptr );
			CHECK( ClientWeaponManager::GetWeaponByIndex( i ) == nullptr );
		}

		// Out of bounds accesses must return nullptr safely
		CHECK( ClientWeaponManager::GetWeapon( -1 ) == nullptr );
		CHECK( ClientWeaponManager::GetWeapon( MAX_WEAPONS ) == nullptr );
		CHECK( ClientWeaponManager::GetWeapon( 999 ) == nullptr );
		CHECK( ClientWeaponManager::GetWeaponByIndex( -1 ) == nullptr );
		CHECK( ClientWeaponManager::GetWeaponByIndex( MAX_WEAPONS ) == nullptr );
	}

	SECTION( "RegisterWeapon and GetWeapon lookup" )
	{
		// Use dummy pointer addresses for lookup verification
		uintptr_t dummy1 = 0x1000;
		uintptr_t dummy2 = 0x2000;
		CBasePlayerWeapon *pWpn1 = reinterpret_cast<CBasePlayerWeapon *>( dummy1 );
		CBasePlayerWeapon *pWpn2 = reinterpret_cast<CBasePlayerWeapon *>( dummy2 );

		ClientWeaponManager::RegisterWeapon( WEAPON_GLOCK, pWpn1 );
		ClientWeaponManager::RegisterWeapon( WEAPON_PYTHON, pWpn2 );

		CHECK( ClientWeaponManager::GetWeapon( WEAPON_GLOCK ) == pWpn1 );
		CHECK( ClientWeaponManager::GetWeaponByIndex( WEAPON_GLOCK ) == pWpn1 );
		CHECK( ClientWeaponManager::GetWeapon( WEAPON_PYTHON ) == pWpn2 );
		CHECK( ClientWeaponManager::GetWeapon( WEAPON_MP5 ) == nullptr );

		// Registering to invalid ID is safely rejected
		ClientWeaponManager::RegisterWeapon( -1, pWpn1 );
		ClientWeaponManager::RegisterWeapon( MAX_WEAPONS, pWpn1 );
	}

	SECTION( "Entity allocation pool bounds safety" )
	{
		struct DummyClientEntity
		{
			entvars_t *pev;
		};

		DummyClientEntity dummyEnts[MAX_CLIENT_ENTITIES + 5];

		// Allocate up to capacity
		for ( int i = 0; i < MAX_CLIENT_ENTITIES; ++i )
		{
			ClientWeaponManager::PrepEntity( reinterpret_cast<CBaseEntity *>( &dummyEnts[i] ), nullptr );
		}
		REQUIRE( ClientWeaponManager::GetEntityCount() == MAX_CLIENT_ENTITIES );

		// Overflow attempt must be safely rejected without crash
		ClientWeaponManager::PrepEntity( reinterpret_cast<CBaseEntity *>( &dummyEnts[MAX_CLIENT_ENTITIES] ), nullptr );
		CHECK( ClientWeaponManager::GetEntityCount() == MAX_CLIENT_ENTITIES );

		// Null entity pointer is safely ignored
		ClientWeaponManager::PrepEntity( nullptr, nullptr );
		CHECK( ClientWeaponManager::GetEntityCount() == MAX_CLIENT_ENTITIES );
	}


	SECTION( "Custom weapon factory registration" )
	{
		static bool s_customFactoryCreated = false;
		class MockCustomFactory : public IClientWeaponFactory
		{
		  public:
			virtual CBasePlayerWeapon *Create( void ) override
			{
				s_customFactoryCreated = true;
				return nullptr;
			}
		};

		MockCustomFactory factory;
		ClientWeaponManager::RegisterCustomFactory( &factory );

		s_customFactoryCreated = false;
		ClientWeaponManager::Init( nullptr );
		// In non-CLIENT_DLL build (test harness), custom factory registration is tracked
	}
}

TEST_CASE( "EventRegistry: declarative event registration and lookup (#87)", "[client][events]" )
{
	EventRegistry::Clear();
	REQUIRE( EventRegistry::GetRegistrations().empty() );

	EventRegistry::Register( "events/glock1.sc", MockEventCallback1 );
	EventRegistry::Register( "events/glock2.sc", MockEventCallback2 );

	REQUIRE( EventRegistry::GetRegistrations().size() == 2 );

	SECTION( "Duplicate script path registration is prevented" )
	{
		EventRegistry::Register( "events/glock1.sc", MockEventCallback1 );
		REQUIRE( EventRegistry::GetRegistrations().size() == 2 );
	}

	SECTION( "FindByScript returns matching registration" )
	{
		const EventRegistration *pReg1 = EventRegistry::FindByScript( "events/glock1.sc" );
		REQUIRE( pReg1 != nullptr );
		CHECK( pReg1->pfnCallback == MockEventCallback1 );

		const EventRegistration *pReg2 = EventRegistry::FindByScript( "events/glock2.sc" );
		REQUIRE( pReg2 != nullptr );
		CHECK( pReg2->pfnCallback == MockEventCallback2 );

		CHECK( EventRegistry::FindByScript( "events/nonexistent.sc" ) == nullptr );
		CHECK( EventRegistry::FindByScript( nullptr ) == nullptr );
	}

	SECTION( "Null parameters are safely rejected" )
	{
		EventRegistry::Register( nullptr, MockEventCallback1 );
		EventRegistry::Register( "events/test.sc", nullptr );
		REQUIRE( EventRegistry::GetRegistrations().size() == 2 );
	}
}
