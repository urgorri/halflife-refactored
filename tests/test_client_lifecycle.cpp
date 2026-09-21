/***
 *
 *	Behavioral Equivalence Verification - Client Lifecycle Unit Tests (Layer 3B)
 *	Verifies client DLL initialization, version handshake, HUD_Init with NULL gViewPort,
 *	and HUD_Reset state machine without running HLDS or graphical engine.
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_client_engine.h"

// Lifecycle entry point wrappers mapping directly to GoldSrc ABI
static inline int Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	return ClientInitialize( pEnginefuncs, iVersion );
}

static inline void HUD_Init( void )
{
	ClientHUD_Init();
}

static inline int HUD_VidInit( void )
{
	return ClientHUD_VidInit();
}

static inline void HUD_Reset( void )
{
	ClientHUD_Reset();
}

static inline int HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	return ClientHUD_GetStudioModelInterface( version, ppinterface, pstudio );
}

TEST_CASE( "Client Lifecycle: Library Loading and Resolution", "[client][lifecycle]" )
{
	REQUIRE( LoadClientLibrary() == true );
	CHECK( g_pfnInitialize != nullptr );
	CHECK( g_pfnHUD_Init != nullptr );
	CHECK( g_pfnHUD_Reset != nullptr );
	CHECK( g_pfnHUD_GetStudioModelInterface != nullptr );
}

TEST_CASE( "Client Lifecycle: Initialize -> HUD_Init with NULL gViewPort", "[client][lifecycle]" )
{
	InitMockClientEngine();

	// Step 1: Initialize client engine function table and version handshake
	int initResult = Initialize( &g_mockClientEngineFuncs, CLDLL_INTERFACE_VERSION );
	REQUIRE( initResult == 1 );

	// Step 2: Pre-Video HUD_Init (gViewPort must be NULL at this point before VGui_Startup)
	REQUIRE( gViewPort == nullptr );

	// Must NOT crash or dereference gViewPort (#118 regression test)
	REQUIRE_NOTHROW( HUD_Init() );

	// Verify HUD elements registered their cvars and commands via mock engine
	CHECK( g_mockCvars.find( "spec_pip" ) != g_mockCvars.end() );
	CHECK( g_mockCvars.find( "spec_autodirector" ) != g_mockCvars.end() );
	CHECK( g_mockCvars.find( "default_fov" ) != g_mockCvars.end() );
	CHECK( g_mockCvars.find( "cl_autowepswitch" ) != g_mockCvars.end() );
}

TEST_CASE( "Client Lifecycle: Version Handshake Rejection", "[client][lifecycle]" )
{
	InitMockClientEngine();

	// Initialize must reject older interface versions
	int badVersionLow = Initialize( &g_mockClientEngineFuncs, CLDLL_INTERFACE_VERSION - 1 );
	CHECK( badVersionLow == 0 );

	// Initialize must reject newer interface versions
	int badVersionHigh = Initialize( &g_mockClientEngineFuncs, CLDLL_INTERFACE_VERSION + 1 );
	CHECK( badVersionHigh == 0 );

	// Correct version must succeed
	int goodVersion = Initialize( &g_mockClientEngineFuncs, CLDLL_INTERFACE_VERSION );
	CHECK( goodVersion == 1 );
}

TEST_CASE( "Client Lifecycle: HUD_Reset State Machine", "[client][lifecycle]" )
{
	InitMockClientEngine();

	int initResult = Initialize( &g_mockClientEngineFuncs, CLDLL_INTERFACE_VERSION );
	REQUIRE( initResult == 1 );
	HUD_Init();

	// Reset before any map is loaded
	REQUIRE( gViewPort == nullptr );
	REQUIRE_NOTHROW( HUD_Reset() );

	// Reset after map transition
	SetMockMapName( "c1a0" );
	REQUIRE_NOTHROW( HUD_Reset() );

	// Reset after map change to multiplayer map
	SetMockMapName( "crossfire" );
	SetMockPlayerCount( 8 );
	REQUIRE_NOTHROW( HUD_Reset() );

	// Disconnection back to empty/no map
	SetMockMapName( "" );
	SetMockPlayerCount( 0 );
	REQUIRE_NOTHROW( HUD_Reset() );

	// Verify client commands were dispatched during reset cycles
	CHECK_FALSE( g_mockClientCmds.empty() );
}
