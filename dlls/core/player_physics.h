/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

void FixPlayerCrouchStuck( edict_t *pPlayer );
int TrainSpeed( int iSpeed, int iMax );
void CheckPowerups( entvars_t *pev );

#define TRAIN_ACTIVE 0x80
#define TRAIN_NEW 0xc0
#define TRAIN_OFF 0x00

#endif // PLAYER_PHYSICS_H
