/*

	This file is part of Frogger2, (c) 1997 Interactive Studios Ltd.


	File		: menus.h
	Programmer	: James (Random) Healey
	Date		: 27/05/99

----------------------------------------------------------------------------------------------- */

#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED

#include "overlays.h"
#include "sprite.h"
#include "actor2.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOOK_NUM_PAGES 10

#define TILENUM_START	204
#define TILENUM_OPTIONS 1
#define TILENUM_CHOICE	2
#define TILENUM_MULTI	3
#define TILENUM_ARCADE	4
#define TILENUM_BOOK	5

extern long tValue;
extern fixed fadeSpeed;
extern unsigned long cWorld;
extern int fogStore;
extern int pauseConfirmMode;
extern int pauseGameSpeed;

extern int oldDiffMode;

#define DEC_ALPHA(x) \
{															\
	tValue = FRoundRandomHack(FMul(gameSpeed,fadeSpeed));	\
	if (x->a > tValue)										\
	{														\
		x->a -= tValue;										\
	}														\
	else													\
	{														\
		x->a = 0;											\
		x->draw = 0;										\
	}														\
}

#define INC_ALPHA(x,y) {tValue = FRoundRandomHack(FMul(gameSpeed,fadeSpeed));	x->draw = 1; if (x->a + tValue < y)	{x->a += tValue;} else {x->a = y;}}
#define MAX_LEVELSTRING 10
#define MAX_SOUND_VOL 100


extern int quittingLevel;
extern short titleHudY[2][4];
extern SPRITEOVERLAY *titleHud[4];

typedef struct
{
	short keyString[12];
	char state;
	signed char toggle;
}CHEAT_COMBO;


enum
{
	CHEAT_OPEN_ALL_LEVELS,
	CHEAT_INFINITE_LIVES,
	CHEAT_OPEN_ALL_CHARS,
	CHEAT_OPEN_ALL_EXTRAS,
	CHEAT_INVULNERABILITY,
	CHEAT_SKIP_LEVEL,
	CHEAT_MAD_GARIBS,
	CHEAT_EXTRA_LEVELS,
	NUMCHEATCOMBOS,
};

extern CHEAT_COMBO cheatCombos[NUMCHEATCOMBOS];

typedef struct LEVELNAMESTRUCT
{
	char name[64];
} LNAMESTRUCT;

extern SPRITE *sp;
extern SPRITEOVERLAY *sprOver;

//extern SPRITEOVERLAY *atari;
extern SPRITEOVERLAY *konami;
extern SPRITEOVERLAY *blitzLogo;
//extern SPRITEOVERLAY *flogo[];

extern char playDemos;
extern char debugMode;
extern long globalSoundVol;
extern long globalMusicVol;
extern FVECTOR storeCamSource;
extern FVECTOR storeCamTarget;
extern char *chapterPic[BOOK_NUM_PAGES];

extern char levelStr[MAX_LEVELSTRING][64];


void RunTitleScreen();
void RunLevelSelect();
void StartPauseMenu();
void RunPauseMenu();
void RunFrontendGameLoop (void);
void DrawPageB();
void CheckForExtras();
void UpdateCheatText();


extern long numPlayers;
extern TextureType *arcadeScreenTex;


#ifdef __cplusplus
}
#endif
#endif
