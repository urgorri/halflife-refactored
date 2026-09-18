#ifndef DAMAGE_DEFS_H
#define DAMAGE_DEFS_H

//=============================================================================
// Damage Type Bitmasks
// Shared across server game DLL, client HUD, and event prediction
//=============================================================================

// Instant damage
#define DMG_GENERIC 0              // Generic damage was done
#define DMG_CRUSH ( 1 << 0 )       // Crushed by falling or moving object
#define DMG_BULLET ( 1 << 1 )      // Shot
#define DMG_SLASH ( 1 << 2 )       // Cut, clawed, stabbed
#define DMG_BURN ( 1 << 3 )        // Heat burned
#define DMG_FREEZE ( 1 << 4 )      // Frozen
#define DMG_FALL ( 1 << 5 )        // Fell too far
#define DMG_BLAST ( 1 << 6 )       // Explosive blast damage
#define DMG_CLUB ( 1 << 7 )        // Crowbar, punch, headbutt
#define DMG_SHOCK ( 1 << 8 )       // Electric shock
#define DMG_SONIC ( 1 << 9 )       // Sound pulse shockwave
#define DMG_ENERGYBEAM ( 1 << 10 ) // Laser or other high energy beam
#define DMG_NEVERGIB ( 1 << 12 )   // No damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB ( 1 << 13 )  // Any damage type can be made to gib victims upon death

// Time-based damage
// Mask off TF-specific bits (24..31) and instant bits (0..13)
#define DMG_TIMEBASED ( ~( 0xff003fff ) )

#define DMG_DROWN ( 1 << 14 ) // Drowning
#define DMG_FIRSTTIMEBASED DMG_DROWN

#define DMG_PARALYZE ( 1 << 15 )     // Slows affected creature down
#define DMG_NERVEGAS ( 1 << 16 )     // Nerve toxins, very bad
#define DMG_POISON ( 1 << 17 )       // Blood poisoning
#define DMG_RADIATION ( 1 << 18 )    // Radiation exposure
#define DMG_DROWNRECOVER ( 1 << 19 ) // Drowning recovery
#define DMG_ACID ( 1 << 20 )         // Toxic chemicals or acid burns
#define DMG_SLOWBURN ( 1 << 21 )     // In an oven
#define DMG_SLOWFREEZE ( 1 << 22 )   // In a subzero freezer
#define DMG_MORTAR ( 1 << 23 )       // Hit by air raid (distinguishes grenade from mortar)

// Extended / Team Fortress damage types
#define DMG_IGNITE ( 1 << 24 )       // Players hit by this begin to burn
#define DMG_RADIUS_MAX ( 1 << 25 )   // Radius damage with this flag doesn't decrease over distance
#define DMG_RADIUS_QUAKE ( 1 << 26 ) // Radius damage is done like Quake. 1/2 damage at 1/2 radius
#define DMG_IGNOREARMOR ( 1 << 27 )  // Damage ignores target's armor
#define DMG_AIMED ( 1 << 28 )        // Does hit location damage
#define DMG_WALLPIERCING ( 1 << 29 ) // Blast damages ents through walls
#define DMG_CALTROP ( 1 << 30 )
#define DMG_HALLUC ( 1 << 31 )

// Damage types allowed to gib corpses
#define DMG_GIB_CORPSE ( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// Damage types that display HUD indicators
#define DMG_SHOWNHUD ( DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK )

// Standard continuous damage intervals and damages
#define PARALYZE_DURATION 2
#define PARALYZE_DAMAGE 1.0

#define NERVEGAS_DURATION 2
#define NERVEGAS_DAMAGE 5.0

#define POISON_DURATION 5
#define POISON_DAMAGE 2.0

#define RADIATION_DURATION 2
#define RADIATION_DAMAGE 1.0

#define ACID_DURATION 2
#define ACID_DAMAGE 5.0

#define SLOWBURN_DURATION 2
#define SLOWBURN_DAMAGE 1.0

#define SLOWFREEZE_DURATION 2
#define SLOWFREEZE_DAMAGE 1.0

#endif // DAMAGE_DEFS_H
