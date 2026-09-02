//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Shared environmental audio and sound local declarations.
//
// $NoKeywords: $
//=============================================================================

#if !defined( SOUND_LOCAL_H )
#define SOUND_LOCAL_H
#pragma once

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "weapons/weapon_base.h"
#include "core/player.h"
#include "ai/talkmonster.h"
#include "gameplay/gamerules.h"

#if !defined( _WIN32 )
#include <ctype.h>
#endif

static inline char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize )
{
	if ( !pMemFile || !pBuffer )
		return NULL;

	if ( filePos >= fileSize )
		return NULL;

	int i    = filePos;
	int last = fileSize;

	if ( last - filePos > ( bufferSize - 1 ) )
		last = filePos + ( bufferSize - 1 );

	int stop = 0;

	while ( i < last && !stop )
	{
		if ( pMemFile[i] == '\n' )
			stop = 1;
		i++;
	}

	if ( i != filePos )
	{
		int size = i - filePos;
		memcpy( pBuffer, pMemFile + filePos, sizeof( byte ) * size );

		if ( size < bufferSize )
			pBuffer[size] = 0;

		filePos = i;
		return pBuffer;
	}

	return NULL;
}

#endif // SOUND_LOCAL_H
