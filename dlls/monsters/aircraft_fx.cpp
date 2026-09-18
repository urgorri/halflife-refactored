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

#include "monsters/aircraft_fx.h"

void Aircraft_FallingEffects( const Vector &vecOrigin, int modelIndexFireball, int modelIndexSmoke, int msgType )
{
	// Random explosion dynamic light
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecOrigin );
	WRITE_BYTE( TE_EXPLOSION );
	WRITE_COORD( vecOrigin.x + RANDOM_FLOAT( -150, 150 ) );
	WRITE_COORD( vecOrigin.y + RANDOM_FLOAT( -150, 150 ) );
	WRITE_COORD( vecOrigin.z + RANDOM_FLOAT( -150, -50 ) );
	WRITE_SHORT( modelIndexFireball );
	WRITE_BYTE( RANDOM_LONG( 0, 29 ) + 30 ); // scale * 10
	WRITE_BYTE( 12 );                        // framerate
	WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	// Smoke puff
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecOrigin );
	WRITE_BYTE( TE_SMOKE );
	WRITE_COORD( vecOrigin.x + RANDOM_FLOAT( -150, 150 ) );
	WRITE_COORD( vecOrigin.y + RANDOM_FLOAT( -150, 150 ) );
	WRITE_COORD( vecOrigin.z + RANDOM_FLOAT( -150, -50 ) );
	WRITE_SHORT( modelIndexSmoke );
	WRITE_BYTE( 100 ); // scale * 10
	WRITE_BYTE( 10 );  // framerate
	MESSAGE_END();
}

void Aircraft_BreakModel( const Vector &vecPos, const Vector &vecSize, const Vector &vecVelocity,
                          int iRandomization, int iModelIndex, int iCount, int iDuration, int iFlags, int msgType )
{
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecPos );
	WRITE_BYTE( TE_BREAKMODEL );
	WRITE_COORD( vecPos.x );
	WRITE_COORD( vecPos.y );
	WRITE_COORD( vecPos.z );
	WRITE_COORD( vecSize.x );
	WRITE_COORD( vecSize.y );
	WRITE_COORD( vecSize.z );
	WRITE_COORD( vecVelocity.x );
	WRITE_COORD( vecVelocity.y );
	WRITE_COORD( vecVelocity.z );
	WRITE_BYTE( iRandomization );
	WRITE_SHORT( iModelIndex );
	WRITE_BYTE( iCount );
	WRITE_BYTE( iDuration );
	WRITE_BYTE( iFlags );
	MESSAGE_END();
}

void Aircraft_CrashBlastCylinder( const Vector &vecOrigin, int iSpriteTexture, int msgType )
{
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecOrigin );
	WRITE_BYTE( TE_BEAMCYLINDER );
	WRITE_COORD( vecOrigin.x );
	WRITE_COORD( vecOrigin.y );
	WRITE_COORD( vecOrigin.z );
	WRITE_COORD( vecOrigin.x );
	WRITE_COORD( vecOrigin.y );
	WRITE_COORD( vecOrigin.z + 2000 ); // reach damage radius over .2 seconds
	WRITE_SHORT( iSpriteTexture );
	WRITE_BYTE( 0 );   // startframe
	WRITE_BYTE( 0 );   // framerate
	WRITE_BYTE( 4 );   // life
	WRITE_BYTE( 32 );  // width
	WRITE_BYTE( 0 );   // noise
	WRITE_BYTE( 255 ); // r
	WRITE_BYTE( 255 ); // g
	WRITE_BYTE( 192 ); // b
	WRITE_BYTE( 128 ); // brightness
	WRITE_BYTE( 0 );   // speed
	MESSAGE_END();
}

void Aircraft_CrashExplosionSprite( const Vector &vecPos, int iSpriteIndex, int iScale, int iBrightness, int msgType )
{
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecPos );
	WRITE_BYTE( TE_SPRITE );
	WRITE_COORD( vecPos.x );
	WRITE_COORD( vecPos.y );
	WRITE_COORD( vecPos.z );
	WRITE_SHORT( iSpriteIndex );
	WRITE_BYTE( iScale );
	WRITE_BYTE( iBrightness );
	MESSAGE_END();
}

void Aircraft_CrashSmoke( const Vector &vecPos, int iSmokeIndex, int iScale, int iFramerate, int msgType )
{
	MESSAGE_BEGIN( msgType, SVC_TEMPENTITY, vecPos );
	WRITE_BYTE( TE_SMOKE );
	WRITE_COORD( vecPos.x );
	WRITE_COORD( vecPos.y );
	WRITE_COORD( vecPos.z );
	WRITE_SHORT( iSmokeIndex );
	WRITE_BYTE( iScale );
	WRITE_BYTE( iFramerate );
	MESSAGE_END();
}
