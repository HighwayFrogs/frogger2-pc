/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: frontend.h
	Programmer	: Andy Eder (originally Random Healey, but now rewritten)
	Date		: 14/10/99 11:58:26

----------------------------------------------------------------------------------------------- */

#ifndef FRONTEND_H_INCLUDED
#define FRONTEND_H_INCLUDED


#include "types2d.h"
#include "actor2.h"

typedef struct
{
	char *texname;
	unsigned short width,height;
	unsigned short tiles[16];
} LOGO;

#ifdef __cplusplus
extern "C" {
#endif


//----- [ GLOBALS ] ----------------------------------------------------------------------------//

extern int frameCount;
extern int frameCount2;
extern int dispFrameCount;
extern unsigned long levelTime;
extern char	ActiveController;

extern long backCharsX[4];
extern long backCharsY[4];
extern long backTextX[4];
extern long backTextY[4];
extern long backWinsTextX[4];
extern long backWinsTextY[4];

//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

extern void GameLoop(void);

extern void RunLevelSelect();
extern void RunTitleScreen();

// Init routines
extern void	StartLevelComplete( );
extern void	StartWorldComplete( );
extern void	StartGameComplete( );
extern void StartGameIntro( );
extern void StartGameOver();

// Run routines
extern void	RunLevelComplete( );
extern void	RunWorldComplete( );
extern void	RunGameOver( );

extern void RunMultiWinRace( );
extern void RunMultiWinCollect( );
extern void RunMultiWinBattle( );
extern void StartMultiWinGame( );

extern void SetTimeForLevel( );
extern void CreateOverlaysFromLogo(const LOGO *logo, int x, int y);
void DoEndMulti();
void SlideSpriteOverlayToPos(SPRITEOVERLAY *s,long x,long y,long fromX,long fromY,long numFrames);
void SlideTextOverlayToPos(TEXTOVERLAY *s,long x,long y,long fromX,long fromY,long numFrames);
int GetAnimNum(ACTOR2 *actor);
int GetAnimLoop(ACTOR2 *actor);
void SetAnimLoop(ACTOR2 *actor,int loop);
int ReachedEndOfAnim(ACTOR2 *actor);

void CheckEOLLoopTrack();

#ifdef __cplusplus
}
#endif
#endif