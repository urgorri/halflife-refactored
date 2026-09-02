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

#ifndef INPUT_HARDWARE_H
#define INPUT_HARDWARE_H

#include "port.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_gamecontroller.h>

void Hardware_InitMouseThread( void );
void Hardware_ShutdownMouseThread( void );
void Hardware_GetRawMouseDelta( int *mx, int *my, bool bRawInput, bool bMouseThread );
void Hardware_PumpRelativeMouse( void );
void Hardware_ResetMousePos( int x, int y );
void Hardware_InitJoystick( void );
int  Hardware_GetJoystickAxis( int axis );
bool Hardware_IsJoystickAvailable( void );
SDL_GameController *Hardware_GetJoystickHandle( void );

#endif // INPUT_HARDWARE_H
