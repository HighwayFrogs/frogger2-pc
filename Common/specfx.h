/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: specfx.h
	Programmer	: Andy Eder
	Date		: 14/05/99 09:32:16

----------------------------------------------------------------------------------------------- */

#ifndef SPECFX_H_INCLUDED
#define SPECFX_H_INCLUDED

//#include <d3d.h>
//#include "directx.h"
#include "actor2.h"
#include "sprite.h"

#ifdef __cplusplus
extern "C"
{
#endif

//----- [ DEFINES & ENUMS ] --------------------------------------------------------------------//

#define FADE_IN				0
#define FADE_OUT			1

#define NUM_RINGSEGS 16

extern char pauseMode;


#define ENTITY_PLATFORM		1
#define ENTITY_ENEMY		2


// These are used in the editor and translated into effect parameters later
#define EF_RIPPLE_RINGS					(1 << 0)	// Lillypad thing
#define EF_SMOKE_STATIC					(1 << 1)	// Smoke that doesn't grow
#define EF_SPARKBURST					(1 << 2)	// Explosion of smoke
#define EF_GREENGLOOP					(1 << 3)	// Lab slime thing
#define EF_BATSWARM						(1 << 4)	// Bats
#define EF_BUBBLES						(1 << 5)	// From the fishes
#define EF_SMOKE_GROWS					(1 << 6)	// Grows as it fades
#define EF_SMOKEBURST					(1 << 7)	// Explosion of smoke
#define EF_FIERYSMOKE					(1 << 8)	// Red at base, goes black
#define EF_BUTTERFLYSWARM				(1 << 9)	// Swarm of butterflies. Uses models
#define EF_LASER						(1 << 10)	// Straight bolt
#define EF_TRAIL						(1 << 11)	// Motion trail
#define EF_BILLBOARDTRAIL				(1 << 12)	// Always faces camera
#define EF_LIGHTNING					(1 << 13)	// Emperors hands effect
#define EF_SPACETHING1					(1 << 14)	// Swirly jobby
#define EF_SPARKLYTRAIL					(1 << 15)	// Flash texture
#define EF_GLOW							(1 << 16)	// Around candles and fire and stuff
#define EF_TWINKLE						(1 << 17)	// For gold and gems and things

#define EF_RANDOMCREATE					(1 << 25)
#define EF_FAST							(1 << 26)
#define EF_MEDIUM						(1 << 27)
#define EF_SLOW							(1 << 28)
#define EF_TINTRED						(1 << 29)
#define EF_TINTGREEN					(1 << 30)
#define EF_TINTBLUE						(1 << 31)


// Preset types. Any other can be made by selecting custom and specifying all the stucture members by hand
enum
{
	FXTYPE_CUSTOM,
	FXTYPE_WATERRIPPLE,
	FXTYPE_GARIBCOLLECT,
	FXTYPE_SPARKLYTRAIL,
	FXTYPE_FROGSTUN,
	FXTYPE_SMOKE_STATIC,
	FXTYPE_SMOKE_GROWS,
	FXTYPE_DECAL,
	FXTYPE_SPLASH,
	FXTYPE_SMOKEBURST,
	FXTYPE_GREENGLOOP,
	FXTYPE_BATSWARM,
	FXTYPE_BUBBLES,
	FXTYPE_SPARKBURST,
	FXTYPE_FIERYSMOKE,
	FXTYPE_POLYRING,
	FXTYPE_BUTTERFLYSWARM,
	FXTYPE_LASER,
	FXTYPE_TRAIL,
	FXTYPE_BILLBOARDTRAIL,
	FXTYPE_LIGHTNING,
	FXTYPE_HEALTHFLY,
	FXTYPE_SPACETHING1,
	FXTYPE_FROGSHIELD,
	FXTYPE_GLOW,
	FXTYPE_TWINKLE,
	FXTYPE_WAKE,
	FXTYPE_CROAK,
	FXTYPE_FLASH,

	FXTYPE_NUMTYPES
};


typedef struct TAGSPECFX
{
	struct TAGSPECFX *next, *prev;

	FVECTOR normal, vel, scale;
	SVECTOR origin;
	PLANE2 *rebound;

	struct _PARTICLE *particles;				// For swarm, explosions etc.
	struct TAGSPRITE *sprites;
	TextureType *tex;
	ACTOR *follow;								// Go where it goes
	ACTOR2 **act;								// For models

	short type, start, end, angle, spin;
	fixed speed, accn, tilt, gravity;
	long lifetime, numP,fade,fadeAlpha;				// numP is number of particles

	unsigned char r, g, b, a;

#ifdef PSX_VERSION
//mm This is used to store a translucency flag for those specfx not in the sprite list. This is
// needed cause on the PSX, fading sprites need to be additive. NB It is independant of the flag
// variable in the sprite structure (which is used to store the translucency flag for other specfx
	short	flags;
	int 	zDepthOff;		//allow for the z depth to be altered by zDepthOff : - to bring closer to camera
#endif

	char updateType;							// Index into lastAdded
	void (*Update) (struct TAGSPECFX *);			// Just like C++
	void (*Draw) (struct TAGSPECFX *);			// Update and draw functions, specified for different types.

} SPECFX;


typedef struct
{
	// Statically allocated array of sprites
	SPECFX *array;
	// Number of sprites allocated
	short count;

	// Defines and data for the sprite allocation stack
	SPECFX **stack;
	short stackPtr;

	// Array of pointers to the last effect of each type allocated
	SPECFX **lastAdded;

	// Static head of list
	SPECFX head;

} SPECFXLIST;

extern SPECFXLIST sfxList;

extern int specFXAlpha;

typedef struct
{
	// Statically allocated array of sprites
	PLANE2 *array;
	// Number of sprites allocated
	short count;

	// Defines and data for the sprite allocation stack
	PLANE2 **stack;
	short stackPtr;

} PLANE2LIST;

extern PLANE2LIST p2List;


extern FVECTOR *ringVtx;

extern TextureType *txtrRipple;
extern TextureType *txtrStar;
extern TextureType *txtrSolidRing;
extern TextureType *txtrSmoke;
extern TextureType *txtrRing;
extern TextureType *txtrBubble;
extern TextureType *txtrBlank;
extern TextureType *txtrTrail;
extern TextureType *txtrFlash;
extern TextureType *txtrWarpflash;

SPECFX *CreateSpecialEffect( short type, SVECTOR *origin, FVECTOR *normal, fixed size, fixed speed, fixed accn, fixed lifetime );
SPECFX *CreateSpecialEffectDirect( short type, SVECTOR *origin, FVECTOR *normal, fixed size, fixed speed, fixed accn, fixed lifetime );

void UpdateSpecialEffects( );

extern PLANE2 *AllocateP2( );
extern void DeallocateP2( PLANE2 *p );
extern SPECFX *AllocateFX( int number, int type );
extern void DeallocateFX( SPECFX *head, int number );
void InitSpecFXList( );
void FreeSpecFXList( );

void SetFXColour( SPECFX *fx, unsigned char r, unsigned char g, unsigned char b );
void SetAttachedFXColour( SPECFX *fx, int effects );
void ProcessAttachedEffects( void *entity, int type );

void CreatePickupEffect( int pl, char r1, char g1, char b1, char r2, char g2, char b2 );
void CreateLightningEffect( SVECTOR *p1, SVECTOR *p2, unsigned long effects, long life, int zDepthOff);
void CreateRingEffect( int i );
void BabyCollectEffect( ACTOR2 *baby, GAMETILE *tile, int i );
void CreateExplodeEffect( SVECTOR *pos, FVECTOR *normal );
void CreateRespawnEffect( GAMETILE *tile, int pl );

void PrepForPriorityEffect( );

#ifdef __cplusplus
}
#endif

#endif

