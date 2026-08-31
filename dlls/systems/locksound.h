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
#ifndef LOCKSOUND_H
#define LOCKSOUND_H

typedef struct locksounds // sounds that doors and buttons make when locked/unlocked
{
	string_t sLockedSound;      // sound a door makes when it's locked
	string_t sLockedSentence;   // sentence group played when door is locked
	string_t sUnlockedSound;    // sound a door makes when it's unlocked
	string_t sUnlockedSentence; // sentence group played when door is unlocked

	int iLockedSentence;   // which sentence in sentence group to play next
	int iUnlockedSentence; // which sentence in sentence group to play next

	float flwaitSound;    // time delay between playing consecutive 'locked/unlocked' sounds
	float flwaitSentence; // time delay between playing consecutive sentences
	BYTE bEOFLocked;      // true if hit end of list of locked sentences
	BYTE bEOFUnlocked;    // true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds( entvars_t *pev, locksound_t *pls, int flocked, int fbutton );

#endif // LOCKSOUND_H
