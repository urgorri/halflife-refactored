/***
 *
 *	Behavioral Equivalence Verification - Golden Value Generator
 *	Links against upstream Valve SDK sources (before refactoring)
 *	to capture exact mathematical baselines.
 *
 ****/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include "archtypes.h"
#include "common/mathlib.h"
#include "common/const.h"
#include "common/usercmd.h"
#include "game_shared/damage_defs.h"
#include "pm_shared/pm_defs.h"
#include "pm_shared/pm_info.h"
#include "pm_shared/pm_materials.h"
#include "pm_shared/pm_movevars.h"
#include "pm_shared/pm_shared.h"

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7

extern playermove_t *pmove;

// Upstream function declarations
float PM_CalcRoll( vec3_t angles, vec3_t velocity, float rollangle, float rollspeed );
void PM_DropPunchAngle( vec3_t punchangle );
void PM_CheckParamters( void );
void PM_Friction( void );
void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel );
void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel );
}

// ----------------------------------------------------------------------
// Combat calculation models from upstream Valve SDK (combat.cpp, player.cpp)
// ----------------------------------------------------------------------

struct RadiusDamageCase
{
	float src[3];
	float end[3];
	float damage;
	float radius;
	float expected_damage;
};

struct HitgroupCase
{
	int hitgroup;
	const char *hitgroup_name;
	float input_damage;
	float monster_damage;
	float player_damage;
};

struct ArmorCase
{
	const char *description;
	float damage;
	float initial_armor;
	int damage_type;
	bool is_multiplayer;
	float expected_damage;
	float expected_armor;
};

static float Upstream_RadiusDamage( const float src[3], const float end[3], float flDamage, float flRadius )
{
	float falloff;
	if ( flRadius != 0.0f )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0f;

	float dx   = src[0] - end[0];
	float dy   = src[1] - end[1];
	float dz   = src[2] - end[2];
	float dist = std::sqrt( dx * dx + dy * dy + dz * dz );

	if ( flRadius == 0.0f )
		return flDamage;

	float adjusted = flDamage - dist * falloff;
	if ( adjusted < 0.0f )
		adjusted = 0.0f;
	return adjusted;
}

static void Upstream_ArmorDamage( float flDamage, float flArmorValue, int bitsDamageType, bool isMultiplayer, float &outDamage, float &outArmor )
{
	float flRatio = 0.2f; // ARMOR_RATIO
	float flBonus = 0.5f; // ARMOR_BONUS

	if ( ( bitsDamageType & DMG_BLAST ) && isMultiplayer )
	{
		flRatio = 0.20f;
		flBonus = 1.0f;
	}

	if ( flArmorValue > 0.0f && !( bitsDamageType & ( DMG_FALL | DMG_DROWN ) ) )
	{
		float flNew   = flDamage * flRatio;
		float flArmor = ( flDamage - flNew ) * flBonus;

		if ( flArmor > flArmorValue )
		{
			flArmor = flArmorValue;
			flArmor *= ( 1.0f / flBonus );
			flNew         = flDamage - flArmor;
			flArmorValue = 0.0f;
		}
		else
		{
			flArmorValue -= flArmor;
		}

		flDamage = flNew;
	}

	outDamage = flDamage;
	outArmor  = flArmorValue;
}

// ----------------------------------------------------------------------
// Main Generator
// ----------------------------------------------------------------------

static pmtrace_t Stub_PM_PlayerTrace( float *start, float *end, int traceFlags, int ignore_pe )
{
	pmtrace_t tr;
	std::memset( &tr, 0, sizeof( tr ) );
	tr.fraction = 0.5f;
	return tr;
}

int main( int argc, char **argv )
{
	std::cout << "Generating golden reference values from upstream SDK..." << std::endl;

	// Setup mock playermove_t and movevars_t for upstream code
	static playermove_t mock_pmove;
	static movevars_t mock_movevars;
	std::memset( &mock_pmove, 0, sizeof( mock_pmove ) );
	std::memset( &mock_movevars, 0, sizeof( mock_movevars ) );

	mock_movevars.friction     = 4.0f;
	mock_movevars.stopspeed    = 100.0f;
	mock_movevars.edgefriction = 1.0f;
	mock_movevars.rollangle    = 2.0f;
	mock_movevars.rollspeed    = 200.0f;

	mock_pmove.movevars        = &mock_movevars;
	mock_pmove.PM_PlayerTrace  = Stub_PM_PlayerTrace;
	mock_pmove.onground        = 0;
	mock_pmove.friction        = 1.0f;
	mock_pmove.frametime       = 0.01f;
	pmove                      = &mock_pmove;

	// 1. PM_CalcRoll test vectors
	struct CalcRollVector
	{
		float angles[3];
		float velocity[3];
		float rollangle;
		float rollspeed;
		float expected;
	};

	std::vector<CalcRollVector> calc_roll_tests = {
	    {{0.0f, 90.0f, 0.0f}, {100.0f, 50.0f, 0.0f}, 2.0f, 200.0f, 0.0f},
	    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 2.0f, 200.0f, 0.0f},
	    {{10.0f, 45.0f, 0.0f}, {-200.0f, 150.0f, 50.0f}, 2.0f, 200.0f, 0.0f},
	    {{0.0f, 180.0f, 0.0f}, {0.0f, -300.0f, 0.0f}, 2.5f, 150.0f, 0.0f},
	    {{0.0f, 270.0f, 0.0f}, {50.0f, -50.0f, 0.0f}, 1.5f, 250.0f, 0.0f},
	};

	std::cout << "Starting calc_roll tests..." << std::endl;
	for ( auto &t : calc_roll_tests )
	{
		vec3_t ang = {t.angles[0], t.angles[1], t.angles[2]};
		vec3_t vel = {t.velocity[0], t.velocity[1], t.velocity[2]};
		t.expected = PM_CalcRoll( ang, vel, t.rollangle, t.rollspeed );
	}
	std::cout << "CalcRoll tests done." << std::endl;

	// 2. PM_DropPunchAngle test vectors
	struct DropPunchVector
	{
		float initial[3];
		float frametime;
		float expected[3];
	};

	std::vector<DropPunchVector> drop_punch_tests = {
	    {{5.0f, 3.0f, 0.0f}, 0.01f, {0.0f, 0.0f, 0.0f}},
	    {{10.0f, -5.0f, 2.0f}, 0.02f, {0.0f, 0.0f, 0.0f}},
	    {{0.1f, 0.05f, 0.0f}, 0.05f, {0.0f, 0.0f, 0.0f}},
	    {{0.0f, 0.0f, 0.0f}, 0.01f, {0.0f, 0.0f, 0.0f}},
	};

	std::cout << "Starting drop_punch tests..." << std::endl;
	for ( auto &t : drop_punch_tests )
	{
		mock_pmove.frametime = t.frametime;
		vec3_t punch         = {t.initial[0], t.initial[1], t.initial[2]};
		PM_DropPunchAngle( punch );
		t.expected[0] = punch[0];
		t.expected[1] = punch[1];
		t.expected[2] = punch[2];
	}
	std::cout << "DropPunch tests done." << std::endl;

	// 3. PM_CheckParamters test vectors
	struct CheckParamVector
	{
		float forwardmove;
		float sidemove;
		float upmove;
		float maxspeed;
		float clientmaxspeed;
		float expected_forward;
		float expected_side;
		float expected_up;
		float expected_speed;
	};

	std::vector<CheckParamVector> check_param_tests = {
	    {500.0f, 500.0f, 0.0f, 320.0f, 320.0f, 0, 0, 0, 0},
	    {200.0f, 100.0f, 0.0f, 320.0f, 320.0f, 0, 0, 0, 0},
	    {0.0f, 0.0f, 0.0f, 320.0f, 320.0f, 0, 0, 0, 0},
	    {400.0f, 0.0f, 300.0f, 250.0f, 250.0f, 0, 0, 0, 0},
	};

	std::cout << "Starting CheckParamters tests..." << std::endl;
	for ( auto &t : check_param_tests )
	{
		mock_pmove.cmd.forwardmove = t.forwardmove;
		mock_pmove.cmd.sidemove    = t.sidemove;
		mock_pmove.cmd.upmove      = t.upmove;
		mock_pmove.maxspeed        = t.maxspeed;
		mock_pmove.clientmaxspeed  = t.clientmaxspeed;
		mock_pmove.movevars        = &mock_movevars;
		mock_pmove.frametime       = 0.01f;

		PM_CheckParamters();

		t.expected_forward = mock_pmove.cmd.forwardmove;
		t.expected_side    = mock_pmove.cmd.sidemove;
		t.expected_up      = mock_pmove.cmd.upmove;
		t.expected_speed   = std::sqrt( t.expected_forward * t.expected_forward +
                                      t.expected_side * t.expected_side +
                                      t.expected_up * t.expected_up );
	}
	std::cout << "CheckParamters tests done." << std::endl;

	// 4. PM_Friction test vectors
	struct FrictionVector
	{
		float initial_vel[3];
		float friction;
		float stopspeed;
		float frametime;
		float expected_vel[3];
	};

	std::vector<FrictionVector> friction_tests = {
	    {{200.0f, 100.0f, 0.0f}, 4.0f, 100.0f, 0.01f, {0, 0, 0}},
	    {{50.0f, 0.0f, 0.0f}, 4.0f, 100.0f, 0.01f, {0, 0, 0}},
	    {{0.05f, 0.0f, 0.0f}, 4.0f, 100.0f, 0.01f, {0, 0, 0}},
	};

	std::cout << "Starting Friction tests..." << std::endl;
	for ( auto &t : friction_tests )
	{
		std::memset( &mock_pmove, 0, sizeof( mock_pmove ) );
		mock_pmove.movevars        = &mock_movevars;
		mock_pmove.PM_PlayerTrace  = Stub_PM_PlayerTrace;
		mock_pmove.onground        = 0;
		mock_pmove.velocity[0]     = t.initial_vel[0];
		mock_pmove.velocity[1]     = t.initial_vel[1];
		mock_pmove.velocity[2]     = t.initial_vel[2];
		mock_pmove.friction        = 1.0f;
		mock_pmove.frametime       = t.frametime;
		mock_movevars.friction     = t.friction;
		mock_movevars.stopspeed    = t.stopspeed;
		mock_movevars.edgefriction = 1.0f;
		mock_pmove.waterjumptime   = 0;

		PM_Friction();

		t.expected_vel[0] = mock_pmove.velocity[0];
		t.expected_vel[1] = mock_pmove.velocity[1];
		t.expected_vel[2] = mock_pmove.velocity[2];
	}
	std::cout << "Friction tests done." << std::endl;

	// 5. PM_Accelerate and PM_AirAccelerate test vectors
	struct AccelVector
	{
		bool is_air;
		float initial_vel[3];
		float wishdir[3];
		float wishspeed;
		float accel;
		float frametime;
		float expected_vel[3];
	};

	std::vector<AccelVector> accel_tests = {
	    // Ground acceleration
	    {false, {100.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 320.0f, 10.0f, 0.01f, {0, 0, 0}},
	    {false, {320.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 320.0f, 10.0f, 0.01f, {0, 0, 0}},
	    {false, {100.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 320.0f, 10.0f, 0.01f, {0, 0, 0}},
	    // Air acceleration
	    {true, {200.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 320.0f, 10.0f, 0.01f, {0, 0, 0}},
	    {true, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 250.0f, 10.0f, 0.01f, {0, 0, 0}},
	};

	std::cout << "Starting Accel tests..." << std::endl;

	for ( auto &t : accel_tests )
	{
		std::memset( &mock_pmove, 0, sizeof( mock_pmove ) );
		mock_pmove.movevars      = &mock_movevars;
		mock_pmove.velocity[0]   = t.initial_vel[0];
		mock_pmove.velocity[1]   = t.initial_vel[1];
		mock_pmove.velocity[2]   = t.initial_vel[2];
		mock_pmove.frametime     = t.frametime;
		mock_pmove.friction      = 1.0f;
		mock_pmove.dead          = 0;
		mock_pmove.waterjumptime = 0;

		vec3_t wdir = {t.wishdir[0], t.wishdir[1], t.wishdir[2]};
		if ( t.is_air )
			PM_AirAccelerate( wdir, t.wishspeed, t.accel );
		else
			PM_Accelerate( wdir, t.wishspeed, t.accel );

		t.expected_vel[0] = mock_pmove.velocity[0];
		t.expected_vel[1] = mock_pmove.velocity[1];
		t.expected_vel[2] = mock_pmove.velocity[2];
	}

	// 6. Combat: RadiusDamage test vectors
	std::vector<RadiusDamageCase> radius_tests = {
	    {{0, 0, 0}, {0, 0, 0}, 100.0f, 250.0f, 0.0f},
	    {{0, 0, 0}, {125.0f, 0, 0}, 100.0f, 250.0f, 0.0f},
	    {{0, 0, 0}, {250.0f, 0, 0}, 100.0f, 250.0f, 0.0f},
	    {{0, 0, 0}, {300.0f, 0, 0}, 100.0f, 250.0f, 0.0f},
	    {{0, 0, 0}, {50.0f, 50.0f, 0}, 150.0f, 0.0f, 0.0f},
	};

	for ( auto &t : radius_tests )
	{
		t.expected_damage = Upstream_RadiusDamage( t.src, t.end, t.damage, t.radius );
	}

	// 7. Combat: Hitgroup damage multipliers
	std::vector<HitgroupCase> hitgroup_tests = {
	    {HITGROUP_GENERIC, "HITGROUP_GENERIC", 40.0f, 40.0f, 40.0f},
	    {HITGROUP_HEAD, "HITGROUP_HEAD", 40.0f, 40.0f * 3.0f, 40.0f * 3.0f},
	    {HITGROUP_CHEST, "HITGROUP_CHEST", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	    {HITGROUP_STOMACH, "HITGROUP_STOMACH", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	    {HITGROUP_LEFTARM, "HITGROUP_LEFTARM", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	    {HITGROUP_RIGHTARM, "HITGROUP_RIGHTARM", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	    {HITGROUP_LEFTLEG, "HITGROUP_LEFTLEG", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	    {HITGROUP_RIGHTLEG, "HITGROUP_RIGHTLEG", 40.0f, 40.0f * 1.0f, 40.0f * 1.0f},
	};

	// 8. Combat: Armor damage absorption
	std::vector<ArmorCase> armor_tests = {
	    {"bullet_standard_absorption", 50.0f, 100.0f, DMG_BULLET, false, 0, 0},
	    {"armor_exhaustion", 100.0f, 30.0f, DMG_BULLET, false, 0, 0},
	    {"fall_damage_unprotected", 50.0f, 100.0f, DMG_FALL, false, 0, 0},
	    {"drown_damage_unprotected", 50.0f, 100.0f, DMG_DROWN, false, 0, 0},
	    {"multiplayer_blast_bonus", 100.0f, 100.0f, DMG_BLAST, true, 0, 0},
	};

	for ( auto &t : armor_tests )
	{
		Upstream_ArmorDamage( t.damage, t.initial_armor, t.damage_type, t.is_multiplayer, t.expected_damage, t.expected_armor );
	}

	// ------------------------------------------------------------------
	// Output tests/golden/pm_movement.json
	// ------------------------------------------------------------------
	{
		std::ofstream out( "tests/golden/pm_movement.json" );
		out << std::setprecision( 6 ) << std::fixed;
		out << "{\n";

		// PM_CalcRoll
		out << "  \"PM_CalcRoll\": [\n";
		for ( size_t i = 0; i < calc_roll_tests.size(); ++i )
		{
			const auto &t = calc_roll_tests[i];
			out << "    {\"angles\": [" << t.angles[0] << ", " << t.angles[1] << ", " << t.angles[2] << "], "
			    << "\"velocity\": [" << t.velocity[0] << ", " << t.velocity[1] << ", " << t.velocity[2] << "], "
			    << "\"rollangle\": " << t.rollangle << ", \"rollspeed\": " << t.rollspeed << ", "
			    << "\"expected\": " << t.expected << "}" << ( i + 1 < calc_roll_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// PM_DropPunchAngle
		out << "  \"PM_DropPunchAngle\": [\n";
		for ( size_t i = 0; i < drop_punch_tests.size(); ++i )
		{
			const auto &t = drop_punch_tests[i];
			out << "    {\"initial\": [" << t.initial[0] << ", " << t.initial[1] << ", " << t.initial[2] << "], "
			    << "\"frametime\": " << t.frametime << ", "
			    << "\"expected\": [" << t.expected[0] << ", " << t.expected[1] << ", " << t.expected[2] << "]}"
			    << ( i + 1 < drop_punch_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// PM_CheckParamters
		out << "  \"PM_CheckParamters\": [\n";
		for ( size_t i = 0; i < check_param_tests.size(); ++i )
		{
			const auto &t = check_param_tests[i];
			out << "    {\"cmd\": [" << t.forwardmove << ", " << t.sidemove << ", " << t.upmove << "], "
			    << "\"maxspeed\": " << t.maxspeed << ", \"clientmaxspeed\": " << t.clientmaxspeed << ", "
			    << "\"expected_cmd\": [" << t.expected_forward << ", " << t.expected_side << ", " << t.expected_up << "], "
			    << "\"expected_speed\": " << t.expected_speed << "}"
			    << ( i + 1 < check_param_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// PM_Friction
		out << "  \"PM_Friction\": [\n";
		for ( size_t i = 0; i < friction_tests.size(); ++i )
		{
			const auto &t = friction_tests[i];
			out << "    {\"velocity\": [" << t.initial_vel[0] << ", " << t.initial_vel[1] << ", " << t.initial_vel[2] << "], "
			    << "\"friction\": " << t.friction << ", \"stopspeed\": " << t.stopspeed << ", \"frametime\": " << t.frametime << ", "
			    << "\"expected_velocity\": [" << t.expected_vel[0] << ", " << t.expected_vel[1] << ", " << t.expected_vel[2] << "]}"
			    << ( i + 1 < friction_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// PM_Accelerate
		out << "  \"PM_Accelerate\": [\n";
		for ( size_t i = 0; i < accel_tests.size(); ++i )
		{
			const auto &t = accel_tests[i];
			out << "    {\"is_air\": " << ( t.is_air ? "true" : "false" ) << ", "
			    << "\"velocity\": [" << t.initial_vel[0] << ", " << t.initial_vel[1] << ", " << t.initial_vel[2] << "], "
			    << "\"wishdir\": [" << t.wishdir[0] << ", " << t.wishdir[1] << ", " << t.wishdir[2] << "], "
			    << "\"wishspeed\": " << t.wishspeed << ", \"accel\": " << t.accel << ", \"frametime\": " << t.frametime << ", "
			    << "\"expected_velocity\": [" << t.expected_vel[0] << ", " << t.expected_vel[1] << ", " << t.expected_vel[2] << "]}"
			    << ( i + 1 < accel_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ]\n";
		out << "}\n";
	}
	std::cout << "Wrote tests/golden/pm_movement.json" << std::endl;

	// ------------------------------------------------------------------
	// Output tests/golden/combat.json
	// ------------------------------------------------------------------
	{
		std::ofstream out( "tests/golden/combat.json" );
		out << std::setprecision( 6 ) << std::fixed;
		out << "{\n";

		// RadiusDamage
		out << "  \"RadiusDamage\": [\n";
		for ( size_t i = 0; i < radius_tests.size(); ++i )
		{
			const auto &t = radius_tests[i];
			out << "    {\"src\": [" << t.src[0] << ", " << t.src[1] << ", " << t.src[2] << "], "
			    << "\"end\": [" << t.end[0] << ", " << t.end[1] << ", " << t.end[2] << "], "
			    << "\"damage\": " << t.damage << ", \"radius\": " << t.radius << ", "
			    << "\"expected_damage\": " << t.expected_damage << "}"
			    << ( i + 1 < radius_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// Hitgroup multipliers
		out << "  \"HitgroupMultipliers\": [\n";
		for ( size_t i = 0; i < hitgroup_tests.size(); ++i )
		{
			const auto &t = hitgroup_tests[i];
			out << "    {\"hitgroup\": " << t.hitgroup << ", \"name\": \"" << t.hitgroup_name << "\", "
			    << "\"input_damage\": " << t.input_damage << ", \"monster_damage\": " << t.monster_damage << ", "
			    << "\"player_damage\": " << t.player_damage << "}"
			    << ( i + 1 < hitgroup_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ],\n";

		// Armor absorption
		out << "  \"ArmorAbsorption\": [\n";
		for ( size_t i = 0; i < armor_tests.size(); ++i )
		{
			const auto &t = armor_tests[i];
			out << "    {\"name\": \"" << t.description << "\", \"damage\": " << t.damage << ", "
			    << "\"initial_armor\": " << t.initial_armor << ", \"damage_type\": " << t.damage_type << ", "
			    << "\"is_multiplayer\": " << ( t.is_multiplayer ? "true" : "false" ) << ", "
			    << "\"expected_damage\": " << t.expected_damage << ", "
			    << "\"expected_armor\": " << t.expected_armor << "}"
			    << ( i + 1 < armor_tests.size() ? "," : "" ) << "\n";
		}
		out << "  ]\n";
		out << "}\n";
	}
	std::cout << "Wrote tests/golden/combat.json" << std::endl;

	// ------------------------------------------------------------------
	// Output tests/golden/golden_data.h (C++ header for direct compile-time inclusion in tests)
	// ------------------------------------------------------------------
	{
		std::ofstream out( "tests/golden/golden_data.h" );
		out << std::setprecision( 6 ) << std::fixed;
		out << "// Auto-generated by tests/generate_golden.cpp - DO NOT EDIT MANUALLY\n";
		out << "#pragma once\n\n";

		// CalcRoll
		out << "struct GoldenCalcRoll { float angles[3]; float velocity[3]; float rollangle; float rollspeed; float expected; };\n";
		out << "static const GoldenCalcRoll kGoldenCalcRoll[] = {\n";
		for ( const auto &t : calc_roll_tests )
		{
			out << "    {{" << t.angles[0] << "f, " << t.angles[1] << "f, " << t.angles[2] << "f}, "
			    << "{" << t.velocity[0] << "f, " << t.velocity[1] << "f, " << t.velocity[2] << "f}, "
			    << t.rollangle << "f, " << t.rollspeed << "f, " << t.expected << "f},\n";
		}
		out << "};\n\n";

		// DropPunch
		out << "struct GoldenDropPunch { float initial[3]; float frametime; float expected[3]; };\n";
		out << "static const GoldenDropPunch kGoldenDropPunch[] = {\n";
		for ( const auto &t : drop_punch_tests )
		{
			out << "    {{" << t.initial[0] << "f, " << t.initial[1] << "f, " << t.initial[2] << "f}, "
			    << t.frametime << "f, {" << t.expected[0] << "f, " << t.expected[1] << "f, " << t.expected[2] << "f}},\n";
		}
		out << "};\n\n";

		// CheckParamters
		out << "struct GoldenCheckParam { float cmd[3]; float maxspeed; float clientmaxspeed; float expected_cmd[3]; float expected_speed; };\n";
		out << "static const GoldenCheckParam kGoldenCheckParam[] = {\n";
		for ( const auto &t : check_param_tests )
		{
			out << "    {{" << t.forwardmove << "f, " << t.sidemove << "f, " << t.upmove << "f}, "
			    << t.maxspeed << "f, " << t.clientmaxspeed << "f, {"
			    << t.expected_forward << "f, " << t.expected_side << "f, " << t.expected_up << "f}, "
			    << t.expected_speed << "f},\n";
		}
		out << "};\n\n";

		// Friction
		out << "struct GoldenFriction { float vel[3]; float friction; float stopspeed; float frametime; float expected_vel[3]; };\n";
		out << "static const GoldenFriction kGoldenFriction[] = {\n";
		for ( const auto &t : friction_tests )
		{
			out << "    {{" << t.initial_vel[0] << "f, " << t.initial_vel[1] << "f, " << t.initial_vel[2] << "f}, "
			    << t.friction << "f, " << t.stopspeed << "f, " << t.frametime << "f, {"
			    << t.expected_vel[0] << "f, " << t.expected_vel[1] << "f, " << t.expected_vel[2] << "f}},\n";
		}
		out << "};\n\n";

		// Accelerate
		out << "struct GoldenAccel { bool is_air; float vel[3]; float wishdir[3]; float wishspeed; float accel; float frametime; float expected_vel[3]; };\n";
		out << "static const GoldenAccel kGoldenAccel[] = {\n";
		for ( const auto &t : accel_tests )
		{
			out << "    {" << ( t.is_air ? "true" : "false" ) << ", {"
			    << t.initial_vel[0] << "f, " << t.initial_vel[1] << "f, " << t.initial_vel[2] << "f}, {"
			    << t.wishdir[0] << "f, " << t.wishdir[1] << "f, " << t.wishdir[2] << "f}, "
			    << t.wishspeed << "f, " << t.accel << "f, " << t.frametime << "f, {"
			    << t.expected_vel[0] << "f, " << t.expected_vel[1] << "f, " << t.expected_vel[2] << "f}},\n";
		}
		out << "};\n\n";

		// RadiusDamage
		out << "struct GoldenRadius { float src[3]; float end[3]; float damage; float radius; float expected; };\n";
		out << "static const GoldenRadius kGoldenRadius[] = {\n";
		for ( const auto &t : radius_tests )
		{
			out << "    {{" << t.src[0] << "f, " << t.src[1] << "f, " << t.src[2] << "f}, {"
			    << t.end[0] << "f, " << t.end[1] << "f, " << t.end[2] << "f}, "
			    << t.damage << "f, " << t.radius << "f, " << t.expected_damage << "f},\n";
		}
		out << "};\n\n";

		// Hitgroup
		out << "struct GoldenHitgroup { int hitgroup; const char* name; float input_damage; float monster_damage; float player_damage; };\n";
		out << "static const GoldenHitgroup kGoldenHitgroup[] = {\n";
		for ( const auto &t : hitgroup_tests )
		{
			out << "    {" << t.hitgroup << ", \"" << t.hitgroup_name << "\", "
			    << t.input_damage << "f, " << t.monster_damage << "f, " << t.player_damage << "f},\n";
		}
		out << "};\n\n";

		// Armor
		out << "struct GoldenArmor { const char* name; float damage; float initial_armor; int damage_type; bool is_multiplayer; float expected_damage; float expected_armor; };\n";
		out << "static const GoldenArmor kGoldenArmor[] = {\n";
		for ( const auto &t : armor_tests )
		{
			out << "    {\"" << t.description << "\", " << t.damage << "f, " << t.initial_armor << "f, "
			    << t.damage_type << ", " << ( t.is_multiplayer ? "true" : "false" ) << ", "
			    << t.expected_damage << "f, " << t.expected_armor << "f},\n";
		}
		out << "};\n";
	}
	std::cout << "Wrote tests/golden/golden_data.h" << std::endl;

	std::cout << "All golden files generated successfully." << std::endl;
	return 0;
}
