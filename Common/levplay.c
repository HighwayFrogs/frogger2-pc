/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: levelplaying.c
	Programmer	: David Swift
	Date		: 17/03/99

----------------------------------------------------------------------------------------------- */

#include <islpad.h>
#include <islfile.h>
#include <islmem.h>
#include <islutil.h>
#include <stdio.h>

#include "frontend.h"
#include "define.h"
#include "layout.h"
#include "game.h"
#include "textover.h"
#include "levplay.h"
#include "overlays.h"
#include "hud.h"
#include "fadeout.h"
#include "lang.h"
#include "menus.h"

#ifdef DREAMCAST_VERSION
#include "main.h"
#endif

#define DEMO_TIMEOUT	25
#define NUM_DEMOS		3

unsigned long playKeyCount;
unsigned long curPlayKey;
unsigned long *playKeyList;

TIMER demoTimeout;

int demoLevels[NUM_DEMOS][2] = {
	{ WORLDID_GARDEN,	LEVELID_GARDEN1 },
	{ WORLDID_ANCIENT,	LEVELID_ANCIENT1 },
	{ WORLDID_HALLOWEEN,LEVELID_HALLOWEEN2 }
//	{ WORLDID_ANCIENT,	LEVELID_ANCIENT1 }
};

int nextDemo = 0;

void LoadDemoFile(int world, int level)
{
	char filename[128]; int count;
	sprintf(filename, "DEMOS\\RECORD-%lu-%lu.KEY", world, level);
	
	playKeyList = (unsigned long*)fileLoad(filename, &count);
	
	if (playKeyList)
		playKeyCount = count / (3*4);
}

/*	--------------------------------------------------------------------------------
	Function		: RunDemoLoop
	Purpose			: Runs the demo-mode game loop
	Parameters		: 
	Returns			: 
*/
void InitDemoMode()
{
	int world, level;

	utilPrintf ( "Next Demo : %d\n", nextDemo );
	world = demoLevels[nextDemo][0];
	level = demoLevels[nextDemo][1];

	gameState.multi = SINGLEPLAYER;
	gameState.single = ARCADE_MODE;
	gameState.mode = DEMO_MODE;

	playKeyList = NULL;
	InitLevel(world, level);

	if (!playKeyList)
	{
		utilPrintf("DEMO MODE: Failed loading %s, returning\n"); return;
	}
	curPlayKey = 0;

#ifdef PSX_VERSION
	CreateAndAddTextOverlay(2048, 512, GAMESTRING(STR_DEMO_MODE), YES, 128, NULL, 0);
	displayPage[0].drawenv.isbg = displayPage[1].drawenv.isbg = 1;
#else
	CreateAndAddTextOverlay(2048, 128, GAMESTRING(STR_DEMO_MODE), YES, 128, NULL, 0);
#endif

#ifdef PC_VERSION
	CreateAndAddTextOverlay(2048, 3500, GAMESTRING(STR_PRESSENTER), YES, 128, NULL, 0);
#else
	CreateAndAddTextOverlay(2048, 3500, GAMESTRING(STR_PRESSSTART), YES, 128, NULL, 0);
#endif
//	CreateAndAddSpriteOverlay( 3000, 2700, "BLITZGAMES", 800, 1000, 190, 0 );
//	CreateAndAddSpriteOverlay( 250, 3200, "FROGGER2", 600, 800, 255, 0 );

	GTInit(&demoTimeout, DEMO_TIMEOUT);

	DisableHUD();
}

/*	--------------------------------------------------------------------------------
	Function		: RunDemoLoop
	Purpose			: Runs the demo-mode game loop
	Parameters		: 
	Returns			: 
*/
quittingDemo = NO;
void RunDemoMode()
{
	int pl;

	GTUpdate(&demoTimeout, -1);
	if((quittingDemo) && (fadingOut == 0))
	{
		quittingDemo = NO;
		utilPrintf("We've finished with demo, then exit..........................................\n");

		FREE(playKeyList);

		if ((++nextDemo) == NUM_DEMOS)
			nextDemo = 0;

#ifdef E3_DEMO
		StartE3LevelSelect();
#else
		if( UndoChangeModel( frog[0]->actor ) )
			player[0].idleEnable = 1;

		gameState.difficulty = oldDiffMode;

		oldDiffMode = gameState.difficulty;


		gameState.mode = FRONTEND_MODE;
		player[0].character = FROG_FROGGER;
		InitLevel(WORLDID_FRONTEND, LEVELID_FRONTEND1);
		frameCount = 0; // why? who can say? (me! me!)
#endif
		return;
	}

	if(quittingDemo)
		return;

	if (padData.debounce[0] || curPlayKey >= playKeyCount || !demoTimeout.time)
	{
		quittingDemo = YES;
		ScreenFade(255,0,30);
		keepFade = 0;
	}
	for (pl=0; pl<4; pl++)
		padData.digital[0] = padData.debounce[0] = 0;

	while ((curPlayKey < playKeyCount) && (playKeyList[curPlayKey*3]<actFrameCount))
	{
		pl = playKeyList[(curPlayKey*3)+2];

		padData.digital[pl] |= (unsigned short)playKeyList[(curPlayKey*3)+1];
		padData.debounce[pl] |= ~(unsigned short)playKeyList[((curPlayKey-1)*3)+1] & padData.digital[pl];
		curPlayKey++;
	}

	RunGameLoop();
}
