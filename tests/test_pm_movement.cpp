/***
 *
 *	Behavioral Equivalence Verification - Player Movement Unit Tests (Layer 3)
 *	Verifies refactored pm_shared physics against upstream Valve SDK golden baselines.
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_engine.h"
#include "tests/golden/golden_data.h"

#ifdef vec3_t
#undef vec3_t
#endif
typedef float vec_t;
typedef vec_t vec3_t[3];

extern "C" {
#include "pm_shared/pm_defs.h"
#include "pm_shared/pm_info.h"
#include "pm_shared/pm_materials.h"
#include "pm_shared/pm_movevars.h"
#include "pm_shared/pm_shared.h"

extern playermove_t *pmove;

float PM_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed );
void PM_DropPunchAngle( vec3_t punchangle );
void PM_CheckParamters( void );
void PM_Friction( void );
void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel );
void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel );
}

static pmtrace_t Mock_PM_PlayerTrace( float *start, float *end, int traceFlags, int ignore_pe )
{
	pmtrace_t tr;
	std::memset( &tr, 0, sizeof( tr ) );
	tr.fraction = 0.5f;
	return tr;
}

TEST_CASE( "PM_CalcRoll produces upstream-equivalent values", "[pm_shared][physics]" )
{
	for ( size_t i = 0; i < sizeof( kGoldenCalcRoll ) / sizeof( kGoldenCalcRoll[0] ); ++i )
	{
		const auto &g = kGoldenCalcRoll[i];
		vec3_t ang    = {g.angles[0], g.angles[1], g.angles[2]};
		vec3_t vel    = {g.velocity[0], g.velocity[1], g.velocity[2]};

		float result = PM_CalcRoll( ang, vel, g.rollangle, g.rollspeed );
		REQUIRE( result == Catch::Approx( g.expected ).margin( 0.0001f ) );
	}
}

TEST_CASE( "PM_DropPunchAngle decays correctly over time", "[pm_shared][physics]" )
{
	static playermove_t pm;
	static movevars_t mv;
	std::memset( &pm, 0, sizeof( pm ) );
	std::memset( &mv, 0, sizeof( mv ) );
	pm.movevars = &mv;
	pmove       = &pm;

	for ( size_t i = 0; i < sizeof( kGoldenDropPunch ) / sizeof( kGoldenDropPunch[0] ); ++i )
	{
		const auto &g = kGoldenDropPunch[i];
		pm.frametime  = g.frametime;
		vec3_t punch  = {g.initial[0], g.initial[1], g.initial[2]};

		PM_DropPunchAngle( punch );

		REQUIRE( punch[0] == Catch::Approx( g.expected[0] ).margin( 0.0001f ) );
		REQUIRE( punch[1] == Catch::Approx( g.expected[1] ).margin( 0.0001f ) );
		REQUIRE( punch[2] == Catch::Approx( g.expected[2] ).margin( 0.0001f ) );
	}
}

TEST_CASE( "PM_CheckParamters clamps speed to maxspeed", "[pm_shared][physics]" )
{
	static playermove_t pm;
	static movevars_t mv;
	std::memset( &pm, 0, sizeof( pm ) );
	std::memset( &mv, 0, sizeof( mv ) );
	pm.movevars = &mv;
	pmove       = &pm;

	for ( size_t i = 0; i < sizeof( kGoldenCheckParam ) / sizeof( kGoldenCheckParam[0] ); ++i )
	{
		const auto &g        = kGoldenCheckParam[i];
		pm.cmd.forwardmove   = g.cmd[0];
		pm.cmd.sidemove      = g.cmd[1];
		pm.cmd.upmove        = g.cmd[2];
		pm.maxspeed          = g.maxspeed;
		pm.clientmaxspeed    = g.clientmaxspeed;
		pm.frametime         = 0.01f;

		PM_CheckParamters();

		REQUIRE( pm.cmd.forwardmove == Catch::Approx( g.expected_cmd[0] ).margin( 0.0001f ) );
		REQUIRE( pm.cmd.sidemove == Catch::Approx( g.expected_cmd[1] ).margin( 0.0001f ) );
		REQUIRE( pm.cmd.upmove == Catch::Approx( g.expected_cmd[2] ).margin( 0.0001f ) );

		float spd = std::sqrt( pm.cmd.forwardmove * pm.cmd.forwardmove +
                               pm.cmd.sidemove * pm.cmd.sidemove +
                               pm.cmd.upmove * pm.cmd.upmove );
		REQUIRE( spd <= pm.maxspeed + 0.0001f );
		REQUIRE( spd == Catch::Approx( g.expected_speed ).margin( 0.0001f ) );
	}
}

TEST_CASE( "PM_Friction applies ground friction matching golden values", "[pm_shared][physics]" )
{
	static playermove_t pm;
	static movevars_t mv;
	std::memset( &pm, 0, sizeof( pm ) );
	std::memset( &mv, 0, sizeof( mv ) );
	pmove = &pm;

	for ( size_t i = 0; i < sizeof( kGoldenFriction ) / sizeof( kGoldenFriction[0] ); ++i )
	{
		const auto &g      = kGoldenFriction[i];
		std::memset( &pm, 0, sizeof( pm ) );
		pm.movevars        = &mv;
		pm.PM_PlayerTrace  = Mock_PM_PlayerTrace;
		pm.onground        = 0;
		pm.velocity[0]     = g.vel[0];
		pm.velocity[1]     = g.vel[1];
		pm.velocity[2]     = g.vel[2];
		pm.friction        = 1.0f;
		pm.frametime       = g.frametime;
		mv.friction        = g.friction;
		mv.stopspeed       = g.stopspeed;
		mv.edgefriction    = 1.0f;
		pm.waterjumptime   = 0;

		PM_Friction();

		REQUIRE( pm.velocity[0] == Catch::Approx( g.expected_vel[0] ).margin( 0.0001f ) );
		REQUIRE( pm.velocity[1] == Catch::Approx( g.expected_vel[1] ).margin( 0.0001f ) );
		REQUIRE( pm.velocity[2] == Catch::Approx( g.expected_vel[2] ).margin( 0.0001f ) );
	}
}

TEST_CASE( "PM_Accelerate and PM_AirAccelerate match golden values", "[pm_shared][physics]" )
{
	static playermove_t pm;
	static movevars_t mv;
	std::memset( &pm, 0, sizeof( pm ) );
	std::memset( &mv, 0, sizeof( mv ) );
	pmove = &pm;

	for ( size_t i = 0; i < sizeof( kGoldenAccel ) / sizeof( kGoldenAccel[0] ); ++i )
	{
		const auto &g      = kGoldenAccel[i];
		std::memset( &pm, 0, sizeof( pm ) );
		pm.movevars        = &mv;
		pm.velocity[0]     = g.vel[0];
		pm.velocity[1]     = g.vel[1];
		pm.velocity[2]     = g.vel[2];
		pm.frametime       = g.frametime;
		pm.friction        = 1.0f;
		pm.dead            = 0;
		pm.waterjumptime   = 0;

		vec3_t wdir = {g.wishdir[0], g.wishdir[1], g.wishdir[2]};
		if ( g.is_air )
			PM_AirAccelerate( wdir, g.wishspeed, g.accel );
		else
			PM_Accelerate( wdir, g.wishspeed, g.accel );

		REQUIRE( pm.velocity[0] == Catch::Approx( g.expected_vel[0] ).margin( 0.0001f ) );
		REQUIRE( pm.velocity[1] == Catch::Approx( g.expected_vel[1] ).margin( 0.0001f ) );
		REQUIRE( pm.velocity[2] == Catch::Approx( g.expected_vel[2] ).margin( 0.0001f ) );
	}
}
