/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: babyfrog.h
	Programmer	: James Healey
	Date		: 17/03/99

----------------------------------------------------------------------------------------------- */


#ifndef BABYFROGS_H_INCLUDED
#define BABYFROGS_H_INCLUDED

#define NUM_BABIES 5

#include "define.h"
#include "actor2.h"
#include "overlays.h"
#include "enemies.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BABY_ACTIVE_RADIUS	3000
#define BABY_IDLE_TIME		150

enum
{
	BABY_ANIM_SNOOZE,
	BABY_ANIM_HOP,
	BABY_ANIM_WAVE,
	BABY_ANIM_WAGGLE,
	BABY_ANIM_FLIP,
	BABY_ANIM_WHEEL,
	BABY_ANIM_TWOWAVE,
	BABY_ANIM_OI,
	BABY_ANIM_LOOKAROUND,
	BABY_ANIM_FLOAT,
	BABY_ANIM_COLLECT,
	BABY_ANIM_COLLECTHOLD,
};

// baby stuff
typedef struct TAGBABY
{
	unsigned long isSaved;
	unsigned char fxColour[4];
	long idle, collect;
	ACTOR2 *baby;
	ENEMY *enemy;

} BABY;

extern BABY babyList[NUM_BABIES];
extern int lastBabySaved;
extern fixed BABY_SCALE;

extern SPRITEOVERLAY *babyIcons[NUM_BABIES];
extern unsigned long babiesSaved;

void InitBabyList(unsigned char createOverlays);
void ResetBabies( );
int PickupBabyFrog( ACTOR2 *baby, GAMETILE *tile );
int GetNearestBabyFrog();
void UpdateBabies( );


#ifdef __cplusplus
}
#endif
#endif