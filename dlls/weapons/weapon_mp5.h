#pragma once

#include "core/cbase.h"
#include "weapons.h"

class CMP5 : public CBasePlayerWeapon
{
  public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 3; }
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;

	virtual BOOL UseDecrement( void )
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

  private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;
};

class CMP5AmmoClip : public CBasePlayerAmmo
{
  public:
	void Spawn( void );
	void Precache( void );
	BOOL AddAmmo( CBaseEntity *pOther );
};

class CMP5Chainammo : public CBasePlayerAmmo
{
  public:
	void Spawn( void );
	void Precache( void );
	BOOL AddAmmo( CBaseEntity *pOther );
};

class CMP5AmmoGrenade : public CBasePlayerAmmo
{
  public:
	void Spawn( void );
	void Precache( void );
	BOOL AddAmmo( CBaseEntity *pOther );
};
