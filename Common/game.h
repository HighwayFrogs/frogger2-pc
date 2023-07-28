/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: game.h
	Programmer	: Matthew Cloy
	Date		: 12/11/98

----------------------------------------------------------------------------------------------- */

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


#include "maths.h"
#include "general.h"
#include "actor2.h"
#include "textover.h"


// Tile status flags.
#define ATTACH (1<<0)


// Various modes where the game is paused
#define PM_PAUSE	1
#define PM_ENDLEVEL	2
#define PM_GAMEOVER	3

// -----------------

enum
{
	NORMAL_PMODE = 0,
	TWO_PMODE = 1,
	FOUR_PMODE = 2
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct gameStateStruct gameState;

extern unsigned long INPUT_POLLPAUSE;

extern SVECTOR *pointOfInterest;
extern fixed	pOIDistance;

extern TIMER scoreTimer, modeTimer;
extern short showEndLevelScreen;

extern long move;

extern ACTOR2 *levelTrophy;

extern char	scoreText[32];
extern char	timeText[32];

extern long numHops_TOTAL;
extern long speedHops_TOTAL;
extern long numHealth_TOTAL;

extern void RunGameLoop (void);
extern void RunLevelCompleteSequence();
extern void GameProcessController(long player);

extern ACTOR2 *backGnd;

//extern unsigned char swingCam;

//extern unsigned long creditsActive;
//extern unsigned long fadingLogos;

extern unsigned long currTileNum;
extern TEXTOVERLAY *tileNum;
extern char tileString[], dkPressed;
extern long displayingTile;

extern char doubleQueue[4];


extern TIMER screenSaveTimer;
#define PAUSEFADETIMESECS 295
#define PAUSEFADETIME ((PAUSEFADETIMESECS<<12)*60)


extern void SetCamFF(FVECTOR src, FVECTOR tar);

#ifdef __cplusplus
}
#endif
#endif