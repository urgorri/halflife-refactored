#ifndef WEAPON_SNARK_H
#define WEAPON_SNARK_H

#include "weapons/weapon_base.h"
#include "weapons/projectile_snark.h"

class CSqueak : public CBasePlayerWeapon
{
  public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo( ItemInfo *p );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	int m_fJustThrown;

	virtual BOOL UseDecrement( void )
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

  private:
	unsigned short m_usSnarkFire;
};

#endif // WEAPON_SNARK_H
