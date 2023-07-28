/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: frogger.h
	Programmer	: James Healey
	Date		: 23/03/99

----------------------------------------------------------------------------------------------- */

#ifndef FROGGER_H_INCLUDED
#define FROGGER_H_INCLUDED

#include "actor2.h"
#include "specfx.h"
#include "types2d.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_IDLE_TIME			300
#define SPAWN_SCOREUPTIMER		15
#define MAX_FROGS 4

extern long NUM_FROGS;

#define FROG_BREATHE_SPEED	50

//----- [ FROG ANIMATION ENUMS ] -----------------------------------------------------------------

enum
{
	FROG_ANIM_HOP,
	FROG_ANIM_HOPLEFT,
	FROG_ANIM_HOPRIGHT,
	FROG_ANIM_SUPERHOP,
	FROG_ANIM_SOMERSAULT,
	FROG_ANIM_HANDSPRING,
	FROG_ANIM_FALL,
	FROG_ANIM_TONGUE,
	FROG_ANIM_BREATHE,
	FROG_ANIM_LOOKAROUND,
	FROG_ANIM_RETROPOPUP,
	FROG_ANIM_WALLFLIP,
	FROG_ANIM_WALLSPLAT,
	FROG_ANIM_ICE1,
	FROG_ANIM_ICE2,
	FROG_ANIM_PIN,
	FROG_ANIM_PINNED,

	MULTI_NUM_ANIMS, // END OF MULTIPLAYER ANIMATIONS
	
	FROG_ANIM_ICETUBE = MULTI_NUM_ANIMS,
	FROG_ANIM_FALLPUFFED,
	FROG_ANIM_FALLFLAP,
	FROG_ANIM_FALLLAND,
	FROG_ANIM_HOP15,
	FROG_ANIM_HOPWHOOHOO,
	FROG_ANIM_LOOKUP,
	FROG_ANIM_LOOKDOWN,
	FROG_ANIM_PREEN,
	FROG_ANIM_DANCE1,
	FROG_ANIM_DANCE2,
	FROG_ANIM_DANCE3,
	FROG_ANIM_DANCE4,
	FROG_ANIM_BOWLEDOVER,
	FROG_ANIM_STUNG,
	FROG_ANIM_ASSONFIRE,
	FROG_ANIM_SPRAWLY,
	FROG_ANIM_DROWN,
	FROG_ANIM_SPLATFRAME,
	FROG_ANIM_ELECTROCUTE,
	FROG_ANIM_BIGHEAD,
	FROG_ANIM_SPACEFLOAT,
	FROG_ANIM_HOOVER,
	FROG_ANIM_SCREENSPLAT,
	FROG_ANIM_SCREENSLIDE,
	FROG_ANIM_VICTORY,

	NUM_FROG_ANIMS,
};

enum
{
	FROG_FROGGER,
	FROG_LILLIE,
	FROG_BABYFROG,
	FROG_TWEE,
	FROG_WART,
	FROG_ROBOFROG,
	FROG_HOPPER,
	FROG_SWAMPY,

	FROG_NUMFROGS
};


#define FANIM_NONE	0
#define FANIM_ANIMS	1
#define FANIM_ALL	2

//------------------------------------------------------------------------------------------------

typedef struct _FROGSTORE
{
	char fileName[16];
	char singleModel[16];
	char multiModel[16];
	char icon[16];
	char active, anim;			// Can this character be selected, and what animation can it do? (0->multi only, 1->all animations, 2->drop in models)
	unsigned char r, g, b;

} FROGSTORE;

extern FROGSTORE frogPool[FROG_NUMFROGS];


typedef struct _PLAYER
{
	char			name[8];
	unsigned char	character;

	short			frogon;				// Which frog is on this one
	short			frogunder;			// Which frog this one is on

//	unsigned long	score;
	short			numSpawn;
//	short			numCredits;
	short			lives;
	short			oldLives;			// How many lives the player had at the start of the level
	unsigned char	healthPoints;

	unsigned char	deathBy;

	unsigned char	inputPause;			// used for controller input

	short			spawnTimer;
	short			spawnScoreLevel;

	unsigned char 	worldNum;
	unsigned char	levelNum;
	unsigned long	frogState;
//	short			saveSlot;

	fixed			idleTime;
	unsigned char	idleEnable;

	// frog movement related stuff
	unsigned char 	canJump;
	unsigned char	hasJumped;
	unsigned char	hasDoubleJumped;
	unsigned char	isSuperHopping;
	unsigned char	isSinking;

	unsigned char	extendedHopDir;

	TIMER			safe;
	TIMER			stun;
	TIMER			dead;
	TIMER			isCroaking;
	TIMER			autohop;
	TIMER			quickhop;
	TIMER			slowhop;

	//	Parabolic curve variables
	//	See frogmove.c for a longwinded and esoteric explanation
	unsigned long	jumpStartFrame;
	fixed			jumpTime;
	fixed			jumpSpeed;
	fixed			jumpMultiplier;
	FVECTOR			jumpFwdVector;
	FVECTOR			jumpUpVector;
	SVECTOR			jumpOrigin;
	fixed			heightJumped;

} PLAYER;

extern PLAYER player[];
							

extern fixed CROAK_SOUND_RANGE;

extern fixed globalFrogScale;

extern ACTOR2	*frog[];

extern fixed	croakRadius;
extern fixed	croakVelocity;
extern char		croakFade;
extern char		croakFadeDec;
extern char		croakR,croakG,croakB;



void CreateFrogger(unsigned char createFrogActor, unsigned char createFrogOverlays);
void CreateFrogActor (GAMETILE *where, char *name,long p);
void FroggerIdleAnim(int i);

#ifdef __cplusplus
}
#endif

#endif


