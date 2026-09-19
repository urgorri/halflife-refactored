/***
 *
 *	Behavioral Equivalence Verification - Mock Engine Stubs
 *	Provides minimal enginefuncs_t and gpGlobals implementations
 *	to allow testing game and movement logic without running HLDS.
 *
 ****/

#include "tests/mock_engine.h"
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <string>

// Global definitions expected by game DLL / pm_shared
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals = nullptr;
static globalvars_t s_mockGlobals;

// Message recording buffers
std::vector<uint8_t> g_mockMessageBuffer;
int g_mockMessageDest = 0;
int g_mockMessageType = 0;
float g_mockMessageOrigin[3] = {0, 0, 0};
edict_t *g_mockMessageEdict = nullptr;
TraceResult g_mockTraceResult;

static std::vector<std::string> s_stringPool;
static std::unordered_map<std::string, int> s_stringMap;

// Default stub implementations
static int stub_PrecacheModel( char *s ) { return 1; }
static int stub_PrecacheSound( char *s ) { return 1; }
static void stub_SetModel( edict_t *e, const char *m ) {}
static int stub_ModelIndex( const char *m ) { return 1; }
static int stub_ModelFrames( int modelIndex ) { return 1; }
static void stub_SetSize( edict_t *e, const float *rgflMin, const float *rgflMax ) {}
static void stub_ChangeLevel( char *s1, char *s2 ) {}
static void stub_GetSpawnParms( edict_t *ent ) {}
static void stub_SaveSpawnParms( edict_t *ent ) {}
static float stub_VecToYaw( const float *rgflVector ) { return 0.0f; }
static void stub_VecToAngles( const float *rgflVectorIn, float *rgflVectorOut ) {
	rgflVectorOut[0] = rgflVectorOut[1] = rgflVectorOut[2] = 0.0f;
}
static void stub_MoveToOrigin( edict_t *ent, const float *pflGoal, float dist, int iMoveType ) {}
static void stub_ChangeYaw( edict_t *ent ) {}
static void stub_ChangePitch( edict_t *ent ) {}
static edict_t *stub_FindEntityByString( edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue ) { return nullptr; }
static int stub_GetEntityIllum( edict_t *pEnt ) { return 128; }
static edict_t *stub_FindEntityInSphere( edict_t *pEdictStartSearchAfter, const float *org, float rad ) { return nullptr; }
static edict_t *stub_FindClientInPVS( edict_t *pEdict ) { return nullptr; }
static edict_t *stub_EntitiesInPVS( edict_t *pplayer ) { return nullptr; }
static void stub_MakeVectors( const float *rgflVector ) {}
static void stub_AngleVectors( const float *rgflVector, float *forward, float *right, float *up ) {
	if ( forward ) { forward[0] = 1; forward[1] = 0; forward[2] = 0; }
	if ( right ) { right[0] = 0; right[1] = 1; right[2] = 0; }
	if ( up ) { up[0] = 0; up[1] = 0; up[2] = 1; }
}
static edict_t *stub_CreateEntity( void ) { return nullptr; }
static void stub_RemoveEntity( edict_t *e ) {}
static edict_t *stub_CreateNamedEntity( int className ) { return nullptr; }
static void stub_MakeStatic( edict_t *ent ) {}
static int stub_EntIsOnFloor( edict_t *e ) { return 1; }
static int stub_DropToFloor( edict_t *e ) { return 1; }
static int stub_WalkMove( edict_t *ent, float yaw, float dist, int iMode ) { return 1; }
static void stub_SetOrigin( edict_t *e, const float *rgflOrigin ) {
	if ( e ) {
		e->v.origin[0] = rgflOrigin[0];
		e->v.origin[1] = rgflOrigin[1];
		e->v.origin[2] = rgflOrigin[2];
	}
}
static void stub_EmitSound( edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch ) {}
static void stub_EmitAmbientSound( edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch ) {}

static void stub_TraceLine( const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
		ptr->vecEndPos[0] = v2[0];
		ptr->vecEndPos[1] = v2[1];
		ptr->vecEndPos[2] = v2[2];
	}
}
static void stub_TraceToss( edict_t *pent, edict_t *pentToIgnore, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
	}
}
static int stub_TraceMonsterHull( edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
	}
	return 0;
}
static void stub_TraceHull( const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
	}
}
static void stub_TraceModel( const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
	}
}
static const char *stub_TraceTexture( edict_t *pTextureEntity, const float *v1, const float *v2 ) {
	return "default";
}
static void stub_TraceSphere( const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr ) {
	if ( ptr ) {
		*ptr = g_mockTraceResult;
	}
}
static void stub_GetAimVector( edict_t *ent, float speed, float *rgflReturn ) {
	if ( rgflReturn ) {
		rgflReturn[0] = 1; rgflReturn[1] = 0; rgflReturn[2] = 0;
	}
}
static void stub_ServerCommand( char *str ) {}
static void stub_ServerExecute( void ) {}
static void stub_ClientCommand( edict_t *pEdict, char *szFmt, ... ) {}
static void stub_ParticleEffect( const float *org, const float *dir, float color, float count ) {}
static void stub_LightStyle( int style, char *val ) {}
static int stub_DecalIndex( const char *name ) { return 1; }
static int stub_PointContents( const float *rgflVector ) { return 0; }

static void stub_MessageBegin( int msg_dest, int msg_type, const float *pOrigin, edict_t *ed ) {
	g_mockMessageBuffer.clear();
	g_mockMessageDest = msg_dest;
	g_mockMessageType = msg_type;
	if ( pOrigin ) {
		g_mockMessageOrigin[0] = pOrigin[0];
		g_mockMessageOrigin[1] = pOrigin[1];
		g_mockMessageOrigin[2] = pOrigin[2];
	}
	g_mockMessageEdict = ed;
}
static void stub_MessageEnd( void ) {}
static void stub_WriteByte( int iValue ) {
	g_mockMessageBuffer.push_back( static_cast<uint8_t>( iValue & 0xFF ) );
}
static void stub_WriteChar( int iValue ) {
	g_mockMessageBuffer.push_back( static_cast<uint8_t>( iValue & 0xFF ) );
}
static void stub_WriteShort( int iValue ) {
	g_mockMessageBuffer.push_back( static_cast<uint8_t>( iValue & 0xFF ) );
	g_mockMessageBuffer.push_back( static_cast<uint8_t>( ( iValue >> 8 ) & 0xFF ) );
}
static void stub_WriteLong( int iValue ) {
	for ( int i = 0; i < 4; ++i )
		g_mockMessageBuffer.push_back( static_cast<uint8_t>( ( iValue >> ( i * 8 ) ) & 0xFF ) );
}
static void stub_WriteAngle( float flValue ) {
	int val = static_cast<int>( ( flValue * 256.0f ) / 360.0f ) & 255;
	g_mockMessageBuffer.push_back( static_cast<uint8_t>( val ) );
}
static void stub_WriteCoord( float flValue ) {
	int val = static_cast<int>( flValue * 8.0f );
	stub_WriteShort( val );
}
static void stub_WriteString( const char *sz ) {
	if ( !sz ) return;
	while ( *sz ) {
		g_mockMessageBuffer.push_back( static_cast<uint8_t>( *sz++ ) );
	}
	g_mockMessageBuffer.push_back( 0 );
}
static void stub_WriteEntity( int iValue ) {
	stub_WriteShort( iValue );
}
static void stub_CVarRegister( cvar_t *pCvar ) {}
static float stub_CVarGetFloat( const char *szVarName ) { return 0.0f; }
static const char *stub_CVarGetString( const char *szVarName ) { return ""; }
static void stub_CVarSetFloat( const char *szVarName, float flValue ) {}
static void stub_CVarSetString( const char *szVarName, const char *szValue ) {}
static void stub_AlertMessage( ALERT_TYPE atype, char *szFmt, ... ) {}
static void stub_EngineFprintf( void *pfile, char *szFmt, ... ) {}

static void *stub_PvAllocEntPrivateData( edict_t *pEdict, int32 cb ) {
	if ( !pEdict ) return nullptr;
	pEdict->pvPrivateData = std::calloc( 1, cb );
	return pEdict->pvPrivateData;
}
static void *stub_PvEntPrivateData( edict_t *pEdict ) {
	return pEdict ? pEdict->pvPrivateData : nullptr;
}
static void stub_FreeEntPrivateData( edict_t *pEdict ) {
	if ( pEdict && pEdict->pvPrivateData ) {
		std::free( pEdict->pvPrivateData );
		pEdict->pvPrivateData = nullptr;
	}
}

static const char *stub_SzFromIndex( int iString ) {
	if ( iString >= 0 && iString < static_cast<int>( s_stringPool.size() ) )
		return s_stringPool[iString].c_str();
	return "";
}

static int stub_AllocString( const char *szValue ) {
	if ( !szValue ) return 0;
	auto it = s_stringMap.find( szValue );
	if ( it != s_stringMap.end() )
		return it->second;
	int idx = static_cast<int>( s_stringPool.size() );
	s_stringPool.push_back( szValue );
	s_stringMap[szValue] = idx;
	return idx;
}

static struct entvars_s *stub_GetVarsOfEnt( edict_t *pEdict ) {
	return pEdict ? &pEdict->v : nullptr;
}
static edict_t *stub_PEntityOfEntIndex( int iEntIndex ) { return nullptr; }
static int stub_EntIndexOfPEntity( const edict_t *pEdict ) { return 0; }
static edict_t *stub_FindEntityByVars( struct entvars_s *pvars ) { return nullptr; }
static void *stub_GetModelPtr( edict_t *pEdict ) { return nullptr; }
static int stub_RegUserMsg( const char *pszName, int iSize ) { return 1; }
static void stub_AnimationAutomove( const edict_t *pEdict, float flTime ) {}
static void stub_GetBonePosition( const edict_t *pEdict, int iBone, float *rgflOrigin, float *rgflAngles ) {}
static uint32 stub_FunctionFromName( const char *pName ) { return 0; }
static const char *stub_NameForFunction( uint32 function ) { return ""; }
static void stub_ClientPrintf( edict_t *pEdict, PRINT_TYPE ptype, const char *szMsg ) {}
static void stub_ServerPrint( const char *szMsg ) {}
static const char *stub_Cmd_Args( void ) { return ""; }
static const char *stub_Cmd_Argv( int argc ) { return ""; }
static int stub_Cmd_Argc( void ) { return 0; }
static void stub_GetAttachment( const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles ) {}
static void stub_CRC32_Init( CRC32_t *pulCRC ) { if ( pulCRC ) *pulCRC = 0; }
static void stub_CRC32_ProcessBuffer( CRC32_t *pulCRC, void *p, int len ) {}
static void stub_CRC32_ProcessByte( CRC32_t *pulCRC, unsigned char ch ) {}
static CRC32_t stub_CRC32_Final( CRC32_t pulCRC ) { return 0; }
static int32 stub_RandomLong( int32 lLow, int32 lHigh ) {
	if ( lHigh <= lLow ) return lLow;
	return lLow + ( std::rand() % ( lHigh - lLow + 1 ) );
}
static float stub_RandomFloat( float flLow, float flHigh ) {
	if ( flHigh <= flLow ) return flLow;
	float r = static_cast<float>( std::rand() ) / static_cast<float>( RAND_MAX );
	return flLow + r * ( flHigh - flLow );
}
static void stub_SetView( const edict_t *pClient, const edict_t *pViewent ) {}
static float stub_Time( void ) { return gpGlobals ? gpGlobals->time : 0.0f; }
static void stub_CrosshairAngle( const edict_t *pClient, float pitch, float yaw ) {}
static byte *stub_LoadFileForMe( char *filename, int *pLength ) { return nullptr; }
static void stub_FreeFile( void *buffer ) {}
static void stub_EndSection( const char *pszSectionName ) {}
static int stub_CompareFileTime( char *filename1, char *filename2, int *iCompare ) { return 0; }
static void stub_GetGameDir( char *szGetGameDir ) { if ( szGetGameDir ) std::strcpy( szGetGameDir, "valve" ); }
static void stub_Cvar_RegisterVariable( cvar_t *variable ) {}
static void stub_FadeClientVolume( const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds ) {}
static void stub_SetClientMaxspeed( const edict_t *pEdict, float fNewMaxspeed ) {}
static edict_t *stub_CreateFakeClient( const char *netname ) { return nullptr; }
static void stub_RunPlayerMove( edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec ) {}
static int stub_NumberOfEntities( void ) { return 0; }
static char *stub_GetInfoKeyBuffer( edict_t *e ) { static char buf[4] = {0}; return buf; }
static char *stub_InfoKeyValue( char *infobuffer, char *key ) { static char buf[4] = {0}; return buf; }
static void stub_SetKeyValue( char *infobuffer, char *key, char *value ) {}
static void stub_SetClientKeyValue( int clientIndex, char *infobuffer, char *key, char *value ) {}
static int stub_IsMapValid( char *filename ) { return 1; }
static int stub_CreateInstancedBaseline( int classname, struct entity_state_s *baseline ) { return 0; }
static void stub_Cvar_DirectSet( struct cvar_s *var, char *value ) {}
static void stub_ForceUnmodified( FORCE_TYPE type, float *mins, float *maxs, const char *filename ) {}
static void stub_GetPlayerStats( const edict_t *pClient, int *ping, int *packet_loss ) {
	if ( ping ) *ping = 0;
	if ( packet_loss ) *packet_loss = 0;
}
static void stub_AddServerCommand( char *cmd_name, void ( *function )( void ) ) {}
static qboolean stub_Voice_GetClientListening( int iReceiver, int iSender ) { return 1; }
static qboolean stub_Voice_SetClientListening( int iReceiver, int iSender, qboolean bListen ) { return 1; }
static const char *stub_GetPlayerAuthId( edict_t *e ) { return "STEAM_0:0:0"; }

void SetMockTraceLineResult( const TraceResult &tr )
{
	g_mockTraceResult = tr;
}

void ResetMockEngine()
{
	g_mockMessageBuffer.clear();
	g_mockMessageDest = 0;
	g_mockMessageType = 0;
	std::memset( g_mockMessageOrigin, 0, sizeof( g_mockMessageOrigin ) );
	g_mockMessageEdict = nullptr;

	std::memset( &g_mockTraceResult, 0, sizeof( g_mockTraceResult ) );
	g_mockTraceResult.flFraction = 1.0f;

	std::memset( &s_mockGlobals, 0, sizeof( s_mockGlobals ) );
	s_mockGlobals.time = 1.0f;
	s_mockGlobals.frametime = 0.01f;
	s_mockGlobals.maxClients = 1;
	gpGlobals = &s_mockGlobals;
}

void InitMockEngine()
{
	std::memset( &g_engfuncs, 0, sizeof( g_engfuncs ) );

	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnModelIndex = stub_ModelIndex;
	g_engfuncs.pfnModelFrames = stub_ModelFrames;
	g_engfuncs.pfnSetSize = stub_SetSize;
	g_engfuncs.pfnChangeLevel = stub_ChangeLevel;
	g_engfuncs.pfnGetSpawnParms = stub_GetSpawnParms;
	g_engfuncs.pfnSaveSpawnParms = stub_SaveSpawnParms;
	g_engfuncs.pfnVecToYaw = stub_VecToYaw;
	g_engfuncs.pfnVecToAngles = stub_VecToAngles;
	g_engfuncs.pfnMoveToOrigin = stub_MoveToOrigin;
	g_engfuncs.pfnChangeYaw = stub_ChangeYaw;
	g_engfuncs.pfnChangePitch = stub_ChangePitch;
	g_engfuncs.pfnFindEntityByString = stub_FindEntityByString;
	g_engfuncs.pfnGetEntityIllum = stub_GetEntityIllum;
	g_engfuncs.pfnFindEntityInSphere = stub_FindEntityInSphere;
	g_engfuncs.pfnFindClientInPVS = stub_FindClientInPVS;
	g_engfuncs.pfnEntitiesInPVS = stub_EntitiesInPVS;
	g_engfuncs.pfnMakeVectors = stub_MakeVectors;
	g_engfuncs.pfnAngleVectors = stub_AngleVectors;
	g_engfuncs.pfnCreateEntity = stub_CreateEntity;
	g_engfuncs.pfnRemoveEntity = stub_RemoveEntity;
	g_engfuncs.pfnCreateNamedEntity = stub_CreateNamedEntity;
	g_engfuncs.pfnMakeStatic = stub_MakeStatic;
	g_engfuncs.pfnEntIsOnFloor = stub_EntIsOnFloor;
	g_engfuncs.pfnDropToFloor = stub_DropToFloor;
	g_engfuncs.pfnWalkMove = stub_WalkMove;
	g_engfuncs.pfnSetOrigin = stub_SetOrigin;
	g_engfuncs.pfnEmitSound = stub_EmitSound;
	g_engfuncs.pfnEmitAmbientSound = stub_EmitAmbientSound;
	g_engfuncs.pfnTraceLine = stub_TraceLine;
	g_engfuncs.pfnTraceToss = stub_TraceToss;
	g_engfuncs.pfnTraceMonsterHull = stub_TraceMonsterHull;
	g_engfuncs.pfnTraceHull = stub_TraceHull;
	g_engfuncs.pfnTraceModel = stub_TraceModel;
	g_engfuncs.pfnTraceTexture = stub_TraceTexture;
	g_engfuncs.pfnTraceSphere = stub_TraceSphere;
	g_engfuncs.pfnGetAimVector = stub_GetAimVector;
	g_engfuncs.pfnServerCommand = stub_ServerCommand;
	g_engfuncs.pfnServerExecute = stub_ServerExecute;
	g_engfuncs.pfnClientCommand = stub_ClientCommand;
	g_engfuncs.pfnParticleEffect = stub_ParticleEffect;
	g_engfuncs.pfnLightStyle = stub_LightStyle;
	g_engfuncs.pfnDecalIndex = stub_DecalIndex;
	g_engfuncs.pfnPointContents = stub_PointContents;
	g_engfuncs.pfnMessageBegin = stub_MessageBegin;
	g_engfuncs.pfnMessageEnd = stub_MessageEnd;
	g_engfuncs.pfnWriteByte = stub_WriteByte;
	g_engfuncs.pfnWriteChar = stub_WriteChar;
	g_engfuncs.pfnWriteShort = stub_WriteShort;
	g_engfuncs.pfnWriteLong = stub_WriteLong;
	g_engfuncs.pfnWriteAngle = stub_WriteAngle;
	g_engfuncs.pfnWriteCoord = stub_WriteCoord;
	g_engfuncs.pfnWriteString = stub_WriteString;
	g_engfuncs.pfnWriteEntity = stub_WriteEntity;
	g_engfuncs.pfnCVarRegister = stub_CVarRegister;
	g_engfuncs.pfnCVarGetFloat = stub_CVarGetFloat;
	g_engfuncs.pfnCVarGetString = stub_CVarGetString;
	g_engfuncs.pfnCVarSetFloat = stub_CVarSetFloat;
	g_engfuncs.pfnCVarSetString = stub_CVarSetString;
	g_engfuncs.pfnAlertMessage = stub_AlertMessage;
	g_engfuncs.pfnEngineFprintf = stub_EngineFprintf;
	g_engfuncs.pfnPvAllocEntPrivateData = stub_PvAllocEntPrivateData;
	g_engfuncs.pfnPvEntPrivateData = stub_PvEntPrivateData;
	g_engfuncs.pfnFreeEntPrivateData = stub_FreeEntPrivateData;
	g_engfuncs.pfnSzFromIndex = stub_SzFromIndex;
	g_engfuncs.pfnAllocString = stub_AllocString;
	g_engfuncs.pfnGetVarsOfEnt = stub_GetVarsOfEnt;
	g_engfuncs.pfnPEntityOfEntIndex = stub_PEntityOfEntIndex;
	g_engfuncs.pfnPEntityOfEntOffset = stub_PEntityOfEntIndex;
	g_engfuncs.pfnEntOffsetOfPEntity = stub_EntIndexOfPEntity;
	g_engfuncs.pfnIndexOfEdict = stub_EntIndexOfPEntity;
	g_engfuncs.pfnFindEntityByVars = stub_FindEntityByVars;
	g_engfuncs.pfnGetModelPtr = stub_GetModelPtr;
	g_engfuncs.pfnRegUserMsg = stub_RegUserMsg;
	g_engfuncs.pfnAnimationAutomove = stub_AnimationAutomove;
	g_engfuncs.pfnGetBonePosition = stub_GetBonePosition;
	g_engfuncs.pfnFunctionFromName = stub_FunctionFromName;
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnClientPrintf = stub_ClientPrintf;
	g_engfuncs.pfnServerPrint = stub_ServerPrint;
	g_engfuncs.pfnCmd_Args = stub_Cmd_Args;
	g_engfuncs.pfnCmd_Argv = stub_Cmd_Argv;
	g_engfuncs.pfnCmd_Argc = stub_Cmd_Argc;
	g_engfuncs.pfnGetAttachment = stub_GetAttachment;
	g_engfuncs.pfnCRC32_Init = stub_CRC32_Init;
	g_engfuncs.pfnCRC32_ProcessBuffer = stub_CRC32_ProcessBuffer;
	g_engfuncs.pfnCRC32_ProcessByte = stub_CRC32_ProcessByte;
	g_engfuncs.pfnCRC32_Final = stub_CRC32_Final;
	g_engfuncs.pfnRandomLong = stub_RandomLong;
	g_engfuncs.pfnRandomFloat = stub_RandomFloat;
	g_engfuncs.pfnSetView = stub_SetView;
	g_engfuncs.pfnTime = stub_Time;
	g_engfuncs.pfnCrosshairAngle = stub_CrosshairAngle;
	g_engfuncs.pfnLoadFileForMe = stub_LoadFileForMe;
	g_engfuncs.pfnFreeFile = stub_FreeFile;
	g_engfuncs.pfnEndSection = stub_EndSection;
	g_engfuncs.pfnCompareFileTime = stub_CompareFileTime;
	g_engfuncs.pfnGetGameDir = stub_GetGameDir;
	g_engfuncs.pfnCvar_RegisterVariable = stub_Cvar_RegisterVariable;
	g_engfuncs.pfnFadeClientVolume = stub_FadeClientVolume;
	g_engfuncs.pfnSetClientMaxspeed = stub_SetClientMaxspeed;
	g_engfuncs.pfnCreateFakeClient = stub_CreateFakeClient;
	g_engfuncs.pfnRunPlayerMove = stub_RunPlayerMove;
	g_engfuncs.pfnNumberOfEntities = stub_NumberOfEntities;
	g_engfuncs.pfnGetInfoKeyBuffer = stub_GetInfoKeyBuffer;
	g_engfuncs.pfnInfoKeyValue = stub_InfoKeyValue;
	g_engfuncs.pfnSetKeyValue = stub_SetKeyValue;
	g_engfuncs.pfnSetClientKeyValue = stub_SetClientKeyValue;
	g_engfuncs.pfnIsMapValid = stub_IsMapValid;
	g_engfuncs.pfnCreateInstancedBaseline = stub_CreateInstancedBaseline;
	g_engfuncs.pfnCvar_DirectSet = stub_Cvar_DirectSet;
	g_engfuncs.pfnForceUnmodified = stub_ForceUnmodified;
	g_engfuncs.pfnGetPlayerStats = stub_GetPlayerStats;
	g_engfuncs.pfnAddServerCommand = stub_AddServerCommand;
	g_engfuncs.pfnVoice_GetClientListening = stub_Voice_GetClientListening;
	g_engfuncs.pfnVoice_SetClientListening = stub_Voice_SetClientListening;
	g_engfuncs.pfnGetPlayerAuthId = stub_GetPlayerAuthId;

	ResetMockEngine();
}

void UTIL_PrecacheOtherWeapon( const char *szClassname )
{
	(void)szClassname;
}

void UTIL_PrecacheOther( const char *szClassname )
{
	(void)szClassname;
}

#include "util.h"
#include "cbase.h"

const Vector g_vecZero = Vector( 0, 0, 0 );

int CBaseEntity::Save( CSave &save ) { return 0; }
int CBaseEntity::Restore( CRestore &restore ) { return 0; }
void CBaseEntity::SetObjectCollisionBox( void ) {}
void CBaseEntity::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) {}
int CBaseEntity::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { return 0; }
int CBaseEntity::TakeHealth( float flHealth, int bitsDamageType ) { return 0; }
void CBaseEntity::Killed( entvars_t *pevAttacker, int iGib ) {}
void CBaseEntity::TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) {}
int CBaseEntity::DamageDecal( int bitsDamageType ) { return 0; }
int CBaseEntity::IsInWorld( void ) { return 1; }
CBaseEntity *CBaseEntity::GetNextTarget( void ) { return nullptr; }
int CBaseEntity::FVisible( CBaseEntity *pEntity ) { return 1; }
int CBaseEntity::FVisible( const Vector &vecTarget ) { return 1; }

