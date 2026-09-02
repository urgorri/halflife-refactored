/***
 *
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "port.h"
#include "hud/hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "in_defs.h"
#include "input_hardware.h"

extern cl_enginefunc_t gEngfuncs;

#ifdef _WIN32
static DWORD s_hMouseThreadId       = 0;
static HANDLE s_hMouseThread        = 0;
static HANDLE s_hMouseQuitEvent     = 0;
static HANDLE s_hMouseDoneQuitEvent = 0;
static long s_mouseDeltaX           = 0;
static long s_mouseDeltaY           = 0;
static POINT old_mouse_pos;
#endif

static SDL_GameController *s_pJoystick = NULL;
static bool s_bJoystickAvail           = false;

extern cvar_t *m_mousethread_sleep;

#ifdef _WIN32
static long ThreadInterlockedExchange( long *pDest, long value )
{
	return InterlockedExchange( pDest, value );
}

static DWORD WINAPI MousePos_ThreadFunction( LPVOID p )
{
	s_hMouseDoneQuitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	while ( 1 )
	{
		int sleepTime = ( m_mousethread_sleep ) ? (int)m_mousethread_sleep->value : 10;
		if ( WaitForSingleObject( s_hMouseQuitEvent, sleepTime ) == WAIT_OBJECT_0 )
		{
			return 0;
		}

		POINT mouse_pos;
		GetCursorPos( &mouse_pos );

		volatile int mx = mouse_pos.x - old_mouse_pos.x + s_mouseDeltaX;
		volatile int my = mouse_pos.y - old_mouse_pos.y + s_mouseDeltaY;

		ThreadInterlockedExchange( &old_mouse_pos.x, mouse_pos.x );
		ThreadInterlockedExchange( &old_mouse_pos.y, mouse_pos.y );

		ThreadInterlockedExchange( &s_mouseDeltaX, mx );
		ThreadInterlockedExchange( &s_mouseDeltaY, my );
	}

	SetEvent( s_hMouseDoneQuitEvent );

	return 0;
}
#endif

void Hardware_InitMouseThread( void )
{
#ifdef _WIN32
	s_mouseDeltaX = s_mouseDeltaY = 0;

	s_hMouseQuitEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( s_hMouseQuitEvent )
	{
		s_hMouseThread = CreateThread( NULL, 0, MousePos_ThreadFunction, NULL, 0, &s_hMouseThreadId );
	}
#endif
}

void Hardware_ShutdownMouseThread( void )
{
#ifdef _WIN32
	if ( s_hMouseQuitEvent )
	{
		SetEvent( s_hMouseQuitEvent );
		WaitForSingleObject( s_hMouseDoneQuitEvent, 100 );
	}

	if ( s_hMouseThread )
	{
		TerminateThread( s_hMouseThread, 0 );
		CloseHandle( s_hMouseThread );
		s_hMouseThread = (HANDLE)0;
	}

	if ( s_hMouseQuitEvent )
	{
		CloseHandle( s_hMouseQuitEvent );
		s_hMouseQuitEvent = (HANDLE)0;
	}

	if ( s_hMouseDoneQuitEvent )
	{
		CloseHandle( s_hMouseDoneQuitEvent );
		s_hMouseDoneQuitEvent = (HANDLE)0;
	}
#endif
}

void Hardware_GetRawMouseDelta( int *mx, int *my, bool bRawInput, bool bMouseThread )
{
	if ( bRawInput )
	{
		int deltaX = 0, deltaY = 0;
		SDL_GetRelativeMouseState( &deltaX, &deltaY );
		*mx = deltaX;
		*my = deltaY;
	}
	else
	{
#ifdef _WIN32
		if ( bMouseThread )
		{
			*mx = s_mouseDeltaX;
			*my = s_mouseDeltaY;
			ThreadInterlockedExchange( &s_mouseDeltaX, 0 );
			ThreadInterlockedExchange( &s_mouseDeltaY, 0 );
		}
		else
		{
			POINT current_pos;
			GetCursorPos( &current_pos );
			*mx = current_pos.x - gEngfuncs.GetWindowCenterX();
			*my = current_pos.y - gEngfuncs.GetWindowCenterY();
		}
#else
		*mx = 0;
		*my = 0;
#endif
	}
}

void Hardware_PumpRelativeMouse( void )
{
	SDL_PumpEvents();
	int deltaX, deltaY;
	SDL_GetRelativeMouseState( &deltaX, &deltaY );
}

void Hardware_ResetMousePos( int x, int y )
{
#ifdef _WIN32
	ThreadInterlockedExchange( &old_mouse_pos.x, x );
	ThreadInterlockedExchange( &old_mouse_pos.y, y );
#endif
}

void Hardware_InitJoystick( void )
{
	if ( gEngfuncs.CheckParm( "-nojoy", NULL ) )
		return;

	static float flLastCheck = 0.0f;
	if ( flLastCheck > 0.0f && ( gEngfuncs.GetAbsoluteTime() - flLastCheck ) < 1.0f )
		return;

	flLastCheck = gEngfuncs.GetAbsoluteTime();

	int nJoysticks = SDL_NumJoysticks();
	if ( nJoysticks > 0 )
	{
		if ( s_pJoystick == NULL )
		{
			for ( int i = 0; i < nJoysticks; i++ )
			{
				if ( SDL_IsGameController( i ) )
				{
					s_pJoystick = SDL_GameControllerOpen( i );
					if ( s_pJoystick )
					{
						gEngfuncs.Con_Printf( "joystick found %s\n\n", SDL_GameControllerName( s_pJoystick ) );
						s_bJoystickAvail = true;
						break;
					}
				}
			}
		}
	}
	else
	{
		if ( s_pJoystick )
			SDL_GameControllerClose( s_pJoystick );
		s_pJoystick = NULL;
		if ( s_bJoystickAvail )
		{
			s_bJoystickAvail = false;
			gEngfuncs.Con_DPrintf( "joystick not found -- driver not present\n\n" );
		}
	}
}

int Hardware_GetJoystickAxis( int axis )
{
	if ( !s_pJoystick )
		return 0;

	switch ( axis )
	{
	default:
	case 0: // JOY_AXIS_X
		return SDL_GameControllerGetAxis( s_pJoystick, SDL_CONTROLLER_AXIS_LEFTX );
	case 1: // JOY_AXIS_Y
		return SDL_GameControllerGetAxis( s_pJoystick, SDL_CONTROLLER_AXIS_LEFTY );
	case 2: // JOY_AXIS_Z
		return SDL_GameControllerGetAxis( s_pJoystick, SDL_CONTROLLER_AXIS_RIGHTX );
	case 3: // JOY_AXIS_R
		return SDL_GameControllerGetAxis( s_pJoystick, SDL_CONTROLLER_AXIS_RIGHTY );
	}
}

bool Hardware_IsJoystickAvailable( void )
{
	return s_bJoystickAvail && ( s_pJoystick != NULL );
}

SDL_GameController *Hardware_GetJoystickHandle( void )
{
	return s_pJoystick;
}
