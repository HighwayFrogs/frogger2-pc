/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File			: E3_Demo.c
	Programmer		: David Swift
	Date			: 14/04/2000
	Comments		: E3_Demo.c is part of the demo for E3'00

----------------------------------------------------------------------------------------------- */

#include <islpad.h>
#include <islutil.h>
#include <islmem.h>

#include "frogger.h"
#include "frontend.h"
#include "backdrop.h"
#include "textover.h"
#include "main.h"
#include "define.h"
#include "game.h"
#include "overlays.h"
#include "sprite.h"
#include "e3_demo.h"
#include "layout.h"
#include "multi.h"
#include "levplay.h"

#ifdef PC_VERSION
#include <banks.h>
#include <pcaudio.h>
#endif

#ifdef PSX_VERSION
#include "audio.h"
#endif


#define E3_MULTI_LEVEL WORLDID_SUPERRETRO
#define NUMSPARKLES 30

#ifdef PSX_VERSION
char e3multi = 0;
#else
char e3multi = 1;
#endif

short e3items;

const int textpos[4][2] = { { 280, 1458 }, { 280, 2012 }, { 280, 2592 }, { 280, 3198 } };

const char* levelseltext[4] = { "Boulder Canyon", "Haunted House", "Space Chase", "2 Player" };
const char* levelicons[4] = { "ANCIENTS1", "HALLOWEEN2", "SPACERIDE2", "RETROMULTI" };
typedef struct _E3LEVELSELSTUFF
{
	SPRITEOVERLAY *levels[4];
	TEXTOVERLAY *leveltxt;
	TEXTOVERLAY *t[4];
	int sel;
	unsigned long prevSparkleTime;
	TIMER demoMode;
} E3LEVELSELSTUFF;

E3LEVELSELSTUFF *e3l;

void StartE3LevelSelect(void)
{
	int i;

#ifdef PSX_VERSION
	char fileName[64];
	FreeAllLists();
#endif

	InitLoadingScreen( "LOADING01" );

	e3items = e3multi?4:3; 
	player[0].character = FROG_FROGGER;
	NUM_FROGS=1;

	if( gameState.multi != SINGLEPLAYER )
		FreeMultiplayer( );

	UpdateLoadingScreen( 10 );

	gameState.multi = SINGLEPLAYER;

#ifdef PC_VERSION
	FreeAllLists();

	PrintTextureInfo();
#endif

	UpdateLoadingScreen( 20 );

	utilPrintf("Starting E3 level select mode ----------------------\n");
	LoadTextureBank(INGAMEGENERIC_TEX_BANK);
	utilPrintf("Starting E3 level select mode ----------------------\n");
	LoadTextureBank(FRONTEND_TEX_BANK);
	
	utilPrintf("Starting E3 level select mode ----------------------\n");
	UpdateLoadingScreen( 20 );

	InitSampleList();
	LoadSfx(0xFFFF);

	UpdateLoadingScreen( 20 );

	UpdateLoadingScreen( 20 );

	gameState.mode = E3_LEVELSELECT_MODE;

	e3l = (E3LEVELSELSTUFF*)MALLOC0(sizeof(E3LEVELSELSTUFF));
	e3l->sel = 0;
	NUM_FROGS = 0;
	GTInit(&e3l->demoMode, 10);

	for (i=0; i<e3items; i++)
	{
		e3l->t[i] = CreateAndAddTextOverlay(
			textpos[i][0], textpos[i][1], (char*)levelseltext[i], 0, 255, fontSmall, 0);

		e3l->t[i]->r = e3l->t[i]->b = 128, e3l->t[i]->g = 255;
		e3l->levels[i] = CreateAndAddSpriteOverlay(2500, 1680, (char*)levelicons[i], 819, 1092, 255, 0);
		e3l->levels[i]->draw = 0;
	}

	UpdateLoadingScreen( 100 );

	FreeLoadingScreen( );

#ifdef PSX_VERSION
	sprintf(fileName, "LEVELSEL" );
	InitBackdrop ( fileName );
#else
	InitBackdrop ("LEVELSEL");
#endif
	/*
	//e3l->leveltxt = CreateAndAddTextOverlay(2200, 2700, (char*)levelseltext[0], 0, 255, font, 0, 0);
#ifdef PC_VERSION
	CreateAndAddTextOverlay(2100, 2900, "Press Enter", 0, 192, font, 0, 0);
#else
	CreateAndAddTextOverlay(2100, 2900, "Press Start", 0, 192, font, 0, 0);
#endif
	*/
#ifdef PSX_VERSION
	bb_InitXA();
#endif

	PrepareSong(WORLDID_FRONTEND,1);

}

void RunE3LevelSelect ( void )
{
	int i;

	if (padData.debounce[0])
		GTInit(&e3l->demoMode, 15);
	
	GTUpdate(&e3l->demoMode, -1);
	if (!e3l->demoMode.time)
	{
#ifdef PSX_VERSION
/*		if ( demoLevels[nextDemo][0] == 0 )
		{
			GTInit(&e3l->demoMode, 15);
			nextDemo++;
			utilPrintf ( "Next Demo Will Be : %d\n", nextDemo );

			return;
		}*/
		// ENDIF
#endif

		NUM_FROGS = 1;
		FREE(e3l);
		FreeAllLists();
		FreeBackdrop();
		InitDemoMode();
		return;
	}

	for (i=0; i<e3items; i++)
	{
		if (i==e3l->sel)
		{
			e3l->levels[i]->draw = 1;
			e3l->t[i]->r = (rcos(actFrameCount*250)>>5)+128;
			e3l->t[i]->b = (rsin(actFrameCount*200)>>5)+128;
			e3l->t[i]->g = 255;
		}
		else
		{
			e3l->levels[i]->draw = 0;
			e3l->t[i]->r = e3l->t[i]->b = 128, e3l->t[i]->g = 192;
		}
	}

	if (padData.debounce[0] & PAD_UP)
	{
		if (e3l->sel > 0)
		{
			e3l->sel--;
			PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, 45 + Random(40));
		}
	}
	else if (padData.debounce[0] & PAD_DOWN)
	{
		if (e3l->sel < e3items-1)
		{
			e3l->sel++;
			PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, 45 + Random(40));
		}
	}
	else if (padData.debounce[0] & (PAD_CROSS | PAD_START))
	{
		int sel = e3l->sel;

		FREE(e3l);
		FreeBackdrop();
		//FreeAllLists();

		gameState.mode = INGAME_MODE;
		gameState.multi = SINGLEPLAYER;
		gameState.single = STORY_MODE;
		NUM_FROGS = 1;

		switch (sel)
		{
		case 0:
			InitLevel(WORLDID_ANCIENT, LEVELID_ANCIENT1);
			return;

		case 1:
			InitLevel(WORLDID_HALLOWEEN, LEVELID_HALLOWEEN2);
			return;

		case 2:
			InitLevel(WORLDID_SPACE, LEVELID_SPACE2);
			return;

		case 3:
			gameState.multi = MULTILOCAL;
			multiplayerMode = multiGameTypes[E3_MULTI_LEVEL];

			NUM_FROGS = 2;
			player[0].character = FROG_FROGGER;
			player[1].character = FROG_LILLIE;
			InitLevel(E3_MULTI_LEVEL, multiLevelIndex[E3_MULTI_LEVEL]);
			return;
		}
	}
}
