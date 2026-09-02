//========= Copyright ? 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#if !defined( EV_HLDMH )
#define EV_HLDMH

// bullet types
typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,      // glock
	BULLET_PLAYER_MP5,      // mp5
	BULLET_PLAYER_357,      // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR,  // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;

enum glock_e
{
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

enum shotgun_e
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

enum python_e
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

#define GAUSS_PRIMARY_CHARGE_VOLUME 256 // how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME 450   // how loud gauss is when discharged

enum gauss_e
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};

enum crowbar_e
{
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};

enum crossbow_e
{
	CROSSBOW_IDLE1 = 0,
	CROSSBOW_IDLE2,
	CROSSBOW_FIDGET1,
	CROSSBOW_FIDGET2,
	CROSSBOW_FIRE1,
	CROSSBOW_FIRE2,
	CROSSBOW_FIRE3,
	CROSSBOW_RELOAD,
	CROSSBOW_DRAW1,
	CROSSBOW_DRAW2,
	CROSSBOW_HOLSTER1,
	CROSSBOW_HOLSTER2
};

enum rpg_e
{
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,
	RPG_FIRE2,
	RPG_HOLSTER1,
	RPG_DRAW1,
	RPG_HOLSTER2,
	RPG_DRAW2,
	RPG_IDLE2,
	RPG_FIDGET2
};

enum egon_e
{
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW1,
	EGON_HOLSTER1
};

enum hgun_e
{
	HGUN_IDLE1 = 0,
	HGUN_FIDGET1,
	HGUN_ALTFIRE,
	HGUN_SHOOT,
	HGUN_SHOOTEMPTY,
	HGUN_RELOAD,
	HGUN_HOLSTER,
	HGUN_DRAW
};

enum tripmine_e
{
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND
};

enum snark_e
{
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

#define VECTOR_CONE_1DEGREES Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES Vector( 0.17365, 0.17365, 0.17365 )

// Helpers
void EV_GetGunPosition( struct event_args_s *args, float *pos, float *origin );
void EV_GetDefaultShellInfo( struct event_args_s *args, float *origin, float *velocity, float *Angles, float *forward, float *right, float *up, float forwardScale, float upScale, float rightScale );
void EV_EjectBrass( float *origin, float *velocity, float rotation, int model, int soundtype );
float EV_HLDM_PlayTextureSound( int idx, pmtrace_t *ptr, float *vecSrc, float *vecEnd, int iBulletType );
char *EV_HLDM_DamageDecal( physent_t *pe );
void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName );
void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType );
int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY );

// C-linkage event entry points
extern "C"
{
	void EV_FireGlock1( struct event_args_s *args );
	void EV_FireGlock2( struct event_args_s *args );
	void EV_FireShotGunSingle( struct event_args_s *args );
	void EV_FireShotGunDouble( struct event_args_s *args );
	void EV_FireMP5( struct event_args_s *args );
	void EV_FireMP52( struct event_args_s *args );
	void EV_FirePython( struct event_args_s *args );
	void EV_FireGauss( struct event_args_s *args );
	void EV_SpinGauss( struct event_args_s *args );
	void EV_StopPreviousGauss( int idx );
	void EV_Crowbar( struct event_args_s *args );
	void EV_FireCrossbow( struct event_args_s *args );
	void EV_FireCrossbow2( struct event_args_s *args );
	void EV_FireRpg( struct event_args_s *args );
	void EV_EgonFire( struct event_args_s *args );
	void EV_EgonStop( struct event_args_s *args );
	void EV_HornetGunFire( struct event_args_s *args );
	void EV_TripmineFire( struct event_args_s *args );
	void EV_SnarkFire( struct event_args_s *args );
	void EV_TrainPitchAdjust( struct event_args_s *args );
	void EV_HLDM_Init( void );
}

#endif // EV_HLDMH
