/***
 *
 *	Behavioral Equivalence Verification - Test Suite Entry Point
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_engine.h"

int main( int argc, char *argv[] )
{
	InitMockEngine();
	int result = Catch::Session().run( argc, argv );
	return result;
}
