#ifndef CHARGERS_H
#define CHARGERS_H

#include "core/extdll.h"
#include "core/util.h"
#include "core/cbase.h"
#include "core/saverestore.h"
#include "core/skill.h"
#include "gameplay/gamerules.h"
#include "weapons/weapon_defs.h"

class CBaseWallCharger : public CBaseToggle
{
  public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int ObjectCaps( void ) { return ( CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE ) & ~FCAP_ACROSS_TRANSITION; }

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void EXPORT Off( void );
	void EXPORT Recharge( void );

  protected:
	virtual int GetCapacity( void ) const = 0;
	virtual float GetRechargeTime( void ) const = 0;
	virtual BOOL GiveResource( CBaseEntity *pActivator ) = 0;
	virtual const char *GetStartSound( void ) const = 0;
	virtual const char *GetLoopSound( void ) const = 0;
	virtual const char *GetDenySound( void ) const = 0;
	virtual float GetSoundVolume( void ) const { return 1.0f; }

	float m_flNextCharge;
	int m_iReactivate; // DeathMatch delay until reactivated
	int m_iJuice;
	int m_iOn; // 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
};

class CWallHealth : public CBaseWallCharger
{
  protected:
	int GetCapacity( void ) const override { return gSkillData.healthchargerCapacity; }
	float GetRechargeTime( void ) const override { return g_pGameRules->FlHealthChargerRechargeTime(); }
	BOOL GiveResource( CBaseEntity *pActivator ) override { return pActivator->TakeHealth( 1, DMG_GENERIC ); }
	const char *GetStartSound( void ) const override { return "items/medshot4.wav"; }
	const char *GetLoopSound( void ) const override { return "items/medcharge4.wav"; }
	const char *GetDenySound( void ) const override { return "items/medshotno1.wav"; }
};

class CWallRecharge : public CBaseWallCharger
{
  protected:
	int GetCapacity( void ) const override { return gSkillData.suitchargerCapacity; }
	float GetRechargeTime( void ) const override { return g_pGameRules->FlHEVChargerRechargeTime(); }
	BOOL GiveResource( CBaseEntity *pActivator ) override
	{
		if ( pActivator->pev->armorvalue < MAX_NORMAL_BATTERY )
		{
			pActivator->pev->armorvalue++;
			return TRUE;
		}
		return FALSE;
	}
	const char *GetStartSound( void ) const override { return "items/suitchargeok1.wav"; }
	const char *GetLoopSound( void ) const override { return "items/suitcharge1.wav"; }
	const char *GetDenySound( void ) const override { return "items/suitchargeno1.wav"; }
	float GetSoundVolume( void ) const override { return 0.85f; }
};

#endif // CHARGERS_H
