#ifndef WEAPON_SPRAYCAN_H
#define WEAPON_SPRAYCAN_H

class CSprayCan : public CBaseEntity
{
  public:
	void Spawn( entvars_t *pevOwner );
	void Think( void );

	virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

class CBloodSplat : public CBaseEntity
{
  public:
	void Spawn( entvars_t *pevOwner );
	void Spray( void );
};

#endif
