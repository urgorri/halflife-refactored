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
#ifndef TRACKCHANGE_H
#define TRACKCHANGE_H

#include "world/plats.h"
#include "world/trains.h"

typedef enum
{
	TRAIN_SAFE,
	TRAIN_BLOCKING,
	TRAIN_FOLLOWING
} TRAIN_CODE;

#define SF_TRACK_ACTIVATETRAIN 0x00000001
#define SF_TRACK_RELINK 0x00000002
#define SF_TRACK_ROTMOVE 0x00000004
#define SF_TRACK_STARTBOTTOM 0x00000008
#define SF_TRACK_DONT_MOVE 0x00000010

#define SF_AUTO_FIREONCE 0x00000001
#define SF_AUTO_REVERSING 0x00000002

class CFuncTrackChange : public CFuncPlatRot
{
  public:
	void Spawn( void );
	void Precache( void );

	virtual void EXPORT GoUp( void );
	virtual void EXPORT GoDown( void );

	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT Find( void );
	TRAIN_CODE EvaluateTrain( CPathTrack *pcurrent );
	void EXPORT UpdateTrain( void );
	void EXPORT Touch( CBaseEntity *pOther );
	virtual void Blocked( CBaseEntity *pOther );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	virtual void OverrideReset( void );

	CPathTrack *m_trackTop;
	CPathTrack *m_trackBottom;

	CFuncTrackTrain *m_train;
	string_t m_trackTopName;
	string_t m_trackBottomName;
	string_t m_trainName;
	int m_code;
	int m_targetState;
	int m_use;
};

class CFuncTrackAuto : public CFuncTrackChange
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void UpdateAutoTargets( int toggleState );
	void TriggerTrackChange( USE_TYPE useType, float value );
	void EXPORT UpdateThink( void );
};

#endif // TRACKCHANGE_H
