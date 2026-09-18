#ifndef AIRCRAFT_FX_H
#define AIRCRAFT_FX_H

#include "core/extdll.h"
#include "core/util.h"

// Emit random mid-air explosion flash and smoke puff during falling phase
void Aircraft_FallingEffects( const Vector &vecOrigin, int modelIndexFireball, int modelIndexSmoke, int msgType = MSG_PVS );

// Emit breakmodel debris shards (TE_BREAKMODEL)
void Aircraft_BreakModel( const Vector &vecPos, const Vector &vecSize, const Vector &vecVelocity,
                          int iRandomization, int iModelIndex, int iCount, int iDuration, int iFlags = 0x02 /* BREAK_METAL */, int msgType = MSG_PVS );

// Emit expanding shockwave blast cylinder (TE_BEAMCYLINDER)
void Aircraft_CrashBlastCylinder( const Vector &vecOrigin, int iSpriteTexture, int msgType = MSG_PVS );

// Emit impact fireball explosion sprite (TE_SPRITE)
void Aircraft_CrashExplosionSprite( const Vector &vecPos, int iSpriteIndex, int iScale, int iBrightness = 255, int msgType = MSG_PVS );

// Emit heavy smoke puff (TE_SMOKE)
void Aircraft_CrashSmoke( const Vector &vecPos, int iSmokeIndex, int iScale, int iFramerate, int msgType = MSG_PVS );

#endif // AIRCRAFT_FX_H
