/***
 *
 *	Half-Life Client Initialization Smoke Test Runner
 *	Accepts a compiled client.dll or client.so path, loads it dynamically,
 *	and verifies the GoldSrc engine boot sequence headlessly.
 *
 *	Exit code 0 on clean initialization, non-zero on failure or crash.
 *
 ****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef float vec_t;
typedef float vec3_t[3];
typedef int ( *pfnUserMsgHook )( const char *pszName, int iSize, void *pbuf );

typedef struct rect_s
{
	int left, right, top, bottom;
} wrect_t;

#include "common/const.h"
#include "common/cvardef.h"
#include "common/com_model.h"
#include "common/demo_api.h"
#include "common/r_studioint.h"
#include "engine/cdll_int.h"

#if defined( _WIN32 )
#include <windows.h>
#else
#include <dlfcn.h>
#include <signal.h>
#include <setjmp.h>
#endif

// -----------------------------------------------------------------------------
// Minimal Embedded Mock Engine Stubs
// -----------------------------------------------------------------------------
static struct cvar_s *smoke_RegisterVariable( char *szName, char *szValue, int flags )
{
	static cvar_t s_cvars[128];
	static int s_numCvars = 0;

	if ( !szName )
		return NULL;

	for ( int i = 0; i < s_numCvars; i++ )
	{
		if ( s_cvars[i].name && strcmp( s_cvars[i].name, szName ) == 0 )
			return &s_cvars[i];
	}

	if ( s_numCvars >= 128 )
		return &s_cvars[0];

	cvar_t *c = &s_cvars[s_numCvars++];
	c->name   = szName;
	c->string = szValue ? szValue : (char *)"";
	c->flags  = flags;
	c->value  = szValue ? (float)atof( szValue ) : 0.0f;
	c->next   = NULL;
	return c;
}

static struct cvar_s *smoke_GetCvarPointer( const char *szName )
{
	(void)szName;
	return NULL;
}

static float smoke_GetCvarFloat( char *szName )
{
	(void)szName;
	return 0.0f;
}

static char *smoke_GetCvarString( char *szName )
{
	(void)szName;
	static char s_empty[1] = { 0 };
	return s_empty;
}

static void smoke_Cvar_SetValue( char *cvar, float value )
{
	(void)cvar;
	(void)value;
}

static int smoke_AddCommand( char *cmd_name, void ( *function )( void ) )
{
	(void)cmd_name;
	(void)function;
	return 1;
}

static int smoke_HookUserMsg( char *szMsgName, pfnUserMsgHook pfn )
{
	(void)szMsgName;
	(void)pfn;
	return 1;
}

static void smoke_HookEvent( char *name, void ( *pfnEvent )( struct event_args_s *args ) )
{
	(void)name;
	(void)pfnEvent;
}

static int smoke_ServerCmd( char *szCmdString )
{
	(void)szCmdString;
	return 1;
}

static int smoke_ClientCmd( char *szCmdString )
{
	(void)szCmdString;
	return 1;
}

static const char *smoke_GetLevelName( void )
{
	return "";
}

static const char *smoke_GetGameDirectory( void )
{
	return "valve";
}

static int smoke_GetScreenInfo( SCREENINFO *pscrinfo )
{
	if ( pscrinfo )
	{
		pscrinfo->iSize   = sizeof( SCREENINFO );
		pscrinfo->iWidth  = 640;
		pscrinfo->iHeight = 480;
		pscrinfo->iFlags  = 0;
		return 1;
	}
	return 0;
}

static struct cl_entity_s *smoke_GetEntityByIndex( int index )
{
	(void)index;
	static struct cl_entity_s s_ent;
	memset( &s_ent, 0, sizeof( s_ent ) );
	return &s_ent;
}

static struct cl_entity_s *smoke_GetLocalPlayer( void )
{
	static struct cl_entity_s s_player;
	memset( &s_player, 0, sizeof( s_player ) );
	s_player.index  = 1;
	s_player.player = 1;
	return &s_player;
}

static float smoke_GetClientTime( void )
{
	return 1.0f;
}

static void smoke_Con_Printf( char *fmt, ... )
{
	(void)fmt;
}

static void smoke_Con_DPrintf( char *fmt, ... )
{
	(void)fmt;
}

static int smoke_COM_ExpandFilename( const char *fileName, char *nameOutBuffer, int nameOutBufferSize )
{
	(void)fileName;
	(void)nameOutBuffer;
	(void)nameOutBufferSize;
	return 0;
}

static char *smoke_COM_ParseFile( char *data, char *token )
{
	if ( !data || !token )
		return NULL;
	token[0] = 0;
	return NULL;
}

static unsigned char *smoke_COM_LoadFile( char *path, int usehunk, int *pLength )
{
	(void)path;
	(void)usehunk;
	if ( pLength )
		*pLength = 0;
	return NULL;
}

static void smoke_COM_FreeFile( void *buffer )
{
	(void)buffer;
}

static int smoke_IsSpectateOnly( void )
{
	return 0;
}

static void *smoke_VGui_GetPanel( void )
{
	return NULL;
}

static void smoke_FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a )
{
	(void)x;
	(void)y;
	(void)width;
	(void)height;
	(void)r;
	(void)g;
	(void)b;
	(void)a;
}

static void smoke_DrawSetTextColor( float r, float g, float b )
{
	(void)r;
	(void)g;
	(void)b;
}

static int smoke_IsDemoPlaying( void )
{
	return 0;
}

static int smoke_IsDemoRecording( void )
{
	return 0;
}

static int smoke_CheckParm( char *parm, char **ppnext )
{
	(void)parm;
	if ( ppnext )
		*ppnext = NULL;
	return 0;
}

static double smoke_GetAbsoluteTime( void )
{
	return 1.0;
}

static int smoke_GetMaxClients( void )
{
	return 32;
}

static int smoke_GetWindowCenterX( void )
{
	return 320;
}

static int smoke_GetWindowCenterY( void )
{
	return 240;
}

static void smoke_GetViewAngles( float *va )
{
	if ( va )
		va[0] = va[1] = va[2] = 0.0f;
}

static void smoke_SetViewAngles( float *va )
{
	(void)va;
}

static void smoke_GetMousePosition( int *mx, int *my )
{
	if ( mx )
		*mx = 320;
	if ( my )
		*my = 240;
}

static struct demo_api_s s_smokeDemoApi = {
	smoke_IsDemoPlaying,
	smoke_IsDemoRecording,
	NULL
};

// Studio Stubs
static void *smoke_Mem_Calloc( int number, size_t size )
{
	return calloc( number, size );
}

static void *smoke_Cache_Check( struct cache_user_s *c )
{
	(void)c;
	return NULL;
}

static void smoke_LoadCacheFile( char *path, struct cache_user_s *cu )
{
	(void)path;
	(void)cu;
}

static struct model_s *smoke_Mod_ForName( const char *name, int crash_if_missing )
{
	(void)name;
	(void)crash_if_missing;
	return NULL;
}

static struct cl_entity_s *smoke_StudioGetCurrentEntity( void )
{
	return smoke_GetLocalPlayer();
}

static struct player_info_s *smoke_PlayerInfo( int index )
{
	(void)index;
	static struct player_info_s s_info;
	memset( &s_info, 0, sizeof( s_info ) );
	strncpy( s_info.name, "SmokePlayer", sizeof( s_info.name ) );
	return &s_info;
}

static struct cl_entity_s *smoke_GetViewEntity( void )
{
	return smoke_GetLocalPlayer();
}

static void smoke_GetTimes( int *framecount, double *current, double *old )
{
	if ( framecount )
		*framecount = 1;
	if ( current )
		*current = 1.0;
	if ( old )
		*old = 0.0;
}

static struct cvar_s *smoke_StudioGetCvar( const char *name )
{
	return smoke_GetCvarPointer( name );
}

#ifndef MAXSTUDIOBONES
#define MAXSTUDIOBONES 128
#endif

static int s_smokeStudioModelCount = 0;
static int s_smokeModelsDrawn      = 0;
static float s_smokeBoneTransform[MAXSTUDIOBONES][3][4];
static float s_smokeLightTransform[MAXSTUDIOBONES][3][4];
static float s_smokeAliasTransform[3][4];
static float s_smokeRotationMatrix[3][4];

static struct model_s *smoke_GetChromeSprite( void )
{
	return NULL;
}

static void smoke_GetModelCounters( int **s, int **a )
{
	if ( s )
		*s = &s_smokeStudioModelCount;
	if ( a )
		*a = &s_smokeModelsDrawn;
}

static float ****smoke_StudioGetBoneTransform( void )
{
	return (float ****)&s_smokeBoneTransform;
}

static float ****smoke_StudioGetLightTransform( void )
{
	return (float ****)&s_smokeLightTransform;
}

static float ***smoke_StudioGetAliasTransform( void )
{
	return (float ***)&s_smokeAliasTransform;
}

static float ***smoke_StudioGetRotationMatrix( void )
{
	return (float ***)&s_smokeRotationMatrix;
}

static void PopulateMockEngine( cl_enginefunc_t *pEng, engine_studio_api_t *pStudio )
{
	memset( pEng, 0, sizeof( *pEng ) );
	pEng->pfnRegisterVariable = smoke_RegisterVariable;
	pEng->pfnGetCvarPointer   = smoke_GetCvarPointer;
	pEng->pfnGetCvarFloat     = smoke_GetCvarFloat;
	pEng->pfnGetCvarString    = smoke_GetCvarString;
	pEng->Cvar_SetValue       = smoke_Cvar_SetValue;
	pEng->pfnAddCommand       = smoke_AddCommand;
	pEng->pfnHookUserMsg      = smoke_HookUserMsg;
	pEng->pfnHookEvent        = smoke_HookEvent;
	pEng->pfnServerCmd        = smoke_ServerCmd;
	pEng->pfnClientCmd        = smoke_ClientCmd;
	pEng->pfnGetLevelName     = smoke_GetLevelName;
	pEng->pfnGetGameDirectory = smoke_GetGameDirectory;
	pEng->pfnGetScreenInfo    = smoke_GetScreenInfo;
	pEng->GetEntityByIndex    = smoke_GetEntityByIndex;
	pEng->GetLocalPlayer      = smoke_GetLocalPlayer;
	pEng->GetClientTime       = smoke_GetClientTime;
	pEng->CheckParm           = smoke_CheckParm;
	pEng->GetAbsoluteTime     = smoke_GetAbsoluteTime;
	pEng->GetMaxClients       = smoke_GetMaxClients;
	pEng->GetWindowCenterX    = smoke_GetWindowCenterX;
	pEng->GetWindowCenterY    = smoke_GetWindowCenterY;
	pEng->GetViewAngles       = smoke_GetViewAngles;
	pEng->SetViewAngles       = smoke_SetViewAngles;
	pEng->GetMousePosition    = smoke_GetMousePosition;
	pEng->Con_Printf          = smoke_Con_Printf;
	pEng->Con_DPrintf         = smoke_Con_DPrintf;
	pEng->COM_ExpandFilename  = smoke_COM_ExpandFilename;
	pEng->COM_ParseFile       = smoke_COM_ParseFile;
	pEng->COM_LoadFile        = smoke_COM_LoadFile;
	pEng->COM_FreeFile        = smoke_COM_FreeFile;
	pEng->IsSpectateOnly      = smoke_IsSpectateOnly;
	pEng->VGui_GetPanel       = smoke_VGui_GetPanel;
	pEng->pfnFillRGBA         = smoke_FillRGBA;
	pEng->pfnDrawSetTextColor = smoke_DrawSetTextColor;
	pEng->pDemoAPI            = &s_smokeDemoApi;

	memset( pStudio, 0, sizeof( *pStudio ) );
	pStudio->Mem_Calloc              = smoke_Mem_Calloc;
	pStudio->Cache_Check             = smoke_Cache_Check;
	pStudio->LoadCacheFile           = smoke_LoadCacheFile;
	pStudio->Mod_ForName             = smoke_Mod_ForName;
	pStudio->GetCurrentEntity        = smoke_StudioGetCurrentEntity;
	pStudio->PlayerInfo              = smoke_PlayerInfo;
	pStudio->GetViewEntity           = smoke_GetViewEntity;
	pStudio->GetTimes                = smoke_GetTimes;
	pStudio->GetCvar                 = smoke_StudioGetCvar;
	pStudio->GetChromeSprite         = smoke_GetChromeSprite;
	pStudio->GetModelCounters        = smoke_GetModelCounters;
	pStudio->StudioGetBoneTransform  = smoke_StudioGetBoneTransform;
	pStudio->StudioGetLightTransform = smoke_StudioGetLightTransform;
	pStudio->StudioGetAliasTransform = smoke_StudioGetAliasTransform;
	pStudio->StudioGetRotationMatrix = smoke_StudioGetRotationMatrix;
}

// -----------------------------------------------------------------------------
// Crash and Signal Handling
// -----------------------------------------------------------------------------
#if !defined( _WIN32 )
static sigjmp_buf s_jmpBuf;
static void sig_handler( int sig )
{
	fprintf( stderr, "\n[FATAL CRASH] Caught signal %d (%s) during client lifecycle execution!\n",
	         sig, strsignal( sig ) );
	siglongjmp( s_jmpBuf, 1 );
}
#endif

// -----------------------------------------------------------------------------
// Main Smoke Runner
// -----------------------------------------------------------------------------
int main( int argc, char **argv )
{
	if ( argc < 2 )
	{
		fprintf( stderr, "Usage: %s <path/to/client.dll or client.so>\n", argv[0] );
		return 1;
	}

	const char *libPath = argv[1];
	printf( "=== GoldSrc Client DLL Smoke Test ===\n" );
	printf( "Target binary: %s\n", libPath );

#if defined( _WIN32 )
	char fullLibPath[MAX_PATH];
	if ( GetFullPathNameA( libPath, MAX_PATH, fullLibPath, NULL ) > 0 )
	{
		libPath = fullLibPath;
		char fullDir[MAX_PATH];
		char drive[_MAX_DRIVE], dir[_MAX_DIR];
		_splitpath( fullLibPath, drive, dir, NULL, NULL );
		snprintf( fullDir, sizeof( fullDir ), "%s%s", drive, dir );
		if ( fullDir[0] )
		{
			SetDllDirectoryA( fullDir );
		}
	}

	const char *vguiPaths[] = { "lib/public/vgui.dll", "../lib/public/vgui.dll", "../../lib/public/vgui.dll", "vgui.dll" };
	for ( const char *vp : vguiPaths )
	{
		char fullVgui[MAX_PATH];
		if ( GetFullPathNameA( vp, MAX_PATH, fullVgui, NULL ) > 0 )
		{
			HMODULE hVgui = LoadLibraryA( fullVgui );
			if ( hVgui )
				break;
		}
	}
	const char *sdlPaths[] = { "lib/public/SDL2.dll", "../lib/public/SDL2.dll", "../../lib/public/SDL2.dll", "SDL2.dll" };
	for ( const char *sp : sdlPaths )
	{
		char fullSdl[MAX_PATH];
		if ( GetFullPathNameA( sp, MAX_PATH, fullSdl, NULL ) > 0 )
		{
			HMODULE hSdl = LoadLibraryA( fullSdl );
			if ( hSdl )
				break;
		}
	}
	HMODULE hLib = LoadLibraryExA( libPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
	if ( !hLib )
	{
		DWORD err = GetLastError();
		fprintf( stderr, "ERROR: Failed to load target library '%s' (Win32 error: %lu)\n", libPath, err );
		return 1;
	}
#else
	const char *vguiLinuxPaths[] = { "linux/vgui.so", "../linux/vgui.so", "../../linux/vgui.so", "vgui.so" };
	for ( const char *vp : vguiLinuxPaths )
	{
		void *hVgui = dlopen( vp, RTLD_NOW | RTLD_GLOBAL );
		if ( hVgui )
			break;
	}
	const char *sdlLinuxPaths[] = { "linux/libSDL2.so", "../linux/libSDL2.so", "../../linux/libSDL2.so", "libSDL2.so", "libSDL2-2.0.so.0" };
	for ( const char *sp : sdlLinuxPaths )
	{
		void *hSdl = dlopen( sp, RTLD_NOW | RTLD_GLOBAL );
		if ( hSdl )
			break;
	}
	void *hLib = dlopen( libPath, RTLD_NOW | RTLD_GLOBAL );
	if ( !hLib )
	{
		fprintf( stderr, "ERROR: Failed to load target library '%s': %s\n", libPath, dlerror() );
		return 1;
	}
#endif

	typedef int ( *pfnInitialize_t )( cl_enginefunc_t *pEnginefuncs, int iVersion );
	typedef void ( *pfnHUD_Init_t )( void );
	typedef int ( *pfnHUD_GetStudioModelInterface_t )( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );

#if defined( _WIN32 )
	pfnInitialize_t pfnInit                  = (pfnInitialize_t)GetProcAddress( hLib, "Initialize" );
	pfnHUD_Init_t pfnHUDInit                 = (pfnHUD_Init_t)GetProcAddress( hLib, "HUD_Init" );
	pfnHUD_GetStudioModelInterface_t pfnStudio = (pfnHUD_GetStudioModelInterface_t)GetProcAddress( hLib, "HUD_GetStudioModelInterface" );
#else
	pfnInitialize_t pfnInit                  = (pfnInitialize_t)dlsym( hLib, "Initialize" );
	pfnHUD_Init_t pfnHUDInit                 = (pfnHUD_Init_t)dlsym( hLib, "HUD_Init" );
	pfnHUD_GetStudioModelInterface_t pfnStudio = (pfnHUD_GetStudioModelInterface_t)dlsym( hLib, "HUD_GetStudioModelInterface" );
#endif

	// Fallback to F() if individual exports not found
	if ( !pfnInit || !pfnHUDInit )
	{
#if defined( _WIN32 )
		typedef void ( *pfnF_t )( void *pv );
		pfnF_t pfnF = (pfnF_t)GetProcAddress( hLib, "F" );
#else
		typedef void ( *pfnF_t )( void *pv );
		pfnF_t pfnF = (pfnF_t)dlsym( hLib, "F" );
#endif
		if ( pfnF )
		{
			cldll_func_t funcs;
			memset( &funcs, 0, sizeof( funcs ) );
			pfnF( &funcs );
			if ( !pfnInit )
				pfnInit = funcs.pInitFunc;
			if ( !pfnHUDInit )
				pfnHUDInit = funcs.pHudInitFunc;
			if ( !pfnStudio )
				pfnStudio = funcs.pStudioInterface;
		}
	}

	if ( !pfnInit )
	{
		fprintf( stderr, "ERROR: Could not resolve 'Initialize' export in '%s'\n", libPath );
#if defined( _WIN32 )
		FreeLibrary( hLib );
#else
		dlclose( hLib );
#endif
		return 1;
	}

	if ( !pfnHUDInit )
	{
		fprintf( stderr, "ERROR: Could not resolve 'HUD_Init' export in '%s'\n", libPath );
#if defined( _WIN32 )
		FreeLibrary( hLib );
#else
		dlclose( hLib );
#endif
		return 1;
	}

	cl_enginefunc_t mockEngfuncs;
	engine_studio_api_t mockStudioApi;
	PopulateMockEngine( &mockEngfuncs, &mockStudioApi );

	// Execute engine boot lifecycle under structured exception / signal protection
	bool bSuccess = false;

#if defined( _WIN32 )
	__try
	{
		printf( "[1/3] Calling Initialize(&mockEngfuncs, %d)...\n", CLDLL_INTERFACE_VERSION );
		int initResult = pfnInit( &mockEngfuncs, CLDLL_INTERFACE_VERSION );
		if ( initResult != 1 )
		{
			fprintf( stderr, "ERROR: Initialize returned %d (expected 1)\n", initResult );
			FreeLibrary( hLib );
			return 1;
		}
		printf( "      Initialize succeeded (return 1).\n" );

		printf( "[2/3] Calling HUD_Init() [Pre-video init, gViewPort == NULL]...\n" );
		pfnHUDInit();
		printf( "      HUD_Init completed without crash!\n" );

		if ( pfnStudio )
		{
			printf( "[3/3] Calling HUD_GetStudioModelInterface(%d, ...)...\n", STUDIO_INTERFACE_VERSION );
			r_studio_interface_t *pStudio = NULL;
			int studioResult               = pfnStudio( STUDIO_INTERFACE_VERSION, &pStudio, &mockStudioApi );
			if ( studioResult != 1 || pStudio == NULL )
			{
				fprintf( stderr, "ERROR: HUD_GetStudioModelInterface failed (result: %d, ptr: %p)\n",
				         studioResult, (void *)pStudio );
				FreeLibrary( hLib );
				return 1;
			}
			printf( "      HUD_GetStudioModelInterface succeeded (version %d callbacks bound).\n",
			        pStudio->version );
		}
		else
		{
			printf( "[3/3] HUD_GetStudioModelInterface not exported, skipping studio check.\n" );
		}

		bSuccess = true;
	}
	__except ( []( unsigned int code, struct _EXCEPTION_POINTERS *ep, HMODULE hMod ) -> int {
		DWORD_PTR crashAddr = (DWORD_PTR)ep->ExceptionRecord->ExceptionAddress;
		DWORD_PTR modBase   = (DWORD_PTR)hMod;
		char modName[MAX_PATH] = "unknown";
		HMODULE hCrashMod = NULL;
		if ( GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)crashAddr, &hCrashMod ) )
		{
			GetModuleFileNameA( hCrashMod, modName, sizeof( modName ) );
		}
		fprintf( stderr, "\n[FATAL CRASH] Crashed with exception code 0x%08lX!\n", (unsigned long)code );
		fprintf( stderr, "              Crash Address: 0x%p in %s (offset 0x%lx)\n",
		         (void *)crashAddr, modName,
		         (unsigned long)( hCrashMod ? crashAddr - (DWORD_PTR)hCrashMod : 0 ) );
		return EXCEPTION_EXECUTE_HANDLER;
	}( GetExceptionCode(), GetExceptionInformation(), hLib ) )
	{
		bSuccess = false;
	}

	FreeLibrary( hLib );

#else // Linux / POSIX
	struct sigaction sa;
	memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = sig_handler;
	sigaction( SIGSEGV, &sa, NULL );
	sigaction( SIGBUS, &sa, NULL );
	sigaction( SIGFPE, &sa, NULL );
	sigaction( SIGILL, &sa, NULL );

	if ( sigsetjmp( s_jmpBuf, 1 ) == 0 )
	{
		printf( "[1/3] Calling Initialize(&mockEngfuncs, %d)...\n", CLDLL_INTERFACE_VERSION );
		int initResult = pfnInit( &mockEngfuncs, CLDLL_INTERFACE_VERSION );
		if ( initResult != 1 )
		{
			fprintf( stderr, "ERROR: Initialize returned %d (expected 1)\n", initResult );
			dlclose( hLib );
			return 1;
		}
		printf( "      Initialize succeeded (return 1).\n" );

		printf( "[2/3] Calling HUD_Init() [Pre-video init, gViewPort == NULL]...\n" );
		pfnHUDInit();
		printf( "      HUD_Init completed without crash!\n" );

		if ( pfnStudio )
		{
			printf( "[3/3] Calling HUD_GetStudioModelInterface(%d, ...)...\n", STUDIO_INTERFACE_VERSION );
			r_studio_interface_t *pStudio = NULL;
			int studioResult               = pfnStudio( STUDIO_INTERFACE_VERSION, &pStudio, &mockStudioApi );
			if ( studioResult != 1 || pStudio == NULL )
			{
				fprintf( stderr, "ERROR: HUD_GetStudioModelInterface failed (result: %d, ptr: %p)\n",
				         studioResult, (void *)pStudio );
				dlclose( hLib );
				return 1;
			}
			printf( "      HUD_GetStudioModelInterface succeeded (version %d callbacks bound).\n",
			        pStudio->version );
		}
		else
		{
			printf( "[3/3] HUD_GetStudioModelInterface not exported, skipping studio check.\n" );
		}

		bSuccess = true;
	}
	else
	{
		bSuccess = false;
	}

	dlclose( hLib );
#endif

	if ( bSuccess )
	{
		printf( "\n[SUCCESS] Client library '%s' initialized and verified cleanly!\n", libPath );
		return 0;
	}
	else
	{
		fprintf( stderr, "\n[FAILURE] Smoke test failed for '%s'!\n", libPath );
		return 1;
	}
}
