/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: hud.h
	Programmer	: James Healey
	Date		: 31/03/99

----------------------------------------------------------------------------------------------- */


#ifndef HUD_H_INCLUDED
#define HUD_H_INCLUDED

#include "sprite.h"
#include "frogger.h"
#include "types2d.h"
#include "actor2.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HUD_SPARKLES 20

typedef struct TAG_ARCADE_HUD
{
//	SPRITEOVERLAY *sparkles[MAX_HUD_SPARKLES];
	unsigned long sX,sY,sW,sH;
	unsigned long sTime;

//	SPRITEOVERLAY *backLeft;
//	SPRITEOVERLAY *backLeftExtra;

//	SPRITEOVERLAY *backRight;
//	SPRITEOVERLAY *backRightExtra;

//	SPRITEOVERLAY *backCentre;
	
	SPRITEOVERLAY *livesOver;
	TEXTOVERLAY   *livesText;

	TEXTOVERLAY   *goText;
	SPRITEOVERLAY *goSparkles[MAX_HUD_SPARKLES];

	
//	SPRITEOVERLAY *helpOver;
	SPRITEOVERLAY *coinsOver;
	SPRITEOVERLAY *coinsBack;
	SPRITEOVERLAY *babiesBack[6];
	TEXTOVERLAY   *coinsText;
//	TEXTOVERLAY   *coinsText2;
	
	SPRITEOVERLAY *autoHopOver, *quickHopOver;

	SPRITEOVERLAY *timeFaceOver;
	SPRITEOVERLAY *timeHeadOver;
	SPRITEOVERLAY *timeHandOver;

	TEXTOVERLAY	  *speedrunTimer;

	TEXTOVERLAY   *timeTextMin;
	TEXTOVERLAY   *timeTextSec;
	TEXTOVERLAY   *timeTextHSec;

	TEXTOVERLAY	  *collectText;

	TEXTOVERLAY   *parText;

	SPRITEOVERLAY *coins;

//	TEXTOVERLAY   *timeOutText;
	unsigned long timedOut;
	SPRITEOVERLAY *coinZoom;
	SPRITEOVERLAY *timeBar;
	SPRITEOVERLAY *timeBak;
	TEXTOVERLAY   *timeBarText;
	TEXTOVERLAY   *timeOutText;
} ARCADE_HUD;


typedef struct TAG_MULTI_HUD
{
	SPRITEOVERLAY *backChars[MAX_FROGS];
	TEXTOVERLAY   *penaliseText[MAX_FROGS];

	TEXTOVERLAY   *timeTextMin;
	TEXTOVERLAY   *timeTextSec;
	TEXTOVERLAY   *timeTextHSec;

	TEXTOVERLAY   *centreText;
	SPRITEOVERLAY *trophies[MAX_FROGS][3];
	SPRITEOVERLAY *penalOver[MAX_FROGS];
} MULTI_HUD;

extern MULTI_HUD multiHud;
extern ARCADE_HUD arcadeHud;

extern char countdownString[];

extern char countdownTimer;
extern char displayFullScreenPoly;

extern TEXTOVERLAY *babySavedText;
extern TEXTOVERLAY *controllerText;
extern TEXTOVERLAY *continueText;
extern char pauseTitleString[32];
extern TEXTOVERLAY *xselectText, *pauseTitleText;
extern TEXTOVERLAY *restartText;
extern TEXTOVERLAY *quitText;
extern TEXTOVERLAY *garibCount, *creditCount;
extern TEXTOVERLAY *time;

extern void UpDateOnScreenInfo		( void );
extern void UpdateScore				( ACTOR2 *act, long scoreUpdate);
extern void InitInGameTextOverlays	( unsigned long worldID,unsigned long levelID );
extern void RunGameOverSequence		( void );
extern void DisableHUD(void);
extern void EnableHUD(void);
extern void DisableMultiHUD( );
extern void EnableMultiHUD( );
extern void UpDateMultiplayerInfo( void );

#ifdef PSX_VERSION
#define BORDER 200
#else
#define BORDER 0
#endif

#ifdef __cplusplus
}
#endif

#endif