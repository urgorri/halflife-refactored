#ifndef MAPRULES_H
#define MAPRULES_H

#include "core/cbase.h"

#define SF_SCORE_NEGATIVE 0x0001
#define SF_SCORE_TEAM 0x0002
#define SF_ENVTEXT_ALLPLAYERS 0x0001
#define SF_TEAMMASTER_FIREONCE 0x0001
#define SF_TEAMMASTER_ANYTEAM 0x0002
#define SF_TEAMSET_FIREONCE 0x0001
#define SF_TEAMSET_CLEARTEAM 0x0002
#define SF_PKILL_FIREONCE 0x0001
#define SF_GAMECOUNT_FIREONCE 0x0001
#define SF_GAMECOUNT_RESET 0x0002
#define SF_GAMECOUNTSET_FIREONCE 0x0001
#define SF_PLAYEREQUIP_USEONLY 0x0001
#define MAX_EQUIP 32
#define SF_PTEAM_FIREONCE 0x0001
#define SF_PTEAM_KILL 0x0002
#define SF_PTEAM_GIB 0x0004

class CRuleEntity : public CBaseEntity
{
  public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void SetMaster( int iszMaster ) { m_iszMaster = iszMaster; }

  protected:
	BOOL CanFireForActivator( CBaseEntity *pActivator );

  private:
	string_t m_iszMaster;
};

//
// CRulePointEntity -- base class for all rule "point" entities (not brushes)
//
class CRulePointEntity : public CRuleEntity
{
  public:
	void Spawn( void );
};

//
// CRuleBrushEntity -- base class for all rule "brush" entities
//
class CRuleBrushEntity : public CRuleEntity
{
  public:
	void Spawn( void );
};

class CGameScore : public CRulePointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	inline int Points( void ) { return pev->frags; }
	inline BOOL AllowNegativeScore( void ) { return pev->spawnflags & SF_SCORE_NEGATIVE; }
	inline BOOL AwardToTeam( void ) { return pev->spawnflags & SF_SCORE_TEAM; }
	inline void SetPoints( int points ) { pev->frags = points; }
};

class CGameEnd : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

class CGameText : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	inline BOOL MessageToAll( void ) { return ( pev->spawnflags & SF_ENVTEXT_ALLPLAYERS ); }
	inline void MessageSet( const char *pMessage ) { pev->message = ALLOC_STRING( pMessage ); }
	inline const char *MessageGet( void ) { return STRING( pev->message ); }

  private:
	hudtextparms_t m_textParms;
};

class CGameTeamMaster : public CRulePointEntity
{
  public:
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps( void ) { return CRulePointEntity::ObjectCaps() | FCAP_MASTER; }

	BOOL IsTriggered( CBaseEntity *pActivator );
	const char *TeamID( void );
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_TEAMMASTER_FIREONCE ) ? TRUE : FALSE; }
	inline BOOL AnyTeam( void ) { return ( pev->spawnflags & SF_TEAMMASTER_ANYTEAM ) ? TRUE : FALSE; }

  private:
	BOOL TeamMatch( CBaseEntity *pActivator );

	int m_teamIndex;
	USE_TYPE triggerType;
};

class CGameTeamSet : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_TEAMSET_FIREONCE ) ? TRUE : FALSE; }
	inline BOOL ShouldClearTeam( void ) { return ( pev->spawnflags & SF_TEAMSET_CLEARTEAM ) ? TRUE : FALSE; }
};

class CGamePlayerHurt : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_PKILL_FIREONCE ) ? TRUE : FALSE; }
};

class CGameCounter : public CRulePointEntity
{
  public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_GAMECOUNT_FIREONCE ) ? TRUE : FALSE; }
	inline BOOL ResetOnFire( void ) { return ( pev->spawnflags & SF_GAMECOUNT_RESET ) ? TRUE : FALSE; }

	inline void CountUp( void ) { pev->frags++; }
	inline void CountDown( void ) { pev->frags--; }
	inline void ResetCount( void ) { pev->frags = pev->dmg; }
	inline int CountValue( void ) { return pev->frags; }
	inline int LimitValue( void ) { return pev->health; }

	inline BOOL HitLimit( void ) { return CountValue() == LimitValue(); }

  private:
	inline void SetCountValue( int value ) { pev->frags = value; }
	inline void SetInitialValue( int value ) { pev->dmg = value; }
};

class CGameCounterSet : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_GAMECOUNTSET_FIREONCE ) ? TRUE : FALSE; }
};

class CGamePlayerEquip : public CRulePointEntity
{
  public:
	void KeyValue( KeyValueData *pkvd );
	void Touch( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	inline BOOL UseOnly( void ) { return ( pev->spawnflags & SF_PLAYEREQUIP_USEONLY ) ? TRUE : FALSE; }

  private:
	void EquipPlayer( CBaseEntity *pPlayer );

	string_t m_weaponNames[MAX_EQUIP];
	int m_weaponCount[MAX_EQUIP];
};

class CGamePlayerTeam : public CRulePointEntity
{
  public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

  private:
	inline BOOL RemoveOnFire( void ) { return ( pev->spawnflags & SF_PTEAM_FIREONCE ) ? TRUE : FALSE; }
	inline BOOL ShouldKillPlayer( void ) { return ( pev->spawnflags & SF_PTEAM_KILL ) ? TRUE : FALSE; }
	inline BOOL ShouldGibPlayer( void ) { return ( pev->spawnflags & SF_PTEAM_GIB ) ? TRUE : FALSE; }

	const char *TargetTeamName( const char *pszTargetName );
};

class CGamePlayerZone : public CRuleBrushEntity
{
  public:
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

  private:
	string_t m_iszInTarget;
	string_t m_iszOutTarget;
	string_t m_iszInCount;
	string_t m_iszOutCount;
};

#endif // MAPRULES_H
