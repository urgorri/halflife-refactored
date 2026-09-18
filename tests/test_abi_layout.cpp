/***
 *
 *	Behavioral Equivalence Verification - Layer 2A: ABI Struct Layout
 *
 *	Validates binary compatibility across the closed-source GoldSrc engine
 *	and game DLL boundary at compile time via static_assert checks.
 *
 ****/

#include <stddef.h>

#include "extdll.h"
#include "util.h"
#include "weapon_defs.h"
#include "pm_defs.h"
#include "pm_movevars.h"
#include "usercmd.h"
#include "entity_state.h"
#include "netadr.h"

// ============================================================================
// STRUCT SIZES (32-bit x86 ABI)
// ============================================================================

static_assert( sizeof( entvars_t ) == 676, "entvars_t size mismatch with engine ABI" );
static_assert( sizeof( edict_t ) == 804, "edict_t size mismatch with engine ABI" );
static_assert( sizeof( globalvars_t ) == 172, "globalvars_t size mismatch with engine ABI" );
static_assert( sizeof( enginefuncs_t ) == 636, "enginefuncs_t size mismatch with engine ABI" );
static_assert( sizeof( DLL_FUNCTIONS ) == 200, "DLL_FUNCTIONS size mismatch with engine ABI" );
static_assert( sizeof( NEW_DLL_FUNCTIONS ) == 20, "NEW_DLL_FUNCTIONS size mismatch with engine ABI" );
static_assert( sizeof( playermove_t ) == 325068, "playermove_t size mismatch with engine ABI" );
static_assert( sizeof( usercmd_t ) == 52, "usercmd_t size mismatch with engine ABI" );
static_assert( sizeof( ItemInfo ) == 44, "ItemInfo size mismatch with engine ABI" );
static_assert( sizeof( TraceResult ) == 56, "TraceResult size mismatch with engine ABI" );
static_assert( sizeof( KeyValueData ) == 16, "KeyValueData size mismatch with engine ABI" );
static_assert( sizeof( physent_t ) == 224, "physent_t size mismatch with engine ABI" );
static_assert( sizeof( pmtrace_t ) == 68, "pmtrace_t size mismatch with engine ABI" );
static_assert( sizeof( netadr_t ) == 20, "netadr_t size mismatch with engine ABI" );
static_assert( sizeof( clientdata_t ) == 476, "clientdata_t size mismatch with engine ABI" );
static_assert( sizeof( weapon_data_t ) == 88, "weapon_data_t size mismatch with engine ABI" );
static_assert( sizeof( entity_state_t ) == 340, "entity_state_t size mismatch with engine ABI" );
static_assert( sizeof( movevars_t ) == 132, "movevars_t size mismatch with engine ABI" );

// ============================================================================
// FIELD OFFSETS - entvars_t
// ============================================================================

static_assert( offsetof( entvars_t, classname ) == 0, "entvars_t::classname offset shifted" );
static_assert( offsetof( entvars_t, globalname ) == 4, "entvars_t::globalname offset shifted" );
static_assert( offsetof( entvars_t, origin ) == 8, "entvars_t::origin offset shifted" );
static_assert( offsetof( entvars_t, oldorigin ) == 20, "entvars_t::oldorigin offset shifted" );
static_assert( offsetof( entvars_t, velocity ) == 32, "entvars_t::velocity offset shifted" );
static_assert( offsetof( entvars_t, basevelocity ) == 44, "entvars_t::basevelocity offset shifted" );
static_assert( offsetof( entvars_t, clbasevelocity ) == 56, "entvars_t::clbasevelocity offset shifted" );
static_assert( offsetof( entvars_t, movedir ) == 68, "entvars_t::movedir offset shifted" );
static_assert( offsetof( entvars_t, angles ) == 80, "entvars_t::angles offset shifted" );
static_assert( offsetof( entvars_t, avelocity ) == 92, "entvars_t::avelocity offset shifted" );
static_assert( offsetof( entvars_t, punchangle ) == 104, "entvars_t::punchangle offset shifted" );
static_assert( offsetof( entvars_t, v_angle ) == 116, "entvars_t::v_angle offset shifted" );
static_assert( offsetof( entvars_t, endpos ) == 128, "entvars_t::endpos offset shifted" );
static_assert( offsetof( entvars_t, startpos ) == 140, "entvars_t::startpos offset shifted" );
static_assert( offsetof( entvars_t, impacttime ) == 152, "entvars_t::impacttime offset shifted" );
static_assert( offsetof( entvars_t, starttime ) == 156, "entvars_t::starttime offset shifted" );
static_assert( offsetof( entvars_t, fixangle ) == 160, "entvars_t::fixangle offset shifted" );
static_assert( offsetof( entvars_t, idealpitch ) == 164, "entvars_t::idealpitch offset shifted" );
static_assert( offsetof( entvars_t, pitch_speed ) == 168, "entvars_t::pitch_speed offset shifted" );
static_assert( offsetof( entvars_t, ideal_yaw ) == 172, "entvars_t::ideal_yaw offset shifted" );
static_assert( offsetof( entvars_t, yaw_speed ) == 176, "entvars_t::yaw_speed offset shifted" );
static_assert( offsetof( entvars_t, modelindex ) == 180, "entvars_t::modelindex offset shifted" );
static_assert( offsetof( entvars_t, model ) == 184, "entvars_t::model offset shifted" );
static_assert( offsetof( entvars_t, viewmodel ) == 188, "entvars_t::viewmodel offset shifted" );
static_assert( offsetof( entvars_t, weaponmodel ) == 192, "entvars_t::weaponmodel offset shifted" );
static_assert( offsetof( entvars_t, absmin ) == 196, "entvars_t::absmin offset shifted" );
static_assert( offsetof( entvars_t, absmax ) == 208, "entvars_t::absmax offset shifted" );
static_assert( offsetof( entvars_t, mins ) == 220, "entvars_t::mins offset shifted" );
static_assert( offsetof( entvars_t, maxs ) == 232, "entvars_t::maxs offset shifted" );
static_assert( offsetof( entvars_t, size ) == 244, "entvars_t::size offset shifted" );
static_assert( offsetof( entvars_t, ltime ) == 256, "entvars_t::ltime offset shifted" );
static_assert( offsetof( entvars_t, nextthink ) == 260, "entvars_t::nextthink offset shifted" );
static_assert( offsetof( entvars_t, movetype ) == 264, "entvars_t::movetype offset shifted" );
static_assert( offsetof( entvars_t, solid ) == 268, "entvars_t::solid offset shifted" );
static_assert( offsetof( entvars_t, skin ) == 272, "entvars_t::skin offset shifted" );
static_assert( offsetof( entvars_t, body ) == 276, "entvars_t::body offset shifted" );
static_assert( offsetof( entvars_t, effects ) == 280, "entvars_t::effects offset shifted" );
static_assert( offsetof( entvars_t, gravity ) == 284, "entvars_t::gravity offset shifted" );
static_assert( offsetof( entvars_t, friction ) == 288, "entvars_t::friction offset shifted" );
static_assert( offsetof( entvars_t, light_level ) == 292, "entvars_t::light_level offset shifted" );
static_assert( offsetof( entvars_t, sequence ) == 296, "entvars_t::sequence offset shifted" );
static_assert( offsetof( entvars_t, gaitsequence ) == 300, "entvars_t::gaitsequence offset shifted" );
static_assert( offsetof( entvars_t, frame ) == 304, "entvars_t::frame offset shifted" );
static_assert( offsetof( entvars_t, animtime ) == 308, "entvars_t::animtime offset shifted" );
static_assert( offsetof( entvars_t, framerate ) == 312, "entvars_t::framerate offset shifted" );
static_assert( offsetof( entvars_t, controller ) == 316, "entvars_t::controller offset shifted" );
static_assert( offsetof( entvars_t, blending ) == 320, "entvars_t::blending offset shifted" );
static_assert( offsetof( entvars_t, scale ) == 324, "entvars_t::scale offset shifted" );
static_assert( offsetof( entvars_t, rendermode ) == 328, "entvars_t::rendermode offset shifted" );
static_assert( offsetof( entvars_t, renderamt ) == 332, "entvars_t::renderamt offset shifted" );
static_assert( offsetof( entvars_t, rendercolor ) == 336, "entvars_t::rendercolor offset shifted" );
static_assert( offsetof( entvars_t, renderfx ) == 348, "entvars_t::renderfx offset shifted" );
static_assert( offsetof( entvars_t, health ) == 352, "entvars_t::health offset shifted" );
static_assert( offsetof( entvars_t, frags ) == 356, "entvars_t::frags offset shifted" );
static_assert( offsetof( entvars_t, weapons ) == 360, "entvars_t::weapons offset shifted" );
static_assert( offsetof( entvars_t, takedamage ) == 364, "entvars_t::takedamage offset shifted" );
static_assert( offsetof( entvars_t, deadflag ) == 368, "entvars_t::deadflag offset shifted" );
static_assert( offsetof( entvars_t, view_ofs ) == 372, "entvars_t::view_ofs offset shifted" );
static_assert( offsetof( entvars_t, button ) == 384, "entvars_t::button offset shifted" );
static_assert( offsetof( entvars_t, impulse ) == 388, "entvars_t::impulse offset shifted" );
static_assert( offsetof( entvars_t, chain ) == 392, "entvars_t::chain offset shifted" );
static_assert( offsetof( entvars_t, dmg_inflictor ) == 396, "entvars_t::dmg_inflictor offset shifted" );
static_assert( offsetof( entvars_t, enemy ) == 400, "entvars_t::enemy offset shifted" );
static_assert( offsetof( entvars_t, aiment ) == 404, "entvars_t::aiment offset shifted" );
static_assert( offsetof( entvars_t, owner ) == 408, "entvars_t::owner offset shifted" );
static_assert( offsetof( entvars_t, groundentity ) == 412, "entvars_t::groundentity offset shifted" );
static_assert( offsetof( entvars_t, spawnflags ) == 416, "entvars_t::spawnflags offset shifted" );
static_assert( offsetof( entvars_t, flags ) == 420, "entvars_t::flags offset shifted" );
static_assert( offsetof( entvars_t, colormap ) == 424, "entvars_t::colormap offset shifted" );
static_assert( offsetof( entvars_t, team ) == 428, "entvars_t::team offset shifted" );
static_assert( offsetof( entvars_t, max_health ) == 432, "entvars_t::max_health offset shifted" );
static_assert( offsetof( entvars_t, teleport_time ) == 436, "entvars_t::teleport_time offset shifted" );
static_assert( offsetof( entvars_t, armortype ) == 440, "entvars_t::armortype offset shifted" );
static_assert( offsetof( entvars_t, armorvalue ) == 444, "entvars_t::armorvalue offset shifted" );
static_assert( offsetof( entvars_t, waterlevel ) == 448, "entvars_t::waterlevel offset shifted" );
static_assert( offsetof( entvars_t, watertype ) == 452, "entvars_t::watertype offset shifted" );
static_assert( offsetof( entvars_t, target ) == 456, "entvars_t::target offset shifted" );
static_assert( offsetof( entvars_t, targetname ) == 460, "entvars_t::targetname offset shifted" );
static_assert( offsetof( entvars_t, netname ) == 464, "entvars_t::netname offset shifted" );
static_assert( offsetof( entvars_t, message ) == 468, "entvars_t::message offset shifted" );
static_assert( offsetof( entvars_t, dmg_take ) == 472, "entvars_t::dmg_take offset shifted" );
static_assert( offsetof( entvars_t, dmg_save ) == 476, "entvars_t::dmg_save offset shifted" );
static_assert( offsetof( entvars_t, dmg ) == 480, "entvars_t::dmg offset shifted" );
static_assert( offsetof( entvars_t, dmgtime ) == 484, "entvars_t::dmgtime offset shifted" );
static_assert( offsetof( entvars_t, noise ) == 488, "entvars_t::noise offset shifted" );
static_assert( offsetof( entvars_t, noise1 ) == 492, "entvars_t::noise1 offset shifted" );
static_assert( offsetof( entvars_t, noise2 ) == 496, "entvars_t::noise2 offset shifted" );
static_assert( offsetof( entvars_t, noise3 ) == 500, "entvars_t::noise3 offset shifted" );
static_assert( offsetof( entvars_t, speed ) == 504, "entvars_t::speed offset shifted" );
static_assert( offsetof( entvars_t, air_finished ) == 508, "entvars_t::air_finished offset shifted" );
static_assert( offsetof( entvars_t, pain_finished ) == 512, "entvars_t::pain_finished offset shifted" );
static_assert( offsetof( entvars_t, radsuit_finished ) == 516, "entvars_t::radsuit_finished offset shifted" );
static_assert( offsetof( entvars_t, pContainingEntity ) == 520, "entvars_t::pContainingEntity offset shifted" );
static_assert( offsetof( entvars_t, playerclass ) == 524, "entvars_t::playerclass offset shifted" );
static_assert( offsetof( entvars_t, maxspeed ) == 528, "entvars_t::maxspeed offset shifted" );
static_assert( offsetof( entvars_t, fov ) == 532, "entvars_t::fov offset shifted" );
static_assert( offsetof( entvars_t, weaponanim ) == 536, "entvars_t::weaponanim offset shifted" );
static_assert( offsetof( entvars_t, pushmsec ) == 540, "entvars_t::pushmsec offset shifted" );
static_assert( offsetof( entvars_t, bInDuck ) == 544, "entvars_t::bInDuck offset shifted" );
static_assert( offsetof( entvars_t, flTimeStepSound ) == 548, "entvars_t::flTimeStepSound offset shifted" );
static_assert( offsetof( entvars_t, flSwimTime ) == 552, "entvars_t::flSwimTime offset shifted" );
static_assert( offsetof( entvars_t, flDuckTime ) == 556, "entvars_t::flDuckTime offset shifted" );
static_assert( offsetof( entvars_t, iStepLeft ) == 560, "entvars_t::iStepLeft offset shifted" );
static_assert( offsetof( entvars_t, flFallVelocity ) == 564, "entvars_t::flFallVelocity offset shifted" );
static_assert( offsetof( entvars_t, gamestate ) == 568, "entvars_t::gamestate offset shifted" );
static_assert( offsetof( entvars_t, oldbuttons ) == 572, "entvars_t::oldbuttons offset shifted" );
static_assert( offsetof( entvars_t, groupinfo ) == 576, "entvars_t::groupinfo offset shifted" );
static_assert( offsetof( entvars_t, iuser1 ) == 580, "entvars_t::iuser1 offset shifted" );
static_assert( offsetof( entvars_t, iuser2 ) == 584, "entvars_t::iuser2 offset shifted" );
static_assert( offsetof( entvars_t, iuser3 ) == 588, "entvars_t::iuser3 offset shifted" );
static_assert( offsetof( entvars_t, iuser4 ) == 592, "entvars_t::iuser4 offset shifted" );
static_assert( offsetof( entvars_t, fuser1 ) == 596, "entvars_t::fuser1 offset shifted" );
static_assert( offsetof( entvars_t, fuser2 ) == 600, "entvars_t::fuser2 offset shifted" );
static_assert( offsetof( entvars_t, fuser3 ) == 604, "entvars_t::fuser3 offset shifted" );
static_assert( offsetof( entvars_t, fuser4 ) == 608, "entvars_t::fuser4 offset shifted" );
static_assert( offsetof( entvars_t, vuser1 ) == 612, "entvars_t::vuser1 offset shifted" );
static_assert( offsetof( entvars_t, vuser2 ) == 624, "entvars_t::vuser2 offset shifted" );
static_assert( offsetof( entvars_t, vuser3 ) == 636, "entvars_t::vuser3 offset shifted" );
static_assert( offsetof( entvars_t, vuser4 ) == 648, "entvars_t::vuser4 offset shifted" );
static_assert( offsetof( entvars_t, euser1 ) == 660, "entvars_t::euser1 offset shifted" );
static_assert( offsetof( entvars_t, euser2 ) == 664, "entvars_t::euser2 offset shifted" );
static_assert( offsetof( entvars_t, euser3 ) == 668, "entvars_t::euser3 offset shifted" );
static_assert( offsetof( entvars_t, euser4 ) == 672, "entvars_t::euser4 offset shifted" );

// ============================================================================
// FIELD OFFSETS - edict_t
// ============================================================================

static_assert( offsetof( edict_t, free ) == 0, "edict_t::free offset shifted" );
static_assert( offsetof( edict_t, serialnumber ) == 4, "edict_t::serialnumber offset shifted" );
static_assert( offsetof( edict_t, area ) == 8, "edict_t::area offset shifted" );
static_assert( offsetof( edict_t, headnode ) == 16, "edict_t::headnode offset shifted" );
static_assert( offsetof( edict_t, num_leafs ) == 20, "edict_t::num_leafs offset shifted" );
static_assert( offsetof( edict_t, leafnums ) == 24, "edict_t::leafnums offset shifted" );
static_assert( offsetof( edict_t, freetime ) == 120, "edict_t::freetime offset shifted" );
static_assert( offsetof( edict_t, pvPrivateData ) == 124, "edict_t::pvPrivateData offset shifted" );
static_assert( offsetof( edict_t, v ) == 128, "edict_t::v offset shifted" );

// ============================================================================
// FIELD OFFSETS - globalvars_t
// ============================================================================

static_assert( offsetof( globalvars_t, time ) == 0, "globalvars_t::time offset shifted" );
static_assert( offsetof( globalvars_t, frametime ) == 4, "globalvars_t::frametime offset shifted" );
static_assert( offsetof( globalvars_t, force_retouch ) == 8, "globalvars_t::force_retouch offset shifted" );
static_assert( offsetof( globalvars_t, mapname ) == 12, "globalvars_t::mapname offset shifted" );
static_assert( offsetof( globalvars_t, startspot ) == 16, "globalvars_t::startspot offset shifted" );
static_assert( offsetof( globalvars_t, deathmatch ) == 20, "globalvars_t::deathmatch offset shifted" );
static_assert( offsetof( globalvars_t, coop ) == 24, "globalvars_t::coop offset shifted" );
static_assert( offsetof( globalvars_t, teamplay ) == 28, "globalvars_t::teamplay offset shifted" );
static_assert( offsetof( globalvars_t, serverflags ) == 32, "globalvars_t::serverflags offset shifted" );
static_assert( offsetof( globalvars_t, found_secrets ) == 36, "globalvars_t::found_secrets offset shifted" );
static_assert( offsetof( globalvars_t, v_forward ) == 40, "globalvars_t::v_forward offset shifted" );
static_assert( offsetof( globalvars_t, v_up ) == 52, "globalvars_t::v_up offset shifted" );
static_assert( offsetof( globalvars_t, v_right ) == 64, "globalvars_t::v_right offset shifted" );
static_assert( offsetof( globalvars_t, trace_allsolid ) == 76, "globalvars_t::trace_allsolid offset shifted" );
static_assert( offsetof( globalvars_t, trace_startsolid ) == 80, "globalvars_t::trace_startsolid offset shifted" );
static_assert( offsetof( globalvars_t, trace_fraction ) == 84, "globalvars_t::trace_fraction offset shifted" );
static_assert( offsetof( globalvars_t, trace_endpos ) == 88, "globalvars_t::trace_endpos offset shifted" );
static_assert( offsetof( globalvars_t, trace_plane_normal ) == 100, "globalvars_t::trace_plane_normal offset shifted" );
static_assert( offsetof( globalvars_t, trace_plane_dist ) == 112, "globalvars_t::trace_plane_dist offset shifted" );
static_assert( offsetof( globalvars_t, trace_ent ) == 116, "globalvars_t::trace_ent offset shifted" );
static_assert( offsetof( globalvars_t, trace_inopen ) == 120, "globalvars_t::trace_inopen offset shifted" );
static_assert( offsetof( globalvars_t, trace_inwater ) == 124, "globalvars_t::trace_inwater offset shifted" );
static_assert( offsetof( globalvars_t, trace_hitgroup ) == 128, "globalvars_t::trace_hitgroup offset shifted" );
static_assert( offsetof( globalvars_t, trace_flags ) == 132, "globalvars_t::trace_flags offset shifted" );
static_assert( offsetof( globalvars_t, msg_entity ) == 136, "globalvars_t::msg_entity offset shifted" );
static_assert( offsetof( globalvars_t, cdAudioTrack ) == 140, "globalvars_t::cdAudioTrack offset shifted" );
static_assert( offsetof( globalvars_t, maxClients ) == 144, "globalvars_t::maxClients offset shifted" );
static_assert( offsetof( globalvars_t, maxEntities ) == 148, "globalvars_t::maxEntities offset shifted" );
static_assert( offsetof( globalvars_t, pStringBase ) == 152, "globalvars_t::pStringBase offset shifted" );

// ============================================================================
// FIELD OFFSETS - playermove_t
// ============================================================================

static_assert( offsetof( playermove_t, player_index ) == 0, "playermove_t::player_index offset shifted" );
static_assert( offsetof( playermove_t, server ) == 4, "playermove_t::server offset shifted" );
static_assert( offsetof( playermove_t, multiplayer ) == 8, "playermove_t::multiplayer offset shifted" );
static_assert( offsetof( playermove_t, time ) == 12, "playermove_t::time offset shifted" );
static_assert( offsetof( playermove_t, frametime ) == 16, "playermove_t::frametime offset shifted" );
static_assert( offsetof( playermove_t, forward ) == 20, "playermove_t::forward offset shifted" );
static_assert( offsetof( playermove_t, right ) == 32, "playermove_t::right offset shifted" );
static_assert( offsetof( playermove_t, up ) == 44, "playermove_t::up offset shifted" );
static_assert( offsetof( playermove_t, origin ) == 56, "playermove_t::origin offset shifted" );
static_assert( offsetof( playermove_t, angles ) == 68, "playermove_t::angles offset shifted" );
static_assert( offsetof( playermove_t, oldangles ) == 80, "playermove_t::oldangles offset shifted" );
static_assert( offsetof( playermove_t, velocity ) == 92, "playermove_t::velocity offset shifted" );
static_assert( offsetof( playermove_t, movedir ) == 104, "playermove_t::movedir offset shifted" );
static_assert( offsetof( playermove_t, basevelocity ) == 116, "playermove_t::basevelocity offset shifted" );
static_assert( offsetof( playermove_t, view_ofs ) == 128, "playermove_t::view_ofs offset shifted" );
static_assert( offsetof( playermove_t, flDuckTime ) == 140, "playermove_t::flDuckTime offset shifted" );
static_assert( offsetof( playermove_t, bInDuck ) == 144, "playermove_t::bInDuck offset shifted" );
static_assert( offsetof( playermove_t, flTimeStepSound ) == 148, "playermove_t::flTimeStepSound offset shifted" );
static_assert( offsetof( playermove_t, iStepLeft ) == 152, "playermove_t::iStepLeft offset shifted" );
static_assert( offsetof( playermove_t, flFallVelocity ) == 156, "playermove_t::flFallVelocity offset shifted" );
static_assert( offsetof( playermove_t, punchangle ) == 160, "playermove_t::punchangle offset shifted" );
static_assert( offsetof( playermove_t, flSwimTime ) == 172, "playermove_t::flSwimTime offset shifted" );
static_assert( offsetof( playermove_t, flNextPrimaryAttack ) == 176, "playermove_t::flNextPrimaryAttack offset shifted" );
static_assert( offsetof( playermove_t, effects ) == 180, "playermove_t::effects offset shifted" );
static_assert( offsetof( playermove_t, flags ) == 184, "playermove_t::flags offset shifted" );
static_assert( offsetof( playermove_t, usehull ) == 188, "playermove_t::usehull offset shifted" );
static_assert( offsetof( playermove_t, gravity ) == 192, "playermove_t::gravity offset shifted" );
static_assert( offsetof( playermove_t, friction ) == 196, "playermove_t::friction offset shifted" );
static_assert( offsetof( playermove_t, oldbuttons ) == 200, "playermove_t::oldbuttons offset shifted" );
static_assert( offsetof( playermove_t, waterjumptime ) == 204, "playermove_t::waterjumptime offset shifted" );
static_assert( offsetof( playermove_t, dead ) == 208, "playermove_t::dead offset shifted" );
static_assert( offsetof( playermove_t, deadflag ) == 212, "playermove_t::deadflag offset shifted" );
static_assert( offsetof( playermove_t, spectator ) == 216, "playermove_t::spectator offset shifted" );
static_assert( offsetof( playermove_t, movetype ) == 220, "playermove_t::movetype offset shifted" );
static_assert( offsetof( playermove_t, onground ) == 224, "playermove_t::onground offset shifted" );
static_assert( offsetof( playermove_t, waterlevel ) == 228, "playermove_t::waterlevel offset shifted" );
static_assert( offsetof( playermove_t, watertype ) == 232, "playermove_t::watertype offset shifted" );
static_assert( offsetof( playermove_t, sztexturename ) == 240, "playermove_t::sztexturename offset shifted" );
static_assert( offsetof( playermove_t, chtexturetype ) == 496, "playermove_t::chtexturetype offset shifted" );
static_assert( offsetof( playermove_t, maxspeed ) == 500, "playermove_t::maxspeed offset shifted" );
static_assert( offsetof( playermove_t, clientmaxspeed ) == 504, "playermove_t::clientmaxspeed offset shifted" );
static_assert( offsetof( playermove_t, iuser1 ) == 508, "playermove_t::iuser1 offset shifted" );
static_assert( offsetof( playermove_t, iuser2 ) == 512, "playermove_t::iuser2 offset shifted" );
static_assert( offsetof( playermove_t, iuser3 ) == 516, "playermove_t::iuser3 offset shifted" );
static_assert( offsetof( playermove_t, iuser4 ) == 520, "playermove_t::iuser4 offset shifted" );
static_assert( offsetof( playermove_t, fuser1 ) == 524, "playermove_t::fuser1 offset shifted" );
static_assert( offsetof( playermove_t, fuser2 ) == 528, "playermove_t::fuser2 offset shifted" );
static_assert( offsetof( playermove_t, fuser3 ) == 532, "playermove_t::fuser3 offset shifted" );
static_assert( offsetof( playermove_t, fuser4 ) == 536, "playermove_t::fuser4 offset shifted" );
static_assert( offsetof( playermove_t, vuser1 ) == 540, "playermove_t::vuser1 offset shifted" );
static_assert( offsetof( playermove_t, vuser2 ) == 552, "playermove_t::vuser2 offset shifted" );
static_assert( offsetof( playermove_t, vuser3 ) == 564, "playermove_t::vuser3 offset shifted" );
static_assert( offsetof( playermove_t, vuser4 ) == 576, "playermove_t::vuser4 offset shifted" );
static_assert( offsetof( playermove_t, numphysent ) == 588, "playermove_t::numphysent offset shifted" );
static_assert( offsetof( playermove_t, physents ) == 592, "playermove_t::physents offset shifted" );
static_assert( offsetof( playermove_t, nummoveent ) == 134992, "playermove_t::nummoveent offset shifted" );
static_assert( offsetof( playermove_t, moveents ) == 134996, "playermove_t::moveents offset shifted" );
static_assert( offsetof( playermove_t, numvisent ) == 149332, "playermove_t::numvisent offset shifted" );
static_assert( offsetof( playermove_t, visents ) == 149336, "playermove_t::visents offset shifted" );
static_assert( offsetof( playermove_t, cmd ) == 283736, "playermove_t::cmd offset shifted" );
static_assert( offsetof( playermove_t, movevars ) == 324848, "playermove_t::movevars offset shifted" );
static_assert( offsetof( playermove_t, PM_Info_ValueForKey ) == 324948, "playermove_t::PM_Info_ValueForKey offset shifted" );
static_assert( offsetof( playermove_t, PM_Particle ) == 324952, "playermove_t::PM_Particle offset shifted" );
static_assert( offsetof( playermove_t, PM_TestPlayerPosition ) == 324956, "playermove_t::PM_TestPlayerPosition offset shifted" );
static_assert( offsetof( playermove_t, PM_PlaySound ) == 325044, "playermove_t::PM_PlaySound offset shifted" );
static_assert( offsetof( playermove_t, PM_TraceTexture ) == 325048, "playermove_t::PM_TraceTexture offset shifted" );

// ============================================================================
// FIELD OFFSETS - usercmd_t
// ============================================================================

static_assert( offsetof( usercmd_t, lerp_msec ) == 0, "usercmd_t::lerp_msec offset shifted" );
static_assert( offsetof( usercmd_t, msec ) == 2, "usercmd_t::msec offset shifted" );
static_assert( offsetof( usercmd_t, viewangles ) == 4, "usercmd_t::viewangles offset shifted" );
static_assert( offsetof( usercmd_t, forwardmove ) == 16, "usercmd_t::forwardmove offset shifted" );
static_assert( offsetof( usercmd_t, sidemove ) == 20, "usercmd_t::sidemove offset shifted" );
static_assert( offsetof( usercmd_t, upmove ) == 24, "usercmd_t::upmove offset shifted" );
static_assert( offsetof( usercmd_t, lightlevel ) == 28, "usercmd_t::lightlevel offset shifted" );
static_assert( offsetof( usercmd_t, buttons ) == 30, "usercmd_t::buttons offset shifted" );
static_assert( offsetof( usercmd_t, impulse ) == 32, "usercmd_t::impulse offset shifted" );
static_assert( offsetof( usercmd_t, weaponselect ) == 33, "usercmd_t::weaponselect offset shifted" );
static_assert( offsetof( usercmd_t, impact_index ) == 36, "usercmd_t::impact_index offset shifted" );
static_assert( offsetof( usercmd_t, impact_position ) == 40, "usercmd_t::impact_position offset shifted" );

// ============================================================================
// FIELD OFFSETS - ItemInfo
// ============================================================================

static_assert( offsetof( ItemInfo, iSlot ) == 0, "ItemInfo::iSlot offset shifted" );
static_assert( offsetof( ItemInfo, iPosition ) == 4, "ItemInfo::iPosition offset shifted" );
static_assert( offsetof( ItemInfo, pszAmmo1 ) == 8, "ItemInfo::pszAmmo1 offset shifted" );
static_assert( offsetof( ItemInfo, iMaxAmmo1 ) == 12, "ItemInfo::iMaxAmmo1 offset shifted" );
static_assert( offsetof( ItemInfo, pszAmmo2 ) == 16, "ItemInfo::pszAmmo2 offset shifted" );
static_assert( offsetof( ItemInfo, iMaxAmmo2 ) == 20, "ItemInfo::iMaxAmmo2 offset shifted" );
static_assert( offsetof( ItemInfo, pszName ) == 24, "ItemInfo::pszName offset shifted" );
static_assert( offsetof( ItemInfo, iMaxClip ) == 28, "ItemInfo::iMaxClip offset shifted" );
static_assert( offsetof( ItemInfo, iId ) == 32, "ItemInfo::iId offset shifted" );
static_assert( offsetof( ItemInfo, iFlags ) == 36, "ItemInfo::iFlags offset shifted" );
static_assert( offsetof( ItemInfo, iWeight ) == 40, "ItemInfo::iWeight offset shifted" );

// ============================================================================
// FIELD OFFSETS - TraceResult
// ============================================================================

static_assert( offsetof( TraceResult, fAllSolid ) == 0, "TraceResult::fAllSolid offset shifted" );
static_assert( offsetof( TraceResult, fStartSolid ) == 4, "TraceResult::fStartSolid offset shifted" );
static_assert( offsetof( TraceResult, fInOpen ) == 8, "TraceResult::fInOpen offset shifted" );
static_assert( offsetof( TraceResult, fInWater ) == 12, "TraceResult::fInWater offset shifted" );
static_assert( offsetof( TraceResult, flFraction ) == 16, "TraceResult::flFraction offset shifted" );
static_assert( offsetof( TraceResult, vecEndPos ) == 20, "TraceResult::vecEndPos offset shifted" );
static_assert( offsetof( TraceResult, flPlaneDist ) == 32, "TraceResult::flPlaneDist offset shifted" );
static_assert( offsetof( TraceResult, vecPlaneNormal ) == 36, "TraceResult::vecPlaneNormal offset shifted" );
static_assert( offsetof( TraceResult, pHit ) == 48, "TraceResult::pHit offset shifted" );
static_assert( offsetof( TraceResult, iHitgroup ) == 52, "TraceResult::iHitgroup offset shifted" );

// ============================================================================
// FIELD OFFSETS - KeyValueData
// ============================================================================

static_assert( offsetof( KeyValueData, szClassName ) == 0, "KeyValueData::szClassName offset shifted" );
static_assert( offsetof( KeyValueData, szKeyName ) == 4, "KeyValueData::szKeyName offset shifted" );
static_assert( offsetof( KeyValueData, szValue ) == 8, "KeyValueData::szValue offset shifted" );
static_assert( offsetof( KeyValueData, fHandled ) == 12, "KeyValueData::fHandled offset shifted" );
