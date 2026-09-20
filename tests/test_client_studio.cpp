/***
 *
 *	Behavioral Equivalence Verification - Studio Model Interface Tests (Layer 3B)
 *	Verifies HUD_GetStudioModelInterface version handshake and engine studio API binding.
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_client_engine.h"

static inline int HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	return ClientHUD_GetStudioModelInterface( version, ppinterface, pstudio );
}

TEST_CASE( "Client Studio: HUD_GetStudioModelInterface Version Handshake", "[client][studio]" )
{
	InitMockClientEngine();

	r_studio_interface_t *pStudio = nullptr;

	// Incompatible version must return 0
	int badStudioLow = HUD_GetStudioModelInterface( STUDIO_INTERFACE_VERSION - 1, &pStudio, &g_mockStudioApi );
	CHECK( badStudioLow == 0 );

	int badStudioHigh = HUD_GetStudioModelInterface( STUDIO_INTERFACE_VERSION + 1, &pStudio, &g_mockStudioApi );
	CHECK( badStudioHigh == 0 );

	// Valid version must return 1 and supply non-null callback pointers
	int goodStudio = HUD_GetStudioModelInterface( STUDIO_INTERFACE_VERSION, &pStudio, &g_mockStudioApi );
	REQUIRE( goodStudio == 1 );
	REQUIRE( pStudio != nullptr );
	CHECK( pStudio->version == STUDIO_INTERFACE_VERSION );
}
