#pragma once

#include <cstdint>
#include <vector>

#include "extdll.h"

extern enginefuncs_t g_engfuncs;
extern globalvars_t *gpGlobals;

// Message recording buffers for testing engine network messages
extern std::vector<uint8_t> g_mockMessageBuffer;
extern int g_mockMessageDest;
extern int g_mockMessageType;
extern float g_mockMessageOrigin[3];
extern edict_t *g_mockMessageEdict;
extern TraceResult g_mockTraceResult;

void SetMockTraceLineResult( const TraceResult &tr );
void ResetMockEngine();
void InitMockEngine();
void UTIL_PrecacheOtherWeapon( const char *szClassname );
