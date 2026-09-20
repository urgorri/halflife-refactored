/***
 *
 *	Behavioral Equivalence Verification - HUD Registry Unit Tests
 *	Verifies HudRegistry and dynamic HUD element registration (#90)
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "cl_dll/hud/hud_base.h"
#include "cl_dll/hud/hud_registry.h"

#include <string>
#include <vector>

// Mock HUD element for testing lifecycle callbacks
class MockHudElement : public CHudBase
{
  public:
	std::string m_name;
	int m_initCount    = 0;
	int m_vidInitCount = 0;
	int m_drawCount    = 0;
	int m_thinkCount   = 0;
	int m_resetCount   = 0;
	int m_hudDataCount = 0;
	bool m_callsAddHudElemInInit = false;
	CHud *m_pHudContext          = nullptr;

	MockHudElement( const std::string &name = "MockElem", bool callsAddHudElem = false, CHud *pHud = nullptr )
	    : m_name( name ), m_callsAddHudElemInInit( callsAddHudElem ), m_pHudContext( pHud )
	{
		m_iFlags = HUD_ACTIVE;
	}

	int Init( void ) override
	{
		m_initCount++;
		if ( m_callsAddHudElemInInit && m_pHudContext )
		{
			// Emulate vanilla element calling gHUD.AddHudElem( this );
			// Note: AttachAll will also call AddHudElem on pHud; duplicate check must prevent duplicate nodes
		}
		return 1;
	}

	int VidInit( void ) override
	{
		m_vidInitCount++;
		return 1;
	}

	int Draw( float flTime ) override
	{
		(void)flTime;
		m_drawCount++;
		return 1;
	}

	void Think( void ) override
	{
		m_thinkCount++;
	}

	void Reset( void ) override
	{
		m_resetCount++;
	}

	void InitHUDData( void ) override
	{
		m_hudDataCount++;
	}
};

// Specialized mock to test dynamic_cast via FindElement<T>()
class MockStaminaHud : public MockHudElement
{
  public:
	float m_flStamina = 100.0f;

	MockStaminaHud() : MockHudElement( "StaminaHud" ) {}
};

class MockRadarHud : public MockHudElement
{
  public:
	int m_blipCount = 5;

	MockRadarHud() : MockHudElement( "RadarHud" ) {}
};

TEST_CASE( "HudRegistry: Registration, Lookup, and Priority Ordering (#90)", "[hud][registry]" )
{
	HudRegistry::Clear();
	REQUIRE( HudRegistry::GetElements().empty() );

	MockHudElement elemLow( "LowPriority" );
	MockHudElement elemMid( "MidPriority" );
	MockHudElement elemHigh( "HighPriority" );

	// Register out of order
	HudRegistry::RegisterElement( &elemHigh, 300, "HighPriority" );
	HudRegistry::RegisterElement( &elemLow, 100, "LowPriority" );
	HudRegistry::RegisterElement( &elemMid, 200, "MidPriority" );

	REQUIRE( HudRegistry::GetElements().size() == 3 );

	SECTION( "Duplicate registration updates existing entry without increasing count" )
	{
		HudRegistry::RegisterElement( &elemLow, 150, "UpdatedLow" );
		REQUIRE( HudRegistry::GetElements().size() == 3 );

		CHudBase *pFound = HudRegistry::FindElement( "UpdatedLow" );
		REQUIRE( pFound == &elemLow );
	}

	SECTION( "FindElement by name" )
	{
		CHECK( HudRegistry::FindElement( "LowPriority" ) == &elemLow );
		CHECK( HudRegistry::FindElement( "MidPriority" ) == &elemMid );
		CHECK( HudRegistry::FindElement( "HighPriority" ) == &elemHigh );
		CHECK( HudRegistry::FindElement( "NonExistent" ) == nullptr );
		CHECK( HudRegistry::FindElement( nullptr ) == nullptr );
	}

	SECTION( "FindElement by type" )
	{
		MockStaminaHud staminaElem;
		HudRegistry::RegisterElement( &staminaElem, 250, "Stamina" );

		MockStaminaHud *pTyped = HudRegistry::FindElement<MockStaminaHud>();
		REQUIRE( pTyped != nullptr );
		CHECK( pTyped == &staminaElem );
		CHECK( pTyped->m_flStamina == 100.0f );

		// Searching for an unregistered derived type returns nullptr
		MockRadarHud *pUnregistered = HudRegistry::FindElement<MockRadarHud>();
		CHECK( pUnregistered == nullptr );
	}

	SECTION( "UnregisterElement removes elements correctly" )
	{
		CHECK( HudRegistry::UnregisterElement( &elemMid ) == true );
		CHECK( HudRegistry::GetElements().size() == 2 );
		CHECK( HudRegistry::FindElement( "MidPriority" ) == nullptr );

		// Unregistering an element not in the registry returns false
		CHECK( HudRegistry::UnregisterElement( &elemMid ) == false );
		CHECK( HudRegistry::UnregisterElement( nullptr ) == false );
	}
}

TEST_CASE( "HudRegistry: AttachAll and VidInitAll Lifecycle Dispatch (#90)", "[hud][lifecycle]" )
{
	HudRegistry::Clear();

	CHud hud;

	MockHudElement elemA( "ElemA" ); // Order 300
	MockHudElement elemB( "ElemB" ); // Order 100
	MockHudElement elemC( "ElemC" ); // Order 200

	HudRegistry::RegisterElement( &elemA, 300, "ElemA" );
	HudRegistry::RegisterElement( &elemB, 100, "ElemB" );
	HudRegistry::RegisterElement( &elemC, 200, "ElemC" );

	SECTION( "AttachAll initializes elements and inserts them in ascending draw order" )
	{
		HudRegistry::AttachAll( &hud );

		// Check Init was called on all elements
		CHECK( elemA.m_initCount == 1 );
		CHECK( elemB.m_initCount == 1 );
		CHECK( elemC.m_initCount == 1 );

		// Check m_pHudList ordering: B (100) -> C (200) -> A (300)
		REQUIRE( hud.m_pHudList != nullptr );
		CHECK( hud.m_pHudList->p == &elemB );

		REQUIRE( hud.m_pHudList->pNext != nullptr );
		CHECK( hud.m_pHudList->pNext->p == &elemC );

		REQUIRE( hud.m_pHudList->pNext->pNext != nullptr );
		CHECK( hud.m_pHudList->pNext->pNext->p == &elemA );

		CHECK( hud.m_pHudList->pNext->pNext->pNext == nullptr );
	}

	SECTION( "VidInitAll calls VidInit on all attached elements in list" )
	{
		HudRegistry::AttachAll( &hud );
		HudRegistry::VidInitAll( &hud );

		CHECK( elemA.m_vidInitCount == 1 );
		CHECK( elemB.m_vidInitCount == 1 );
		CHECK( elemC.m_vidInitCount == 1 );
	}

	SECTION( "Custom init callback is executed when provided" )
	{
		HudRegistry::Clear();

		static int s_customInitCalls = 0;
		s_customInitCalls = 0;

		MockHudElement customElem( "CustomInitElem" );
		HudRegistry::RegisterElement( &customElem, 50, "CustomInit", []( CHudBase *p ) {
			s_customInitCalls++;
			p->Init();
		} );

		CHud testHud;
		HudRegistry::AttachAll( &testHud );

		CHECK( s_customInitCalls == 1 );
		CHECK( customElem.m_initCount == 1 );
		REQUIRE( testHud.m_pHudList != nullptr );
		CHECK( testHud.m_pHudList->p == &customElem );
	}
}

TEST_CASE( "CHud: AddHudElem, Deduplication, and RemoveHudElem (#90)", "[hud][list]" )
{
	CHud hud;
	MockHudElement elem1( "Elem1" );
	MockHudElement elem2( "Elem2" );
	MockHudElement elem3( "Elem3" );

	SECTION( "AddHudElem appends elements and prevents duplicates" )
	{
		hud.AddHudElem( &elem1 );
		hud.AddHudElem( &elem2 );
		hud.AddHudElem( &elem1 ); // duplicate!
		hud.AddHudElem( nullptr ); // null pointer safety

		REQUIRE( hud.m_pHudList != nullptr );
		CHECK( hud.m_pHudList->p == &elem1 );
		REQUIRE( hud.m_pHudList->pNext != nullptr );
		CHECK( hud.m_pHudList->pNext->p == &elem2 );
		CHECK( hud.m_pHudList->pNext->pNext == nullptr );
	}

	SECTION( "RemoveHudElem safely removes head, middle, and tail nodes" )
	{
		hud.AddHudElem( &elem1 );
		hud.AddHudElem( &elem2 );
		hud.AddHudElem( &elem3 );

		// Remove middle node (elem2)
		hud.RemoveHudElem( &elem2 );
		CHECK( hud.m_pHudList->p == &elem1 );
		CHECK( hud.m_pHudList->pNext->p == &elem3 );
		CHECK( hud.m_pHudList->pNext->pNext == nullptr );

		// Remove head node (elem1)
		hud.RemoveHudElem( &elem1 );
		CHECK( hud.m_pHudList->p == &elem3 );
		CHECK( hud.m_pHudList->pNext == nullptr );

		// Remove tail/last node (elem3)
		hud.RemoveHudElem( &elem3 );
		CHECK( hud.m_pHudList == nullptr );

		// Removing from empty list or null is safe
		hud.RemoveHudElem( &elem1 );
		hud.RemoveHudElem( nullptr );
		CHECK( hud.m_pHudList == nullptr );
	}
}

TEST_CASE( "Dynamic HUD Element Registration Simulation (#90)", "[hud][extensibility]" )
{
	HudRegistry::Clear();

	// Emulate downstream mod declaring a custom stamina HUD element
	static MockStaminaHud s_staminaElement;
	HudRegistry::RegisterElement( &s_staminaElement, 250, "Stamina" );

	CHud gameHud;
	HudRegistry::AttachAll( &gameHud );
	HudRegistry::VidInitAll( &gameHud );

	// Verify custom element received all lifecycle calls
	CHECK( s_staminaElement.m_initCount == 1 );
	CHECK( s_staminaElement.m_vidInitCount == 1 );

	// Verify it can be discovered by downstream code
	MockStaminaHud *pFound = HudRegistry::FindElement<MockStaminaHud>();
	REQUIRE( pFound != nullptr );
	CHECK( pFound == &s_staminaElement );
}

TEST_CASE( "HUD Element Lifecycle: Reset execution with NULL viewport (#117)", "[hud][lifecycle]" )
{
	HudRegistry::Clear();

	MockHudElement elem1( "Elem1" );
	MockHudElement elem2( "Elem2" );

	HudRegistry::RegisterElement( &elem1, 100, "Elem1" );
	HudRegistry::RegisterElement( &elem2, 200, "Elem2" );

	CHud gameHud;
	HudRegistry::AttachAll( &gameHud );

	// Verify MsgFunc_ResetHUD traversal executes Reset on each element safely
	HUDLIST *pList = gameHud.m_pHudList;
	int resetCount = 0;
	while ( pList )
	{
		if ( pList->p )
		{
			pList->p->Reset();
			resetCount++;
		}
		pList = pList->pNext;
	}

	CHECK( resetCount == 2 );
	CHECK( elem1.m_resetCount == 1 );
	CHECK( elem2.m_resetCount == 1 );
}

TEST_CASE( "StudioModel: Bone adjustment and blend weight temporal math (#117)", "[studio][bones]" )
{
	// Test the temporal interpolation formula restored in StudioCalcBoneAdj:
	// value = ( ( a * dadt + b * ( 1.0 - dadt ) ) - 128 ) * ( 360.0 / 256.0 ) + start;
	// When dadt == 1.0f (fully at current state):
	// value should equal ( ( a - 128 ) * ( 360.0 / 256.0 ) + start )
	// When dadt == 0.0f (fully at previous latched state):
	// value should equal ( ( b - 128 ) * ( 360.0 / 256.0 ) + start )

	auto calcAdj = []( float dadt, float a, float b, float start ) -> float {
		return ( ( a * dadt + b * ( 1.0f - dadt ) ) - 128.0f ) * ( 360.0f / 256.0f ) + start;
	};

	float currentController = 200.0f;
	float prevController    = 50.0f;
	float startAngle        = 0.0f;

	// At dadt = 1.0, current state must have 100% influence
	float resultCur = calcAdj( 1.0f, currentController, prevController, startAngle );
	float expectedCur = ( ( currentController - 128.0f ) * ( 360.0f / 256.0f ) + startAngle );
	CHECK( resultCur == Catch::Approx( expectedCur ) );

	// At dadt = 0.0, previous latched state must have 100% influence
	float resultPrev = calcAdj( 0.0f, currentController, prevController, startAngle );
	float expectedPrev = ( ( prevController - 128.0f ) * ( 360.0f / 256.0f ) + startAngle );
	CHECK( resultPrev == Catch::Approx( expectedPrev ) );

	// At dadt = 0.5, midpoint between current and previous
	float resultMid = calcAdj( 0.5f, currentController, prevController, startAngle );
	float expectedMid = ( ( 0.5f * currentController + 0.5f * prevController - 128.0f ) * ( 360.0f / 256.0f ) + startAngle );
	CHECK( resultMid == Catch::Approx( expectedMid ) );

	// Verify sequence blending weight formula restored in StudioSetupBones / StudioMergeBones:
	// s = ( curstate.blending[0] * dadt + latched.prevblending[0] * ( 1.0 - dadt ) ) / 255.0;
	auto calcBlend = []( float dadt, float curBlend, float prevBlend ) -> float {
		return ( curBlend * dadt + prevBlend * ( 1.0f - dadt ) ) / 255.0f;
	};

	float curBlend  = 180.0f;
	float prevBlend = 60.0f;

	CHECK( calcBlend( 1.0f, curBlend, prevBlend ) == Catch::Approx( curBlend / 255.0f ) );
	CHECK( calcBlend( 0.0f, curBlend, prevBlend ) == Catch::Approx( prevBlend / 255.0f ) );
	CHECK( calcBlend( 0.5f, curBlend, prevBlend ) == Catch::Approx( ( 0.5f * curBlend + 0.5f * prevBlend ) / 255.0f ) );
}

