#ifndef ITEM_BASE_H
#define ITEM_BASE_H

#include "core/cbase.h"

class CItem : public CBaseEntity
{
  public:
	void Spawn( void );
	CBaseEntity *Respawn( void );
	void EXPORT ItemTouch( CBaseEntity *pOther );
	void EXPORT Materialize( void );
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; };
};

#define IMPLEMENT_WORLD_ITEM( className, entityName, modelPath, precacheCode, touchBody ) \
class className : public CItem                                                            \
{                                                                                         \
  public:                                                                                 \
	void Spawn( void ) override                                                           \
	{                                                                                     \
		Precache();                                                                       \
		SET_MODEL( ENT( pev ), modelPath );                                               \
		CItem::Spawn();                                                                   \
	}                                                                                     \
	void Precache( void ) override                                                        \
	{                                                                                     \
		PRECACHE_MODEL( (char *)modelPath );                                              \
		precacheCode                                                                      \
	}                                                                                     \
	BOOL MyTouch( CBasePlayer *pPlayer ) override                                         \
	{                                                                                     \
		touchBody                                                                         \
	}                                                                                     \
};                                                                                        \
LINK_ENTITY_TO_CLASS( entityName, className );

#endif // ITEM_BASE_H
