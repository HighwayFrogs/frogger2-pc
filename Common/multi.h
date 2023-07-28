/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: multi.h
	Programmer	: Jim Hubbard
	Date		: 25/3/00

----------------------------------------------------------------------------------------------- */

#ifndef __MULTI_H
#define __MULTI_H


#ifdef __cplusplus
extern "C" {
#endif

#include "define.h"
#include "specfx.h"
#include "types.h"
#include "general.h"
#include "enemies.h"
#include "layout.h"


#define MULTIMODE_RACE				1
#define MULTIMODE_BATTLE			2
#define MULTIMODE_COLLECT			3


#define MULTI_BATTLE_MAXITEMS		15
#define MULTI_BATTLE_TRAILLENGTH	8

#define MULTI_NUM_CHARS				9

#define MULTI_RACE_TIMEPENALTY		60

#define MULTI_ROUND_DRAW			0x7F


typedef struct _BATTLENODE
{
	struct _BATTLENODE *next, *prev;// Next and previous pathnode in path

	GAMETILE *tile;					// Node is on this tile
//	SVECTOR to;						// Vector to next node
	ACTOR2 *fx;						// Used in battle multiplayer mode to make tiles look dangerous

} BATTLENODE;

typedef struct
{
	BATTLENODE *array;
	short count;

	BATTLENODE **stack;
	short stackPtr;

} BATTLENODELIST;


typedef struct
{
	short wins;				// Race and battle
	short lap;

	unsigned long timer;	// Race
	unsigned long penalty;	// Race

	short check;		// Race
	short score;		// Battle

	short trail;			// Battle

//	unsigned char r, g, b;	// Both

	char ready;				// Both
	short lasttile;			// Battle

	BATTLENODE *path;		// Battle

	short scrX,scrY;
	TEXTOVERLAY *penalText;	// haha... penal.. - ds
	TEXTOVERLAY *numText;
#ifdef PC_VERSION
	TEXTOVERLAY *nameText;
#endif
} MPINFO;

extern char multiGameTypes[MAX_WORLDS];

extern TIMER multiTimer,endTimer;
extern signed char gameWinner, matchWinner;
extern MPINFO mpl[];
extern int multiplayerMode;
extern unsigned long numMultiItems;

extern char multiLevelIndex[];

void RunMultiplayer( );
void ResetMultiplayer( );
void ReinitialiseMultiplayer();
void FreeMultiplayer( );

void RaceRespawn( int pl );
void PickupBabyFrogMulti( ENEMY *baby, int pl );
void KillMPFrog(int pl);

void CalcMPCamera( FVECTOR *target );

char IsPointVisible(SVECTOR *pos);
void RestartMulti();

void PlayerRaceWin( int pl );

#ifdef __cplusplus
}
#endif


#endif