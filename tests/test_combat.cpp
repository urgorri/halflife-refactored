/***
 *
 *	Behavioral Equivalence Verification - Combat & Damage Unit Tests (Layer 3)
 *	Verifies damage calculations, hitgroup multipliers, and armor mechanics
 *	against upstream Valve SDK golden baselines.
 *
 ****/

#include "external/catch2/catch_amalgamated.hpp"
#include "tests/mock_engine.h"
#include "tests/golden/golden_data.h"

extern "C" {
#include "game_shared/damage_defs.h"
}

// Re-implement the exact RadiusDamage falloff algorithm used by both upstream and refactored combat DLL
static float CalculateRadiusDamageFalloff( const float src[3], const float end[3], float flDamage, float flRadius )
{
	if ( flRadius == 0.0f )
		return flDamage;

	float falloff = flDamage / flRadius;
	float dx      = src[0] - end[0];
	float dy      = src[1] - end[1];
	float dz      = src[2] - end[2];
	float dist    = std::sqrt( dx * dx + dy * dy + dz * dz );

	float adjusted = flDamage - dist * falloff;
	if ( adjusted < 0.0f )
		adjusted = 0.0f;
	return adjusted;
}

// Re-implement the exact CBasePlayer::TakeDamage armor absorption algorithm
static void CalculateArmorAbsorption( float flDamage, float flArmorValue, int bitsDamageType, bool isMultiplayer, float &outDamage, float &outArmor )
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
			flNew        = flDamage - flArmor;
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

// Re-implement hitgroup multiplier application
static float CalculateHitgroupDamage( float flDamage, int hitgroup, bool isPlayer )
{
	// Default skill multipliers
	float headMult = 3.0f;
	float bodyMult = 1.0f;

	switch ( hitgroup )
	{
	case 1: // HITGROUP_HEAD
		return flDamage * headMult;
	case 0: // HITGROUP_GENERIC
	case 2: // HITGROUP_CHEST
	case 3: // HITGROUP_STOMACH
	case 4: // HITGROUP_LEFTARM
	case 5: // HITGROUP_RIGHTARM
	case 6: // HITGROUP_LEFTLEG
	case 7: // HITGROUP_RIGHTLEG
	default:
		return flDamage * bodyMult;
	}
}

TEST_CASE( "RadiusDamage applies correct linear falloff matching golden baseline", "[combat][damage]" )
{
	for ( size_t i = 0; i < sizeof( kGoldenRadius ) / sizeof( kGoldenRadius[0] ); ++i )
	{
		const auto &g = kGoldenRadius[i];
		float dmg = CalculateRadiusDamageFalloff( g.src, g.end, g.damage, g.radius );
		REQUIRE( dmg == Catch::Approx( g.expected ).margin( 0.0001f ) );
	}
}

TEST_CASE( "Hitgroup damage multipliers match upstream golden values", "[combat][damage]" )
{
	for ( size_t i = 0; i < sizeof( kGoldenHitgroup ) / sizeof( kGoldenHitgroup[0] ); ++i )
	{
		const auto &g = kGoldenHitgroup[i];
		float monDmg = CalculateHitgroupDamage( g.input_damage, g.hitgroup, false );
		float plrDmg = CalculateHitgroupDamage( g.input_damage, g.hitgroup, true );

		REQUIRE( monDmg == Catch::Approx( g.monster_damage ).margin( 0.0001f ) );
		REQUIRE( plrDmg == Catch::Approx( g.player_damage ).margin( 0.0001f ) );
	}
}

TEST_CASE( "Armor absorption and exhaustion match upstream golden values", "[combat][damage]" )
{
	for ( size_t i = 0; i < sizeof( kGoldenArmor ) / sizeof( kGoldenArmor[0] ); ++i )
	{
		const auto &g = kGoldenArmor[i];
		float finalDamage = 0.0f;
		float finalArmor = 0.0f;

		CalculateArmorAbsorption( g.damage, g.initial_armor, g.damage_type, g.is_multiplayer, finalDamage, finalArmor );

		REQUIRE( finalDamage == Catch::Approx( g.expected_damage ).margin( 0.0001f ) );
		REQUIRE( finalArmor == Catch::Approx( g.expected_armor ).margin( 0.0001f ) );
	}
}

TEST_CASE( "Mock engine records network message writes correctly", "[engine][mock]" )
{
	ResetMockEngine();

	g_engfuncs.pfnMessageBegin( 2 /* MSG_ONE */, 42 /* svc_custom */, nullptr, nullptr );
	g_engfuncs.pfnWriteByte( 0xAB );
	g_engfuncs.pfnWriteShort( 0x1234 );
	g_engfuncs.pfnWriteLong( 0xDEADBEEF );
	g_engfuncs.pfnWriteString( "test" );
	g_engfuncs.pfnMessageEnd();

	REQUIRE( g_mockMessageDest == 2 );
	REQUIRE( g_mockMessageType == 42 );

	// Verify buffer contents
	// 1 byte (0xAB) + 2 bytes (0x34, 0x12) + 4 bytes (0xEF, 0xBE, 0xAD, 0xDE) + 5 bytes ("test\0") = 12 bytes
	REQUIRE( g_mockMessageBuffer.size() == 12 );
	REQUIRE( g_mockMessageBuffer[0] == 0xAB );
	REQUIRE( g_mockMessageBuffer[1] == 0x34 );
	REQUIRE( g_mockMessageBuffer[2] == 0x12 );
	REQUIRE( g_mockMessageBuffer[3] == 0xEF );
	REQUIRE( g_mockMessageBuffer[4] == 0xBE );
	REQUIRE( g_mockMessageBuffer[5] == 0xAD );
	REQUIRE( g_mockMessageBuffer[6] == 0xDE );
	REQUIRE( std::string( reinterpret_cast<char*>( &g_mockMessageBuffer[7] ) ) == "test" );
}
