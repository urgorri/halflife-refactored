#ifndef TRIGGERS_BRUSH_H
#define TRIGGERS_BRUSH_H

#include "core/cbase.h"

#define SF_TRIGGER_HURT_TARGETONCE 1
#define SF_TRIGGER_HURT_START_OFF 2
#define SF_TRIGGER_HURT_NO_CLIENTS 8
#define SF_TRIGGER_HURT_CLIENTONLYFIRE 16
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32
#define SF_TRIGGER_PUSH_START_OFF 2
#define SF_ENDSECTION_USEONLY 0x0001
#define SF_CHANGELEVEL_USEONLY 0x0002

class CBaseTrigger : public CBaseToggle
{
  public:
	void EXPORT TeleportTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT MultiTouch( CBaseEntity *pOther );
	void EXPORT HurtTouch( CBaseEntity *pOther );
	void EXPORT CDAudioTouch( CBaseEntity *pOther );
	void ActivateMultiTrigger( CBaseEntity *pActivator );
	void EXPORT MultiWaitOver( void );
	void EXPORT CounterUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT ToggleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void InitTrigger( void );

	virtual int ObjectCaps( void ) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

class CTriggerMultiple : public CBaseTrigger
{
  public:
	void Spawn( void );
};

class CTriggerOnce : public CTriggerMultiple
{
  public:
	void Spawn( void );
};

class CTriggerCounter : public CBaseTrigger
{
  public:
	void Spawn( void );
};

class CTriggerHurt : public CBaseTrigger
{
  public:
	void Spawn( void );
	void EXPORT RadiationThink( void );
};

class CTriggerPush : public CBaseTrigger
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Touch( CBaseEntity *pOther );
};

class CTriggerTeleport : public CBaseTrigger
{
  public:
	void Spawn( void );
};

class CLadder : public CBaseTrigger
{
  public:
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );
};

class CTriggerVolume : public CPointEntity
{
  public:
	void Spawn( void );
};

class CTriggerMonsterJump : public CBaseTrigger
{
  public:
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void Think( void );
};

class CTriggerGravity : public CBaseTrigger
{
  public:
	void Spawn( void );
	void EXPORT GravityTouch( CBaseEntity *pOther );
};

class CFrictionModifier : public CBaseEntity
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT ChangeFriction( CBaseEntity *pOther );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	static TYPEDESCRIPTION m_SaveData[];

	float m_frictionFraction;
};

class CTriggerEndSection : public CBaseTrigger
{
  public:
	void Spawn( void );
	void EXPORT EndSectionTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT EndSectionUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

class CTriggerCDAudio : public CBaseTrigger
{
  public:
	void Spawn( void );
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void PlayTrack( void );
	void Touch( CBaseEntity *pOther );
};

class CTriggerSave : public CBaseTrigger
{
  public:
	void Spawn( void );
	void EXPORT SaveTouch( CBaseEntity *pOther );
};

class CChangeLevel : public CBaseTrigger
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void EXPORT UseChangeLevel( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TriggerChangeLevel( void );
	void EXPORT ExecuteChangeLevel( void );
	void EXPORT TouchChangeLevel( CBaseEntity *pOther );
	void ChangeLevelNow( CBaseEntity *pActivator );

	static edict_t *FindLandmark( const char *pLandmarkName );
	static int ChangeList( LEVELLIST *pLevelList, int maxList );
	static int AddTransitionToList( LEVELLIST *pLevelList, int listCount, const char *pMapName, const char *pLandmarkName, edict_t *pentLandmark );
	static int InTransitionVolume( CBaseEntity *pEntity, char *pVolumeName );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );

	static TYPEDESCRIPTION m_SaveData[];

	char m_szMapName[cchMapNameMost];
	char m_szLandmarkName[cchMapNameMost];
	int m_changeTarget;
	float m_changeTargetDelay;
};

void PlayCDTrack( int iTrack );
void NextLevel( void );

#endif // TRIGGERS_BRUSH_H
