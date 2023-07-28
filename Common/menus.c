/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: menus.c
	Programmer	: James Healey
	Date		: 27/05/99

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2
#define NSLIPT 4

#include <isltex.h>
#include <islpad.h>
#include <islutil.h>
#include "ultra64.h"
#include "menus.h"
#include "layout.h"
#include "frontend.h"
#include "textover.h"
#include "frogger.h"
#include "game.h"
#include "hud.h"
#include "actor2.h"
#include "Shell.h"
#include "frogmove.h"
#include "cam.h"
#include "platform.h"
#include "enemies.h"
#include "babyfrog.h"
#include "event.h"
#include "collect.h"
#include "levplay.h"
#include "multi.h"
#include "game.h"
#include "lang.h"
#include "options.h"
#include "story.h"
#include "fadeout.h"

#ifdef PC_VERSION
#include <pcmisc.h>
#include <stdio.h>
#include <pcaudio.h>
#include <Main.h>
#include <controll.h>
#else
#include "Textures.h"
#include "temp_psx.h"
#include "audio.h"
#include "temp_psx.h"
#include "memcard.h"
#endif


#define LONG_DEMO_WAIT 300

extern int pauseController;
int restartingLevel = NO;
int fogStore;
int quittingLevel = 0;
int doneTraining = 0;
int lastArcade = 0;

extern int maxPlayers;

#define SHOW_ME_THE_TILE_NUMBERS 1

FVECTOR storeCamSource;
FVECTOR storeCurrCamSource;
FVECTOR storeCamTarget;
FVECTOR storeCurrCamTarget;
FVECTOR storeCamDist;
FVECTOR storeCurrCamDist;
FVECTOR storeCamOffset;
FVECTOR storeCurrCamOffset;
FVECTOR storeCamVect;

int oldDiffMode = 0;


int pauseConfirmMode;
int pauseGameSpeed;


SPRITEOVERLAY *frogLogo = NULL;
//SPRITEOVERLAY *atari = NULL;
SPRITEOVERLAY *konami = NULL;
SPRITEOVERLAY *blitzLogo = NULL;
int fadingLogos = NO;
//SPRITEOVERLAY *flogo[10];

void GameProcessController(long pl);


#ifdef FINAL_MASTER
char playDemos = 1;
#else
char playDemos = 0;
#endif

#ifdef PC_VERSION
 char debugKeys = 0;
#else
 #if GOLDCD==0
  char debugKeys = 1;
 #else
  char debugKeys = 0;
 #endif
#endif

extern psFont *font;
extern psFont *fontSmall;
extern psFont *fontWhite;

// pause mode cheat combos

#define CHEAT_TEXT_TIME 2

int cheatActivated = -1;
TIMER cheatTimer;

TEXTOVERLAY *cheatText = NULL;
char cheatStr[64] = "";

CHEAT_COMBO cheatCombos[NUMCHEATCOMBOS] = 
{
	{{PAD_UP,   PAD_DOWN, PAD_LEFT, PAD_RIGHT, PAD_RIGHT, PAD_RIGHT, PAD_DOWN,  PAD_LEFT,  0},0,-1},//CHEAT_OPEN_ALL_LEVELS
	{{PAD_DOWN, PAD_DOWN, PAD_UP,   PAD_DOWN,  PAD_RIGHT, PAD_DOWN,  PAD_UP,    PAD_UP,    0},0,1},	//CHEAT_INFINITE_LIVES
	{{PAD_LEFT, PAD_RIGHT,PAD_LEFT, PAD_LEFT,  PAD_LEFT,  PAD_UP,    PAD_LEFT,  PAD_LEFT,  0},0,0},	//CHEAT_OPEN_ALL_CHARS
	{{PAD_RIGHT,PAD_UP,   PAD_UP,   PAD_DOWN,  PAD_RIGHT, PAD_RIGHT, PAD_DOWN,  PAD_RIGHT, 0},0,0},	//CHEAT_OPEN_ALL_EXTRAS
	{{PAD_LEFT, PAD_LEFT, PAD_UP,   PAD_LEFT,  PAD_DOWN,  PAD_RIGHT, PAD_RIGHT, PAD_RIGHT, 0},0,0},//CHEAT_INVULNERABILITY
	{{PAD_RIGHT,PAD_LEFT, PAD_UP,   PAD_UP,    PAD_UP,    PAD_RIGHT, PAD_LEFT,  PAD_LEFT,  0},0,0},//CHEAT_SKIP_LEVEL
	{{PAD_RIGHT,PAD_LEFT, PAD_RIGHT,PAD_LEFT,  PAD_UP,    PAD_UP,    PAD_LEFT,  PAD_RIGHT, 0},0,1},//CHEAT_MAD_GARIBS
	{{PAD_UP,PAD_UP,PAD_DOWN,PAD_DOWN,PAD_LEFT,PAD_RIGHT,0},0,0},//CHEAT_EXTRA_LEVELS
};


long currentCheatCombo[12];
int currCheat = -1;


#ifdef E3_DEMO
int pauseTimeout, pause_gamespeed_decrement_counter_thing;
#endif

unsigned long currentPauseSelection = 0;

short titleDir[4] = {1,1,1,1};
#ifdef PC_VERSION
short titleHudX[2][4] = {{1898,1898,0,3818},{1898,1898,128,3818-128}};
short titleHudY[2][4] = {{0,3818,1898,1898},{128,3818-128,1898,1898}};
#elif PSX_VERSION
short titleHudX[2][4] = {{1898,1898,96,3700},{1898,1898,96+128,3700-128}};
short titleHudY[2][4] = {{220,3668,1898,1898},{220+128*2,3668-128*2,1898,1898}};
#endif

char titleHudOn[7][4] = 
{
	0,0,1,1,
	0,1,1,0,
	1,1,1,1,
	0,1,1,1,
	0,0,1,1,
	1,1,1,1,
	0,1,0,1,
};

char *titleHudName[4] = 
{
	"BUT_UP","BUT_DOWN","BUT_LEFT","BUT_RIGH",
};
SPRITEOVERLAY *titleHud[4];
TEXTOVERLAY *titleHudText[4];

int pauseFrameCount;
int pauseFaded = 0;
long pauseFadeTimer;
/*	--------------------------------------------------------------------------------
	Function 	: StartPauseMenu
	Purpose 	: Pause the Start menu or something
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void StartPauseMenu()
{
	int i;

#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
	pauseFrameCount = 0;
	quittingLevel = NO;
	pauseConfirmMode = NO;
	restartingLevel = NO;

	gameState.oldMode = gameState.mode;
	gameState.mode = PAUSE_MODE;
	pauseMode = 1;

	// Fade screen after 300 seconds for Sega reqs.
	pauseFaded = 0;
#ifdef DREAMCAST_VERSION
	pauseFadeTimer = PAUSEFADETIME;
#endif
	PauseAudio( );

//	EnableTextOverlay ( controllerText );
#ifndef DREAMCAST_VERSION
	currentPauseSelection = 0;
	EnableTextOverlay( continueText );
#else
	currentPauseSelection = 1;
#endif
	EnableTextOverlay ( xselectText );

	if(NUM_FROGS == 1)
		sprintf( pauseTitleString, "%s",GAMESTRING(STR_PAUSE_MODE));
	else
		sprintf( pauseTitleString, "%s %s", GAMESTRING(STR_CHAR_NAME_1+player[pauseController].character), GAMESTRING(STR_PAUSE_MODE) );
	EnableTextOverlay ( pauseTitleText );
	continueText->r = continueText->g = continueText->b = 255;
	if((gameState.oldMode == FRONTEND_MODE) || (gameState.multi == MULTIREMOTE))
		quitText->yPos = quitText->yPosTo = restartText->yPos;
	else
	{
		quitText->yPos = quitText->yPosTo = 2180;
		restartText->r = restartText->b = 64;
		restartText->g = 160;
		EnableTextOverlay ( restartText );
	}
	quitText->r = quitText->b = 64;
	quitText->g = 160;
	EnableTextOverlay ( quitText );

//	if( gameState.multi == SINGLEPLAYER )
		DisableHUD( );

	for(i=0; i<numBabies; i++)
		babyIcons[i]->draw = 0;

#ifdef E3_DEMO
	pauseTimeout = ToFixed(20*60);
	pause_gamespeed_decrement_counter_thing = gameSpeed;
#endif
}


int CheatAllowed(int cheat)
{
	switch(cheat)
	{
		case CHEAT_OPEN_ALL_LEVELS:
			return TRUE;

		case CHEAT_INFINITE_LIVES:
			return TRUE;

		case CHEAT_OPEN_ALL_CHARS:
			return TRUE;

		case CHEAT_OPEN_ALL_EXTRAS:
			return TRUE;

		case CHEAT_INVULNERABILITY:
			return (NUM_FROGS == 1);

		case CHEAT_SKIP_LEVEL:
			return ((NUM_FROGS == 1) && (gameState.mode != FRONTEND_MODE));
		
		case CHEAT_MAD_GARIBS:
			return TRUE;

		case CHEAT_EXTRA_LEVELS:
#ifdef PC_VERSION
			return TRUE;
#else
			return FALSE;
#endif
	
		default:
			return TRUE;
	}
}

void ComboCheat(int cheat)
{
	extern NUM_ARCADE_WORLDS;
	int i,j;
	GARIB *garib;

	if(!CheatAllowed(cheat))
		return;

	PlaySample(genSfx[GEN_DEATHDROWN],NULL,0,SAMPLE_VOLUME,-1);
	if(cheatCombos[cheat].toggle > 0)
		cheatCombos[cheat].state = 1 - cheatCombos[cheat].state;
	else
		cheatCombos[cheat].state = 1;
	
	cheatActivated = cheat;
	GTInit(&cheatTimer,CHEAT_TEXT_TIME);

	if(cheatCombos[cheat].state)
		sprintf(cheatStr,"%s %s",GAMESTRING(STR_CHEAT_1 + cheat),GAMESTRING(STR_ON));
	else
		sprintf(cheatStr,"%s %s",GAMESTRING(STR_CHEAT_1 + cheat),GAMESTRING(STR_OFF));

	switch (cheat)
	{
		case CHEAT_OPEN_ALL_LEVELS:
			for(i = 0;i < NUM_WORLDS;i++)
			{
				worldVisualData[i].worldOpen = WORLD_OPEN;
				for(j = 0;j < worldVisualData[i].numLevels;j++)
					worldVisualData[i].levelVisualData[j].levelOpen = LEVEL_OPEN;
			}
			options.maxPageAllowed = BOOK_NUM_PAGES - 1;
			break;

		case CHEAT_INFINITE_LIVES:
			break;

		case CHEAT_OPEN_ALL_CHARS:
			for(i = 0;i < FROG_NUMFROGS;i++)
				frogPool[i].active = 1;
			break;

		case CHEAT_OPEN_ALL_EXTRAS:
			CheckForExtras();
			break;

		case CHEAT_INVULNERABILITY:
			GTInit(&player[0].safe,20);
			break;

		case CHEAT_SKIP_LEVEL:
			gameState.mode = LEVELCOMPLETE_MODE;
				
			babiesSaved = numBabies;
			break;

		case CHEAT_MAD_GARIBS:
			if(cheatCombos[cheat].state)
			{
				for(garib = garibList.head.next; garib != &garibList.head; garib = garib->next)
				{
					if(garib->type == SILVERCOIN_GARIB)
						garib->sprite->texture = FindTexture("RGARIB01");
				}
				if(arcadeHud.coinsOver)
					arcadeHud.coinsOver->tex = FindTexture("RGARIB01");
				if(arcadeHud.coinZoom)
					arcadeHud.coinZoom->tex = FindTexture("RGARIB01");
			}
			else
			{
				for(garib = garibList.head.next; garib != &garibList.head; garib = garib->next)
				{
					if(garib->type == SILVERCOIN_GARIB)
						garib->sprite->texture = FindTexture("SCOIN0001");
				}
				if(arcadeHud.coinsOver)
					arcadeHud.coinsOver->tex = FindTexture("SCOIN0001");
				if(arcadeHud.coinZoom)
					arcadeHud.coinZoom->tex = FindTexture("SCOIN0001");
			}
			break;

		case CHEAT_EXTRA_LEVELS:
			NUM_ARCADE_WORLDS = 10;
			break;

	}
}

/*	--------------------------------------------------------------------------------
	Function 	: RunPauseMenu
	Purpose 	: In game screen that appears when start is pressed.
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
TEXTOVERLAY *yesText = NULL;
TEXTOVERLAY *noText = NULL;
void RunPauseMenu()
{
	int exitPause = FALSE;
	int i;


	pauseFrameCount += max((pauseGameSpeed>>12),1);
	if((quittingLevel) && (pauseConfirmMode == 0))
	{
#ifdef DREAMCAST_VERSION
		pauseFadeTimer = PAUSEFADETIME;
		// Bring screen back if faded
		if( pauseFaded )
		{
			fadingOut = 0;
			fadeProc = NULL;
			keepFade = 1;
			pauseFaded = 0;
//			ScreenFade( 255, 255, 0 );
		}
#endif
		if(fadingOut == 0)
		{
#ifdef PC_VERSION
			checkMenuKeys = 0;
#endif
			if(quittingLevel == 1)
			{
				if( UndoChangeModel( frog[0]->actor ) )
					player[0].idleEnable = 1;

				if( gameState.multi != SINGLEPLAYER )
					ResetMultiplayer( );

				if( player[0].worldNum == WORLDID_FRONTEND && player[0].levelNum == LEVELID_FRONTEND1 )
					gameState.mode = FRONTEND_MODE;
				else
					gameState.mode = INGAME_MODE;

				if( gameState.single == STORY_MODE )
					player[0].lives = player[0].oldLives;

				restartingLevel = YES;
				InitLevel(player[0].worldNum,player[0].levelNum);

				frameCount = 0;
			}
			else if(quittingLevel == 2)
			{
#ifdef PC_DEMO
				gameState.mode = TEASERSCREEN_MODE;
				return;
#endif

 				if((gameState.oldMode == FRONTEND_MODE) || (gameState.multi == MULTIREMOTE))
 				{
					PostQuitMessage(0);
 				}
 				else
 				{
					if( UndoChangeModel( frog[0]->actor ) )
						player[0].idleEnable = 1;

					player[0].character = FROG_FROGGER;
					NUM_FROGS=1;

					if( gameState.multi != SINGLEPLAYER )
						FreeMultiplayer( );

					if( gameState.multi != MULTIREMOTE )
						gameState.multi = SINGLEPLAYER;
	#ifdef E3_DEMO
					StartE3LevelSelect();
	#else
					if( gameState.single == STORY_MODE )
						player[0].lives = player[0].oldLives;

					gameState.mode = FRONTEND_MODE;
					player[0].character = FROG_FROGGER;
 					InitLevel(WORLDID_FRONTEND,LEVELID_FRONTEND1);
	#endif
 				}

				frameCount = 0;
			}
		}
		return;
	}






#ifdef E3_DEMO
	if (padData.debounce[pauseController])
	{
		pauseTimeout = ToFixed(20*60);
	}
	else
	{
		pauseTimeout -= pause_gamespeed_decrement_counter_thing;
		if (pauseTimeout < 0)
		{
			StartE3LevelSelect();
			return;
		}
	}
#endif
#ifdef DREAMCAST_VERSION
	if( padData.debounce[pauseController] )
	{
		pauseFadeTimer = PAUSEFADETIME;
		// Bring screen back if faded
		if( pauseFaded )
		{
			keepFade = 1;
			pauseFaded = 0;
			fadingOut = 0;
			fadeProc = NULL;
//			ScreenFade( 255, 255, 0 );
		}
	}
#endif
	if (padData.debounce[pauseController]&PAD_SQUARE)
	{
		currCheat = 0;		
		for(i = 0;i < 12;i++)
			currentCheatCombo[i] = 0;
	}

	if (padData.digital[pauseController]&PAD_SQUARE)
	{
		if (padData.debounce[pauseController] & ~PAD_SQUARE)
		{
			currentCheatCombo[currCheat++] = padData.debounce[pauseController];
			if (currCheat == 12) currCheat = 0;
		}
	}
	else if (currCheat>0)	// if we've stored a cheat, check it
	{
		int cheat, i;
		for (cheat=0; cheat<NUMCHEATCOMBOS; cheat++)
		{
			for (i = 0; cheatCombos[cheat].keyString[i] && currentCheatCombo[i]==cheatCombos[cheat].keyString[i]; i++);

			if((cheatCombos[cheat].keyString[i] == 0) && ((cheatCombos[cheat].state == 0) || (cheatCombos[cheat].toggle >= 0)))
			{
				ComboCheat(cheat);
				currCheat = 0;
				exitPause = 1;
				currentPauseSelection = 0;
				break;
//				return;
			}
		}
		currCheat = 0;
	}

	if(!exitPause)
	{
		if(padData.debounce[pauseController]&PAD_UP)
		{
			if (currentPauseSelection ==
#ifdef DREAMCAST_VERSION
				(pauseConfirmMode?0:1)
#else
				0
#endif
				)
			{
				if((gameState.oldMode == FRONTEND_MODE) || (pauseConfirmMode) || (gameState.multi == MULTIREMOTE))
					currentPauseSelection = 1;
				else
					currentPauseSelection = 2;
			}
			else
				currentPauseSelection--;
		}
			

		if(padData.debounce[pauseController]&PAD_DOWN)
		{
			if((((gameState.oldMode == FRONTEND_MODE) || (pauseConfirmMode) || (gameState.multi == MULTIREMOTE)) && (currentPauseSelection == 1)) || (currentPauseSelection == 2))
				currentPauseSelection =
#ifdef DREAMCAST_VERSION
				(pauseConfirmMode?0:1)
#else
				0
#endif
				;
			else
				currentPauseSelection++;
		}
	}

	// If no activity for 300 secs, go to "screensaver" mode
#ifdef DREAMCAST_VERSION
	pauseFadeTimer -= pauseGameSpeed;
	if( pauseFadeTimer <= 0 )
	{
		// Fade to 75%
		ScreenFade( 63, 63, 0 );
		keepFade = 1;
		pauseFaded = 1;
	}
#endif

	if((pauseConfirmMode) && (padData.debounce[pauseController] & PAD_TRIANGLE))
	{
		currentPauseSelection = quittingLevel;
		pauseConfirmMode = 0;
		SubTextOverlay(yesText);
		SubTextOverlay(noText);
		quittingLevel = 0;
		if((gameState.oldMode != FRONTEND_MODE) && (gameState.multi != MULTIREMOTE))
			restartText->draw = 1;
		quitText->draw = 1;
#ifndef DREAMCAST_VERSION
		continueText->draw = 1;
#endif
	}
	if( (padData.debounce[pauseController]&PAD_CROSS) || (padData.debounce[pauseController]&PAD_START) || (exitPause))
	{
//		DisableTextOverlay ( controllerText );
//		DisableTextOverlay ( continueText );
//		DisableTextOverlay ( restartText );
//		DisableTextOverlay ( quitText );

		pauseMode = 0;

		if(padData.debounce[pauseController]&PAD_START)
		{
			currentPauseSelection = 0;
			if(pauseConfirmMode)
			{
				pauseConfirmMode = 0;
				SubTextOverlay(yesText);
				SubTextOverlay(noText);
			}
			quittingLevel = 0;
		}

		switch(currentPauseSelection)
		{
			case 0:   // continue game
			{
				DisableTextOverlay ( continueText );
				DisableTextOverlay ( xselectText );
				DisableTextOverlay ( pauseTitleText );
				DisableTextOverlay ( restartText );
				DisableTextOverlay ( quitText );
				if(pauseConfirmMode)
				{
					pauseConfirmMode = 0;
					ScreenFade(255,0,30);
					SubTextOverlay(yesText);
					SubTextOverlay(noText);
				}
				else
				{
#ifdef PC_VERSION
					checkMenuKeys = 0;
#endif
					if( player[0].worldNum == WORLDID_FRONTEND && player[0].levelNum == LEVELID_FRONTEND1 )
					{
						gameState.mode = FRONTEND_MODE;
					}
					else
					{
						if((player[0].worldNum == WORLDID_FRONTEND) && (player[0].levelNum == LEVELID_FRONTEND4))
						{
							gameState.mode = TRAINING_MODE;
							for(i = 0;i < numBabies;i++)
							{
								arcadeHud.babiesBack[i]->draw = 1;
								babyIcons[i]->draw = 1;
							}
						}
						else
						{
							gameState.mode = INGAME_MODE;
							gameSpeed = pauseGameSpeed;
							EnableHUD( );
						}
					}

#ifdef DREAMCAST_VERSION
					GTInit( &screenSaveTimer, PAUSEFADETIME );
					keepFade = 1;
					fadingOut = 0;
					fadeProc = NULL;
#endif
					UnPauseAudio( );
				}
				return;
			}

/*			case 1:
				StartControllerView();
				return;
*/
			case 1:
				if(pauseConfirmMode)
				{
					pauseConfirmMode = 0;
					SubTextOverlay(yesText);
					SubTextOverlay(noText);
					if((gameState.oldMode == FRONTEND_MODE) || (gameState.multi == MULTIREMOTE))
						currentPauseSelection = 1;
					else
					{
						restartText->draw = 1;
						currentPauseSelection = quittingLevel;
					}

#ifndef DREAMCAST_VERSION
					continueText->draw = 1;
#endif
					quitText->draw = 1;
					quittingLevel = 0;
					break;
				}
				else
				{
					if((gameState.oldMode != FRONTEND_MODE) && (gameState.multi != MULTIREMOTE))
					{
						quittingLevel = 1;
						pauseConfirmMode = 1;
						restartText->r = restartText->g = restartText->b = 255;
						yesText = CreateAndAddTextOverlay(2048,restartText->yPos + 400,GAMESTRING(STR_YES),YES,255,font,TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
						yesText->r = 64;
						yesText->g = 160;
						yesText->b = 64;
						noText = CreateAndAddTextOverlay(2048,restartText->yPos + 700,GAMESTRING(STR_NO),YES,255,font,TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
						noText->r = 64;
						noText->g = 160;
						noText->b = 64;
						keepFade = 1;
						continueText->draw = 0;
						quitText->draw = 0;
						currentPauseSelection = 1;
						return;
					}
				}
			//deliberate drop-through!!!!!!!		

			case 2:   // quit game
				if(pauseConfirmMode == 0)
				{
					pauseConfirmMode = 1;
					currentPauseSelection = 1;
					quitText->r = quitText->g = quitText->b = 255;
					yesText = CreateAndAddTextOverlay(2048,quitText->yPos + 400,GAMESTRING(STR_YES),YES,255,font,TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
					noText = CreateAndAddTextOverlay(2048,quitText->yPos + 700,GAMESTRING(STR_NO),YES,255,font,TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
					continueText->draw = 0;
					restartText->draw = 0;
				}
				quittingLevel = 2;
				keepFade = 1;
				break;
		}

	}

	if(pauseConfirmMode)
	{
		switch(currentPauseSelection)
		{
			case 0:
				yesText->r = 127+((rsin(pauseFrameCount*4000)+4096)*64)/4096;
				yesText->g = 127+((rcos(pauseFrameCount*4000)+4096)*64)/4096;
				yesText->b = 10;
//				yesText->r = yesText->g = yesText->b = 255;
				noText->r = 64;
				noText->g = 160;
				noText->b = 64;
				break;

			case 1:
				noText->r = 127+((rsin(pauseFrameCount*4000)+4096)*64)/4096;
				noText->g = 127+((rcos(pauseFrameCount*4000)+4096)*64)/4096;
				noText->b = 10;
//				noText->r = noText->g = noText->b = 255;
				yesText->r = 64;
				yesText->g = 160;
				yesText->b = 64;
				break;
		}
	}
	else
	{
		switch(currentPauseSelection)
		{
			case 0:
				continueText->r = 127+((rsin(pauseFrameCount*4000)+4096)*64)/4096;
				continueText->g = 127+((rcos(pauseFrameCount*4000)+4096)*64)/4096;
				continueText->b = 10;
//				continueText->r = continueText->g = continueText->b = 255;
				restartText->r = restartText->b = quitText->r = quitText->b /*= controllerText->r = controllerText->b*/ = 64;
				restartText->g = quitText->g /*= controllerText->g*/ = 160;
				break;
	/*		
			case 1:
				//controllerText->r = controllerText->g = controllerText->b = 255;
				continueText->r = continueText->b = restartText->r = restartText->b = quitText->r = quitText->b = 100;
				restartText->g = quitText->g = continueText->g = 200;
				break;
	*/	
			case 1:
				if((gameState.oldMode != FRONTEND_MODE) && (gameState.multi != MULTIREMOTE))
				{
					restartText->r = 127+((rsin(pauseFrameCount*4000)+4096)*64)/4096;
					restartText->g = 127+((rcos(pauseFrameCount*4000)+4096)*64)/4096;
					restartText->b = 10;
//					restartText->r = restartText->g = restartText->b = 255;
					continueText->r = continueText->b = quitText->r = quitText->b /*= controllerText->r = controllerText->b*/ = 64;
					continueText->g = quitText->g /*= controllerText->g*/ = 160;
					break;
				}
			//deliberate drop-through!!!!!!!		
			case 2:
				quitText->r = 127+((rsin(pauseFrameCount*4000)+4096)*64)/4096;
				quitText->g = 127+((rcos(pauseFrameCount*4000)+4096)*64)/4096;
				quitText->b = 10;
//				quitText->r = quitText->g = quitText->b = 255;
				continueText->r = continueText->b = restartText->r = restartText->b /*= controllerText->r = controllerText->b*/ = 64;
				continueText->g = restartText->g /*= controllerText->g*/ = 160;
				break;

		}
	}
}






#define MAX_SPARKLES 15
long tValue;

SPRITEOVERLAY *sparkles[MAX_SPARKLES];

TIMER frontendTimer;	// when we run out of time, play a demo or a video or something

char worldStr[64] = "";
//char pcStr[64] = "00 %%";
char levelStr[MAX_LEVELSTRING][64] = {"","","","","","","","","",""};
char levelParStr[MAX_LEVELSTRING][16] = {"","","","","","","","","",""};
char levelSetByStr[MAX_LEVELSTRING][8] = {"","","","","","","","","",""};
char levelCoinStr[MAX_LEVELSTRING][3] = {"","","","","","","","","",""};
unsigned long cWorld;
fixed fadeSpeed = ToFixed(10);

long globalSoundVol = 100;
long globalMusicVol = 70;

long reachedPoint = 0;

TextureType *arcadeScreenTex = NULL;

int arcadeTexNum = 0;
extern long creditsRunning;

int bounce = 0;
int bounceDir = 1;
int bounceSpeed = 5;
char goingToDemo = NO;

#ifdef PSX_VERSION
extern SCENICOBJ *lightBeam;
#endif

void RunFrontendGameLoop (void)
{
	unsigned long i,j;
	long hudNum,maxHud;
	GAMETILE *cur,*testTile;
	ENEMY *tempEnemy;
	char texname[16];
	
	// setup for frogger point of interest
	pOIDistance = 20000<<12;
	pointOfInterest = NULL;



#ifdef PSX_VERSION
	if (	( &firstTile[ TILENUM_CHOICE ]	== currTile[0] ) &&
				( &firstTile[ TILENUM_BOOK ]		== destTile[0] ) )
	{
		FMA_MESH_HEADER **mesh;

		if ( lightBeam )
		{
			mesh = ADD2POINTER(lightBeam->fmaObj,sizeof(FMA_WORLD));

			SetActorGouraudValuesMinus	(	*mesh, 5, 5, 5 );
		}
		// ENDIF
	}
	// ENDIF

	if (	( &firstTile[ TILENUM_CHOICE ]	== destTile[0] ) &&
				( &firstTile[ TILENUM_BOOK ]	== currTile[0] ) )
	{
		FMA_MESH_HEADER **mesh;

		if ( lightBeam )
		{
			mesh = ADD2POINTER(lightBeam->fmaObj,sizeof(FMA_WORLD));

			SetActorGouraudValuesPlus ( *mesh, 5, 5, 5, lightBeamGouraudValues );

			//SetObjectGouraudValues ( *mesh, lightBeamGouraudValues );
		}
		// ENDIF
	}
	// ENDIF
#endif

#ifdef PC_VERSION
	checkMenuKeys = 0;
#endif
	if(goingToDemo)
	{
		if(fadingOut == 0)
		{
			oldDiffMode = gameState.difficulty;

			gameState.difficulty = DIFFICULTY_NORMAL;

			goingToDemo = NO;
			InitDemoMode();
		}
		return;
	}


	if(bounceDir > 0)
	{
		bounce += FMul(bounceSpeed,gameSpeed);
		if(bounce >= 128)
		{
			bounceDir = -1;
			bounce = 128;
		}
	}
	else
	{
		bounce -= FMul(bounceSpeed,gameSpeed);
		if(bounce <= 0)
		{
			bounceDir = 1;
			bounce = 0;
		}
	}

	camSpeed3 = 4096*20;
	if (frameCount == 1)
	{
		SetMusicVolume();

		fadingLogos = NO;
#ifdef PSX_VERSION
		frogLogo = CreateAndAddSpriteOverlay(1168,2700,"FROGLOGO",4096,1,255,0);
#elif PC_VERSION
		frogLogo = CreateAndAddSpriteOverlay(1024,2800,"FROGLOGO",2048,2048,255,0);
#endif

		fogStore = fog.max;
		
		CheckForExtras();
		InitOptionsMenu();
		ScreenFade(0,255,30);
		keepFade = 0;

#ifdef PSX_VERSION
#if GOLDCD==0
	#ifndef FINAL_MASTER
			if(cheatCombos[CHEAT_OPEN_ALL_LEVELS].state == 0)
				ComboCheat(CHEAT_OPEN_ALL_LEVELS);
	#endif	
#endif
#else
#ifndef FINAL_MASTER
		if(cheatCombos[CHEAT_OPEN_ALL_LEVELS].state == 0)
			ComboCheat(CHEAT_OPEN_ALL_LEVELS);
#endif	
#endif


		for(i = 0;i < 4;i++)
		{
			titleHud[i] = CreateAndAddSpriteOverlay(titleHudX[0][i],titleHudY[0][i],titleHudName[i],300,300,0,0);
			titleHudText[i] = CreateAndAddTextOverlay(0,0,NULL,(i < 2),255,fontSmall,TEXTOVERLAY_SHADOW);
			titleHudText[i]->b = 0;
			titleHudText[i]->draw = 0;
//			titleHud[i]->xPosTo = titleHudX[1][i];
//			titleHud[i]->yPosTo = titleHudY[1][i];
//			if(i < 2)
//				titleHud[i]->speed = 10*4096;
//			else
//				titleHud[i]->speed = 5*4096;
		}


		for(i = NUM_STORY_LEVELS - 1;i >= 0;i--)
		{
			if((worldVisualData[storySequence[i].worldNum].worldOpen) && (worldVisualData[storySequence[i].worldNum].levelVisualData[storySequence[i].levelNum].levelOpen))
			{
				gameState.storySequenceLevel = i;
				break;
			}
		}

		for(i = 0;i < BOOK_NUM_PAGES;i++)
		{
			if(gameState.storySequenceLevel >= pageToStoryLevel[i])
				options.maxPageAllowed = options.pageNum = i;
		}

		options.page = NULL;

		if( (tempEnemy = GetEnemyFromUID(123)) )
			options.book = tempEnemy->nmeActor->actor;
		else
			options.book = NULL;

		if(tempEnemy = GetEnemyFromUID(124))
			options.chestLid = tempEnemy->nmeActor;
		else
			options.chestLid = NULL;

		if(tempEnemy = GetEnemyFromUID(125))
			options.door = tempEnemy->nmeActor;
		else
			options.door = NULL;

		if(options.door)
			actorAnimate(options.door->actor,1,NO,NO,4000,NO);
		if(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].levelCompleted)
		{
/*
			if(doneTraining == 0)
			{
				if(options.door)
					actorAnimate(options.door->actor,1,NO,NO,40,NO);
			}
			else
			{
				if(options.door)
					actorAnimate(options.door->actor,1,NO,NO,4000,NO);
			}
*/
			doneTraining = 1;
			staticFlash = 5;
		}

#ifdef PC_VERSION
		if(rHardware)
		{
			if (options.book)
			{
				options.page = CreateAndAddActor("bookpage.obe",-3200,800,-2800,INIT_ANIMATION);
				if (options.page)
				{
					options.page->actor->position = options.book->position;
					options.page->actor->position.vy += 5;
					options.page->actor->qRot = options.book->qRot;
					options.page->actor->size = options.book->size;
					options.pageDir = 1;
					options.pageDrawCount = 0;
					actorAnimate(options.page->actor, 1, NO, NO, 9999, NO );
					grabToTexture = 1;
				}
			}
		}
#endif

		if(options.page == NULL)
		{
			sprintf(chapterStr[0],GAMESTRING(STR_CHAPTER),options.pageNum + 1);
			for(i = 0;i < 2;i++)
			{
				options.chapterText[i] = CreateAndAddTextOverlay(2150,800,chapterStr[i],NO,0,font,TEXTOVERLAY_SHADOW);
				options.chapterText[i]->draw = 0;
				options.chapterNameText[i][0] = CreateAndAddTextOverlay(2150,2800,GAMESTRING(STR_CHAPTER_1a + options.pageNum*2),NO,0,fontSmall,TEXTOVERLAY_SHADOW);
				options.chapterNameText[i][1] = CreateAndAddTextOverlay(2150,3000,GAMESTRING(STR_CHAPTER_1a + options.pageNum*2 + 1),NO,0,fontSmall,TEXTOVERLAY_SHADOW);
				options.chapterNameText[i][0]->draw = 0;
				options.chapterNameText[i][1]->draw = 0;

				options.chapterPic[i] = CreateAndAddSpriteOverlay(2400,1500,chapterPic[options.pageNum],1024,1024,0,i==1 ? SPRITE_CLIP1 : SPRITE_CLIP2);
				options.chapterPic[i]->draw = 0;
			}
			player[0].hasJumped = 0;
		}

		DisableHUD();
		reachedPoint = 0;

		for (i=0; i<MAX_SPARKLES; i++)
		{
			sparkles[i] = CreateAndAddSpriteOverlay( 256,1535,"FLASH",102,137,(UBYTE)Random(0xff),SPRITE_ADDITIVE);
			sparkles[i]->num = 1;
		}

		options.arcadeText = CreateAndAddTextOverlay(2048,270,GAMESTRING(STR_ARCADEMODE),YES,255,font,0); 
		options.arcadeText->b = 0;
		options.selectText = CreateAndAddTextOverlay(2048,576,GAMESTRING(STR_SELECT_LEVEL),YES,255,fontSmall,0); 
		options.worldText = CreateAndAddTextOverlay(306,1105,worldStr,NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		
		options.parText[0] = CreateAndAddTextOverlay(1950,1105,GAMESTRING(STR_PAR),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		options.parText[1] = CreateAndAddTextOverlay(2750,1105,GAMESTRING(STR_SET_BY),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		options.parText[2] = CreateAndAddTextOverlay(3450,1105,GAMESTRING(STR_COINS),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		
		options.worldText->r = options.parText[0]->r = options.parText[1]->r = options.parText[2]->r = 255;
		options.worldText->g = options.parText[0]->g = options.parText[1]->g = options.parText[2]->g = 200;
		options.worldText->b = options.parText[0]->b = options.parText[1]->b = options.parText[2]->b = 0;

		for (i=0; i<MAX_LEVELSTRING; i++)
		{
			options.levelText[i] = CreateAndAddTextOverlay(306,(short)(1445+i*170),levelStr[i],NO,(char)0,fontSmall,TEXTOVERLAY_SHADOW);
			options.levelText[i]->draw = 0;
			options.levelParText[i] = CreateAndAddTextOverlay(1950,(short)(1445+i*170),levelParStr[i],NO,0,fontSmall,TEXTOVERLAY_SHADOW);
			options.levelParText[i]->draw = 0;
			options.levelSetByText[i] = CreateAndAddTextOverlay(2750,(short)(1445+i*170),levelSetByStr[i],NO,0,fontSmall,TEXTOVERLAY_SHADOW);
			options.levelSetByText[i]->draw = 0;
			options.levelCoinText[i] = CreateAndAddTextOverlay(3600,(short)(1445+i*170),levelCoinStr[i],NO,0,fontSmall,TEXTOVERLAY_SHADOW);
			options.levelCoinText[i]->draw = 0;
			options.levelCoinMedal[i] = CreateAndAddSpriteOverlay(3582,(short)(1477+i*170),"COINMEDAL",160,160,0,0);
			options.levelCoinMedal[i]->draw = 0;
			options.beatenIcon[i] = CreateAndAddSpriteOverlay(150,(short)(1477+i*170)-32,"FLASH",160,160,0,SPRITE_ADDITIVE);
			options.beatenIcon[i]->draw = 0;
		}

		options.worldBak = CreateAndAddSpriteOverlay(150,938,"WBACK",3796,1365,0,0);
		options.worldBak->r = 10;
		options.worldBak->g = 200;
		options.worldBak->b = 10;

		arcadeScreenTex = FindTexture( "TEST5" );

		CopyTexture ( arcadeScreenTex, FindTexture("GARDEN1"), 1 );

		options.titleBak = CreateAndAddSpriteOverlay(0,0,"",4096,800,0,SPRITE_SUBTRACTIVE);

		options.statusBak = CreateAndAddSpriteOverlay(0,3660,"",4096,500,0,SPRITE_SUBTRACTIVE);

		options.arcadeText->draw = 0;
		options.selectText->draw = 0;
		options.worldText->draw = 0;
		options.worldBak->draw = 0;
		options.titleBak->draw = 0;
		options.statusBak->draw = 0;
		options.parText[0]->draw = options.parText[1]->draw = options.parText[2]->draw = 0;

		CheckForDynamicCameraChange(currTile[0],0); // TEMPORARY FIX!!
		lastActFrameCount = 0;

		GTInit(&frontendTimer, 30);	// 30 seconds
	}

	currTileNum = 0;

	cur = firstTile + 1;
	for ( i = 0; i < tileCount; i++, cur++ )
	{
		if(cur == currTile[0])
		{
			currTileNum++;
			break;
		}
		currTileNum++;
	}

	if (padData.debounce[0])
	{
		if(currTileNum == TILENUM_START)
		{
			GTInit(&frontendTimer, 30);
		}
		else
		{
			GTInit(&frontendTimer, LONG_DEMO_WAIT);
		}
	}


	if((frontendTimer.time > 0) && (playDemos) && (!creditsRunning) && (options.mode != OP_GLOBALMENU) && (options.mode != OP_EXTRA))
	{
		if((currTileNum == TILENUM_START) && (frontendTimer.time > 30))
			GTInit(&frontendTimer,30);
		GTUpdate(&frontendTimer, -1);
		if (!frontendTimer.time)
		{
			utilPrintf("Front-end idle timer ran out.. run demo mode!\n");
			// run a demo here

			goingToDemo = YES;
			ScreenFade(255,0,30);
			keepFade = 0;
		}
	}

	if((!doneTraining) || (staticFlash))
	{
		if(staticFlash)
		{
			staticFlash--;
			if(staticFlash == 0)
			{
				if(cWorld == WORLDID_GARDEN)
				{
					if(options.levelNum == 0)
						CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].texCRC), 1 );
					else
						CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].texCRC), 1 );
				}
				else
				{
					CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[cWorld].levelVisualData[options.levelNum].texCRC), 1 );
				}
			}
		}
		if((!doneTraining) || (staticFlash))
		{
			sprintf(texname,"0%dNOIS04",arcadeTexNum);
			arcadeTexNum = (arcadeTexNum + 1) MOD 3;
			CopyTexture(arcadeScreenTex,FindTexture(texname),1);
		}
	}

//	flogo[0]->num = 0;
	
//	if( fadingLogos )
//	{
//		if (atari->xPos < 6400)
//		{
//			atari->xPosTo = 10000;
//			konami->xPosTo = -1000;
//			flogo[0]->yPosTo = 10000;

//#ifdef PSX_VERSION
//			flogo[1]->yPosTo = 10000;
//			flogo[2]->yPosTo = 10000;
//			flogo[3]->yPosTo = 10000;
//#endif
//			blitzLogo->yPosTo = -10000;
//		}
//		else
//		{
//			atari->draw = 0;
//			konami->draw = 0;
//			flogo[0]->draw = 0;

//#ifdef PSX_VERSION
//			flogo[1]->draw = 0;
//			flogo[2]->draw = 0;
//			flogo[3]->draw = 0;
//#endif

//			blitzLogo->draw = 0;
//		}
//	}
//	else
//	{
//		if (player[0].hasJumped == 1)
//			fadingLogos = 1;
//	}
	
	for (i=0; i<MAX_SPARKLES; i++)
	{
		unsigned long spkLev;

		if (sparkles[i]->a > (gameSpeed * 3)/4096)
			sparkles[i]->a-= (gameSpeed * 3)/4096;
		else
		{
			if (options.mode == OP_ARCADE)
				spkLev = options.levelNum;
			else
				spkLev = options.spChar;
			

			if(player[0].canJump)
			{
				sparkles[i]->yPosTo = sparkles[i]->yPos = Random(128) + options.levelText[spkLev]->yPos-64;
				sparkles[i]->a = Random(160);
				if (options.levelText[spkLev]->centred)
					sparkles[i]->xPosTo = sparkles[i]->xPos = (2000-options.levelText[spkLev]->tWidth*3) + Random(options.levelText[spkLev]->tWidth*6);
				else
					sparkles[i]->xPosTo = sparkles[i]->xPos = (options.levelText[spkLev]->xPos) + Random(options.levelText[spkLev]->tWidth*6);
			}
			else
				sparkles[i]->a = 0;
		}
			
		sparkles[i]->speed = 0;
		sparkles[i]->draw = options.levelText[options.levelNum]->draw;
	}

	// Take this out for release
	if( !tileNum )
		tileNum = CreateAndAddTextOverlay(2048,1024,tileString,YES,(char)255,font,0);

	// displays the tile numbers
	
	if((frameCount == 1) && (lastArcade))
	{
		testTile = firstTile + 1;
		for ( i = 0; i < tileCount; i++, testTile++ )
		{
			cur = firstTile + 1;
			currTileNum = 0;

			for ( j = 0; j < tileCount; j++, cur++ )
			{
				if(cur == testTile)
				{
					currTileNum++;
					break;
				}
				currTileNum++;
			}

			
			if(currTileNum == lastArcade)
			{
				currTile[0] = lastTile[0] = testTile;
				break;
			}
		}

		SetVectorSS(&frog[0]->actor->position,&currTile[0]->centre);
		SetVectorFF(&currCamSource,&storeCurrCamSource);
		SetVectorFF(&camSource,&storeCamSource);
		SetVectorFF(&currCamTarget,&storeCurrCamTarget);
		SetVectorFF(&camTarget,&storeCamTarget);
		SetVectorFF(&currCamDist,&storeCurrCamDist);
		SetVectorFF(&camDist,&storeCamDist);
		SetVectorFF(&currCamOffset,&storeCurrCamOffset);
		SetVectorFF(&camOffset,&storeCamOffset);
		SetVectorFF(&camVect,&storeCamVect);
		if(lastArcade == TILENUM_ARCADE)
		{
			camFacing[0] = 1;
		}
		else
		{
			for(i = 0;i < options.currentPlayer;i++)
			{
				options.multiFace[options.playerChar[i]]->draw = 1;
				options.multiFace[options.playerChar[i]]->r = options.multiFace[options.playerChar[i]]->g = options.multiFace[options.playerChar[i]]->b = options.multiFace[options.playerChar[i]]->a = 255;
				options.multiFace[options.playerChar[i]]->xPos = options.multiFace[options.playerChar[i]]->xPosTo = backCharsX[i];
				options.multiFace[options.playerChar[i]]->yPos = options.multiFace[options.playerChar[i]]->yPosTo = backCharsY[i];
			}
			options.mode = OP_LEVELSEL;
			reachedPoint = 1;
			storeCamSource.vx = -3884265;
			storeCamSource.vy = 3653915;
			storeCamSource.vz = 2103810;
			storeCamTarget.vx = 6143995;
			storeCamTarget.vy = 409600;
			storeCamTarget.vz = -3686400;
			storeCurrCamOffset.vx = -10028260;
			storeCurrCamOffset.vy = 3244315;
			storeCurrCamOffset.vz = 5790205;
			player[0].hasJumped = 0;
		}

		lastArcade = 0;
		fadingLogos = 1;
		frogLogo->draw = 0;
	}

	if((fadingLogos == NO) && (currTileNum != TILENUM_START))
		fadingLogos = 1;

	if(fadingLogos)
	{
	  	DEC_ALPHA(frogLogo);
	}

	maxHud = 4;


	if(player[0].canJump)
	{
		switch(currTileNum)
		{
			case TILENUM_START:
				hudNum = 0;
				titleHudText[0]->draw = 0;
				titleHudText[1]->draw = 0;
				titleHudText[2]->draw = 1;
				titleHudText[3]->draw = 1;
				titleHudText[2]->text = GAMESTRING(STR_OPTIONS);
				titleHudText[3]->text = GAMESTRING(STR_START_GAME);
				break;

			case TILENUM_OPTIONS:
				if(options.mode == OP_GLOBALMENU)
				{
					titleHudText[0]->draw = 0;
					titleHudText[1]->draw = 1;
					titleHudText[1]->text = GAMESTRING(STR_START_GAME);
					if(options.selection == 0)
					{
						hudNum = 1;
						titleHudText[2]->draw = 1;
						titleHudText[3]->draw = 0;
						titleHudText[2]->text = GAMESTRING(STR_OPTIONS_2);
					}
					else
					{
						hudNum = 6;
						titleHudText[2]->draw = 0;
						titleHudText[3]->draw = 1;
						titleHudText[3]->text = GAMESTRING(STR_OPTIONS_1);
					}
					titleHud[2]->yPos = titleHud[2]->yPosTo = titleHud[3]->yPos = titleHud[3]->yPosTo = titleHudY[0][2];
					titleHud[0]->xPos = titleHud[0]->xPosTo = titleHud[1]->xPos = titleHud[1]->xPosTo = titleHudX[0][0];
				}
				else if(options.mode == OP_SOUND)
				{
					titleHudText[0]->draw = 0;
					titleHudText[1]->draw = 0;
					titleHudText[2]->draw = 0;
					titleHudText[3]->draw = 0;
					hudNum = 5;
					titleHud[0]->a = titleHud[1]->a = titleHud[2]->a = titleHud[3]->a = 255;
				}
				else
					hudNum = -1;
				break;

			case TILENUM_CHOICE:
				//close off multiplayer and arcade machine if training not completed
				titleHudText[0]->draw = 1;
				titleHudText[1]->draw = 1;
				titleHudText[2]->draw = 1;
				titleHudText[3]->draw = doneTraining;

				titleHudText[0]->text = GAMESTRING(STR_STORYMODE);
				titleHudText[1]->text = GAMESTRING(STR_OPTIONS);
				titleHudText[2]->text = GAMESTRING(STR_MULTIPLAYER);
				titleHudText[3]->text = GAMESTRING(STR_ARCADEMODE);
				titleHud[2]->yPos = titleHud[2]->yPosTo = titleHud[3]->yPos = titleHud[3]->yPosTo = titleHudY[0][2];
#ifdef FINAL_MASTER
				if(doneTraining == 0)
				{
					currTile[0]->tilePtrs[2] = NULL;
					maxHud = 3;
				}
				GTInit( &modeTimer, 5 );
#endif
				hudNum = 2;
				break;

			case TILENUM_BOOK:
				hudNum = 3;
				titleHudText[0]->draw = 0;
				titleHudText[1]->draw = 0;
				titleHudText[2]->draw = 0;
				titleHudText[3]->draw = 0;
				break;


			case TILENUM_MULTI:
				titleHudText[0]->draw = 0;
				titleHudText[1]->draw = 0;
				titleHudText[2]->draw = 0;
				titleHudText[3]->draw = 0;
				if((options.mode == OP_MULTIPLAYERNUMBER) && (maxPlayers <= 2))
					hudNum = -1;
				else
					hudNum = 4;
				break;

			case TILENUM_ARCADE:
				titleHudText[0]->draw = 0;
				titleHudText[1]->draw = 0;
				titleHudText[2]->draw = 0;
				titleHudText[3]->draw = 0;
				if(options.mode == OP_ARCADE)
					hudNum = 4;
				else
					hudNum = -1;
				break;

			default:
				titleHudText[0]->draw = 0;
				titleHudText[1]->draw = 0;
				titleHudText[2]->draw = 0;
				titleHudText[3]->draw = 0;
				hudNum = -1;
				break;
		}
	}
	else
		hudNum = -1;


	titleHudText[0]->xPos = 2048;
	titleHudText[1]->xPos = 2048;

	titleHudText[2]->xPos = titleHudX[0][2] + 164;
	titleHudText[2]->yPos = titleHud[2]->yPos + 300;
	titleHudText[3]->yPos = titleHud[3]->yPos + 300;
#ifdef PSX_VERSION
	titleHudText[0]->yPos = titleHudY[0][0] + 520;
	titleHudText[1]->yPos = titleHudY[0][1] - 520;
	titleHudText[3]->xPos = titleHudX[0][3] + 100 - fontExtentWScaled(fontSmall,titleHudText[3]->text,4096)*8;
#elif PC_VERSION
	titleHudText[0]->yPos = titleHudY[0][0] + 400;
	titleHudText[1]->yPos = titleHudY[0][1] - 400;
	titleHudText[3]->xPos = titleHudX[0][3] + 100 - CalcStringWidth(titleHudText[3]->text,(MDX_FONT *)fontSmall,1)*
		6.4*((!rHardware&& rXRes < 640)+1);
#endif

	if(hudNum == 5)
	{
		titleHud[0]->yPos = 3000;
		titleHud[1]->yPos = 3000;
		titleHud[0]->xPos = 1000;
		titleHud[1]->xPos = 1000 + 300;
		titleHud[2]->yPos = 3300;
		titleHud[3]->yPos = 3300;
		titleHud[2]->xPos = 1000;
		titleHud[3]->xPos = 1000 + 300;
	}
	else
	{
#ifdef PSX_VERSION
		titleHud[0]->yPos = bounce*2 + 96*2;
		titleHud[1]->yPos = 4096 - titleHud[0]->height - bounce*2 - 96*2;

		titleHud[2]->xPos = bounce + 96;
		titleHud[3]->xPos = 4096 - titleHud[3]->width - bounce - 96*2;
#elif PC_VERSION
		titleHud[0]->yPos = bounce;
		titleHud[1]->yPos = 4096 - titleHud[0]->height - bounce;

		titleHud[2]->xPos = bounce;
		titleHud[3]->xPos = 4096 - titleHud[3]->width - bounce;
#endif
	}

	if(hudNum == -1)
	{
		for(i = 0;i < 4;i++)
		{
			DEC_ALPHA(titleHud[i]);
			DEC_ALPHA(titleHudText[i]);
		}
	}
	else
	{
		for(i = 0;i < 4;i++)
		{
			if(titleHudText[i]->draw)
			{
				INC_ALPHA(titleHudText[i],255);
			}
			else
			{
				DEC_ALPHA(titleHudText[i]);
			}
			if(titleHudOn[hudNum][i] == 0)
			{
				DEC_ALPHA(titleHud[i]);
			}
			else if(i < maxHud)
			{
				if(currTileNum == TILENUM_BOOK)
				{
					if(i == 2)
					{
						if(options.pageNum == 0)
						{
							DEC_ALPHA(titleHud[i]);
						}
						else
						{
							INC_ALPHA(titleHud[i],255);
						}
					}
					else if(i == 3)
					{
						if(options.pageNum == options.maxPageAllowed)
						{
							DEC_ALPHA(titleHud[i]);
						}
						else
						{
							INC_ALPHA(titleHud[i],255);
						}
					}
					else
					{
						INC_ALPHA(titleHud[i],255);
					}
				}
				else
					INC_ALPHA(titleHud[i],255);
				titleHud[i]->tex = FindTexture(titleHudName[i]);
			}
		}
	}


	if((currTileNum != TILENUM_BOOK) && (currTileNum != TILENUM_MULTI) && (currTileNum != TILENUM_OPTIONS) && (currTileNum != TILENUM_ARCADE))
	{
		if(frameCount > 15)
			GameProcessController(0);                                      
	}

	if(((currTileNum != TILENUM_BOOK) || (player[0].canJump == 0)) && (options.page == NULL))
	{
		DEC_ALPHA(options.chapterNameText[0][0]);
		DEC_ALPHA(options.chapterNameText[0][1]);
		DEC_ALPHA(options.chapterNameText[1][0]);
		DEC_ALPHA(options.chapterNameText[1][1]);
		DEC_ALPHA(options.chapterText[0]);
		DEC_ALPHA(options.chapterText[1]);
		DEC_ALPHA(options.chapterPic[0]);
		DEC_ALPHA(options.chapterPic[1]);
	}

	if((currTileNum != TILENUM_ARCADE) || (player[0].canJump == 0))
	{
		DEC_ALPHA(options.arcadeText);
		DEC_ALPHA(options.parText[0]);
		DEC_ALPHA(options.parText[1]);
		DEC_ALPHA(options.parText[2]);
		
		if(((currTileNum != TILENUM_BOOK) || (player[0].canJump == 0)) && ((currTileNum != TILENUM_OPTIONS) || (creditsRunning) || (player[0].canJump == 0)))
			DEC_ALPHA(options.selectText);
		DEC_ALPHA(options.worldText);
		DEC_ALPHA(options.worldBak);

		for (i=0; i<MAX_LEVELSTRING; i++)
		{
			DEC_ALPHA(options.levelText[i]);
			DEC_ALPHA(options.levelParText[i]);
			DEC_ALPHA(options.levelSetByText[i]);
			DEC_ALPHA(options.levelCoinText[i]);
			DEC_ALPHA(options.levelCoinMedal[i]);
			DEC_ALPHA(options.beatenIcon[i]);
		}
	}


	if(options.mode == -1)
	{
		DEC_ALPHA(options.title);
		DEC_ALPHA(options.subTitle);
		DEC_ALPHA(options.titleBak);
		DEC_ALPHA(options.statusBak);
	}

	//options tile by waterfall
	if ((currTileNum == TILENUM_OPTIONS) && (player[0].hasJumped == 0))
	{
		frogFacing[0] = 1;
		if(!reachedPoint)
			reachedPoint = 1;
		if(options.mode == -1)
		{
			options.mode = OP_GLOBALMENU;
			GTInit(&frontendTimer,LONG_DEMO_WAIT);
		}
		RunOptionsMenu();
		if(gameState.mode == ARTVIEWER_MODE)
			return;
	}
	

	

	//multiplayer tile
	if ((currTileNum == TILENUM_MULTI) && (player[0].hasJumped == 0))
	{
		if (!reachedPoint)
		{
			SetVectorFF(&storeCamSource,&currCamSource);
			SetVectorFF(&storeCamTarget,&currCamTarget);
			SetVectorFF(&storeCurrCamOffset,&currCamOffset);

			currCamSource.vx = -9591334;
			currCamSource.vy = 2500000;
			currCamSource.vz = 20425392;
			
			currCamTarget.vx = -9593916;
			currCamTarget.vy = 0;
			currCamTarget.vz = 20430628;

			SetVectorFF(&camTarget,&currCamTarget);
			SetVectorFF(&camSource,&currCamSource);

			reachedPoint = 1;
			fogStore = fog.max;
			fog.max = 2500;
		}

		if (reachedPoint)
		{					
			if (options.mode == -1)
			{
				options.mode = OP_MULTIPLAYERNUMBER;
				GTInit(&frontendTimer,LONG_DEMO_WAIT);
			}
			RunOptionsMenu();
			if (gameState.mode == INGAME_MODE)
			{
				reachedPoint = 0;
				return;
			}
		}
	}

	if((currTileNum == TILENUM_ARCADE) && (player[0].hasJumped == 0))
	{
		if(options.mode == -1)
		{
			options.mode = OP_ARCADE;
			GTInit(&frontendTimer,LONG_DEMO_WAIT);
			ScreenFade(255,210,30);
			keepFade = YES;
			fadeText = NO;
		}
		RunOptionsMenu();
		if(gameState.mode != FRONTEND_MODE)
			return;
	}

	if((currTileNum == TILENUM_BOOK) && (player[0].hasJumped == 0))
	{
		if(options.mode == -1)
		{
			options.mode = OP_BOOK;
			GTInit(&frontendTimer,LONG_DEMO_WAIT);
		}
		RunOptionsMenu();
		if(gameState.mode != FRONTEND_MODE)
			return;
	}

	if(options.page)
	{
		if((options.pageDir == 0) && (options.pageDrawCount))
		{
			if(--options.pageDrawCount == 0)
				options.page->draw = 0;
		}
	}
	else
	{
	}


//	Orientate(&frog[0]->actor->qRot, &currTile[0]->dirVector[frogFacing[0]], &currTile[0]->normal);
#ifdef PSX_VERSION
	//bbtest
	QuatToPSXMatrix(&frog[0]->actor->qRot, &frog[0]->actor->psiData.object->matrix);
#endif


//	if(currTileNum != 1)
	{
		if (!reachedPoint)
		{
			if(player[0].canJump)
				CheckForDynamicCameraChange(currTile[0],0);
			if(((currTileNum != TILENUM_OPTIONS) || (player[0].canJump == 0)) && (currTileNum != TILENUM_MULTI) && ((currTileNum != TILENUM_CHOICE) || (player[0].canJump) || (player[0].extendedHopDir != 1)))
				UpdateCameraPosition();
		}
	}

	SetCamFF(currCamSource,currCamTarget);

	for( i=0; i<NUM_FROGS; i++ )
	{
		if( frog[i] )
		{
			UpdateFroggerPos(i);
			if(player[i].jumpMultiplier)
				player[i].jumpMultiplier = 1024;

			if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
			{
				frog[i]->draw = 1;
				if (player[i].safe.time)
				{
					GTUpdate( &player[i].safe, -1 );
				}
			}

			if((player[i].canJump) && (player[i].idleEnable && gameState.multi == SINGLEPLAYER ))
			{
				player[i].idleTime-=gameSpeed;
				if(player[i].idleTime<ToFixed(1))
				{
					FroggerIdleAnim(i);
					player[i].idleTime = ToFixed(500 + Random(400));
					if( !Random(10) )
						PlayVoice( i, "frogbored" );
				}
			}

		}
	}  

	UpdatePlatforms();
	UpdateEnemies();
	UpdateSpecialEffects();
	UpdateEvents();
	if( gameState.mode != PAUSE_MODE )
		UpdateAmbientSounds();

	ProcessCollectables();

	UpDateOnScreenInfo();

#ifndef E3_DEMO
/*#ifdef SHOW_ME_THE_TILE_NUMBERS
	cur = &firstTile[0];
	currTileNum = 0;
	while(cur)
	{
		cur = cur->next;

		if(cur == currTile[0])
		{
			currTileNum++;
			break;
		}

		currTileNum++;
	}
	
 	if (tileNum)
 		if (tileNum->text)
 		{
 			if (displayingTile)
 				sprintf(tileString,"%d",currTileNum);
 			else
				sprintf(tileString,"");
 		}
#endif*/
#endif
}



char *chapterPic[BOOK_NUM_PAGES] = 
{
	"CHAPTER1",
	"CHAPTER2",
	"CHAPTER3",
	"CHAPTER4",
	"CHAPTER5",
	"CHAPTER6",
	"CHAPTER7",
	"CHAPTER8",
	"CHAPTER9",
	"CHAPTER10",
};

#ifdef PC_VERSION

void DrawPageB()
{
	char worldStr[100];
	RECT r = {0,0,0xff,0xff};
	RECT r2 = {60,70,0xff-60,0xff-100};
	MDX_TEXENTRY *tex;

	BeginDraw();
	DrawTexturedRect(r,D3DRGBA(1,1,1,1),GetTexEntryFromCRC(UpdateCRC("page32.bmp"))->surf,0,0,1,1);
	sprintf (worldStr,GAMESTRING(STR_CHAPTER),options.pageNum+1);

	DrawFontStringAtLoc(10,10,worldStr,D3DRGBA(0,0,0,0.5),(MDX_FONT *)fontWhite,1,0,0);

	DrawFontStringAtLoc(50,200,GAMESTRING(options.pageNum*2 + STR_CHAPTER_1a),D3DRGBA(0,0,0,0.5),(MDX_FONT *)fontWhite,0.5,128,0);
	DrawFontStringAtLoc(50,220,GAMESTRING(1+options.pageNum*2 + STR_CHAPTER_1a),D3DRGBA(0,0,0,0.5),(MDX_FONT *)fontWhite,0.5,128,0);
	

	sprintf(worldStr,"%s.bmp",chapterPic[options.pageNum]);
	strlwr(worldStr);
	if( (tex = GetTexEntryFromCRC(UpdateCRC(worldStr))) )
		DrawTexturedRect(r2,D3DRGBA(1,1,1,1),tex->surf,0,0,1,1);

	EndDraw();
}

#else
void DrawPageB()
{
}
#endif

unsigned long USE_MENUS = 0;

void DoArcadeMenu()
{
	int i;


	INC_ALPHA(options.subTitle,255);
	INC_ALPHA(options.statusBak,128);
	for(i = 0;i < 2;i++)
		titleHud[i + 2]->yPos = titleHud[i + 2]->yPosTo = options.subTitle->yPos;

	if(!CheckCamStill())
		return;

	strcpy (worldStr,GAMESTRING(worldVisualData[cWorld].description_str));
	
	
	for (i=0; i<MAX_LEVELSTRING; i++)
	{
		levelStr[i][0] = 0;
		levelParStr[i][0] = 0;
		levelSetByStr[i][0] = 0;
		levelCoinStr[i][0] = 0;
	}

	if(cWorld == WORLDID_GARDEN)
		options.worldBak->height = 2*170 + 682;
	else
		options.worldBak->height = (short)worldVisualData[cWorld].numSinglePlayerLevels*170 + 682;


	if(cWorld == WORLDID_GARDEN)
	{
		strcpy (levelStr[0], GAMESTRING(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].description_str));
		options.levelText[0]->b = 255;
		options.levelText[0]->g = 255;
		options.levelText[0]->r = 255;

		strcpy (levelStr[1], GAMESTRING(worldVisualData[WORLDID_GARDEN].levelVisualData[0].description_str));
		options.levelText[1]->b = 255;
		options.levelText[1]->g = 255;
		options.levelText[1]->r = 255;

//		sprintf(levelSetByStr[0],"%s",worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].parName);
//		options.levelSetByText[0]->b = 255;
//		options.levelSetByText[0]->g = 255;
//		options.levelSetByText[0]->r = 255;

		sprintf(levelSetByStr[1],"%s",worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].parName);
		options.levelSetByText[1]->b = 255;
		options.levelSetByText[1]->g = 255;
		options.levelSetByText[1]->r = 255;

//		sprintf(levelParStr[0],"%lu:%02i",worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].parTime/60,worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].parTime%60);
//		options.levelParText[0]->b = 255;
//		options.levelParText[0]->g = 255;
//		options.levelParText[0]->r = 255;

		sprintf(levelParStr[1],"%lu:%02i.%d0",worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].parTime/600,((worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].parTime)/10)%60,(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].parTime)%10);
		options.levelParText[1]->b = 255;
		options.levelParText[1]->g = 255;
		options.levelParText[1]->r = 255;

//		sprintf(levelCoinStr[0],"%d",worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].maxCoins);
//		options.levelCoinMedal[0]->b = 255;
//		options.levelCoinMedal[0]->g = 255;
//		options.levelCoinMedal[0]->r = 255;
//		options.levelCoinText[0]->b = 255;
//		options.levelCoinText[0]->g = 255;
//		options.levelCoinText[0]->r = 255;

		sprintf(levelCoinStr[1],"%d",worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].maxCoins);
		options.levelCoinMedal[1]->b = 255;
		options.levelCoinMedal[1]->g = 255;
		options.levelCoinMedal[1]->r = 255;
		options.levelCoinText[1]->b = 255;
		options.levelCoinText[1]->g = 255;
		options.levelCoinText[1]->r = 255;
	}
	else
	{
		for (i=0; i<worldVisualData[cWorld].numSinglePlayerLevels; i++)
		{
			strcpy (levelStr[i], GAMESTRING(worldVisualData[cWorld].levelVisualData[i].description_str));
			sprintf(levelParStr[i],"%lu:%02i.%d0",worldVisualData[cWorld].levelVisualData[i].parTime/600,((worldVisualData[cWorld].levelVisualData[i].parTime)/10)%60,(worldVisualData[cWorld].levelVisualData[i].parTime)%10);
			sprintf(levelSetByStr[i],"%s",worldVisualData[cWorld].levelVisualData[i].parName);
			if(cWorld == WORLDID_SUPERRETRO)
				levelCoinStr[i][0] = 0;
			else
				sprintf(levelCoinStr[i],"%d",worldVisualData[cWorld].levelVisualData[i].maxCoins);
			if (worldVisualData[cWorld].levelVisualData[i].levelOpen == LEVEL_OPEN)
			{
				options.levelText[i]->b = 255;
				options.levelText[i]->g = 255;
				options.levelText[i]->r = 255;
				options.levelParText[i]->b = 255;
				options.levelParText[i]->g = 255;
				options.levelParText[i]->r = 255;
				options.levelSetByText[i]->b = 255;
				options.levelSetByText[i]->g = 255;
				options.levelSetByText[i]->r = 255;
				options.levelCoinText[i]->b = 255;
				options.levelCoinText[i]->g = 255;
				options.levelCoinText[i]->r = 255;
				options.levelCoinMedal[i]->b = 255;
				options.levelCoinMedal[i]->g = 255;
				options.levelCoinMedal[i]->r = 255;
			}
			else
			{
				options.levelText[i]->b = 0;
				options.levelText[i]->g = 60;
				options.levelText[i]->r = 30;
				options.levelParText[i]->b = 0;
				options.levelParText[i]->g = 60;
				options.levelParText[i]->r = 30;
				options.levelSetByText[i]->b = 0;
				options.levelSetByText[i]->g = 60;
				options.levelSetByText[i]->r = 30;
				options.levelCoinText[i]->b = 0;
				options.levelCoinText[i]->g = 60;
				options.levelCoinText[i]->r = 30;
				options.levelCoinMedal[i]->b = 0;
				options.levelCoinMedal[i]->g = 60;
				options.levelCoinMedal[i]->r = 30;
			}
		}
	}

	options.levelSetByText[options.levelNum]->r = options.levelCoinMedal[options.levelNum]->r = options.levelCoinText[options.levelNum]->r = options.levelParText[options.levelNum]->r = options.levelText[options.levelNum]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
	options.levelSetByText[options.levelNum]->g = options.levelCoinMedal[options.levelNum]->g = options.levelCoinText[options.levelNum]->g = options.levelParText[options.levelNum]->g = options.levelText[options.levelNum]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
	options.levelSetByText[options.levelNum]->b = options.levelCoinMedal[options.levelNum]->b = options.levelCoinText[options.levelNum]->b = options.levelParText[options.levelNum]->b = options.levelText[options.levelNum]->b = 10;



	frogFacing[0] = 3;
	OrientateSS( &frog[0]->actor->qRot, &currTile[0]->dirVector[frogFacing[0]], &currTile[0]->normal );

	
	INC_ALPHA(options.arcadeText,0xff);
	
	INC_ALPHA(options.parText[0],0xff);
	INC_ALPHA(options.parText[1],0xff);
	if(cWorld == WORLDID_SUPERRETRO)
		options.parText[2]->draw = 0;
	else
		INC_ALPHA(options.parText[2],0xff);
	INC_ALPHA(options.selectText,0xff);
	INC_ALPHA(options.worldText,0xff);
	INC_ALPHA(options.worldBak,128);
	INC_ALPHA(options.titleBak,128);
	INC_ALPHA(options.statusBak,128);
	
	for (i=0; i<MAX_LEVELSTRING; i++)
	{
		INC_ALPHA(options.levelText[i],0xff);
		INC_ALPHA(options.levelParText[i],0xff);
		INC_ALPHA(options.levelSetByText[i],0xff);
		if(cWorld == WORLDID_GARDEN)
		{
			if(i == 0)
			{
				options.beatenIcon[i]->draw = 0;
				if(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].maxCoins == 25)
				{
					INC_ALPHA(options.levelCoinMedal[i],0xff);
					options.levelCoinText[i]->draw = 0;
				}
				else
				{
					options.levelCoinMedal[i]->draw = 0;
					INC_ALPHA(options.levelCoinText[i],0xff);
				}
			}
			else if(i == 1)
			{
				if(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].levelBeaten)
				{
					INC_ALPHA(options.beatenIcon[i],254);
				}
				else
					options.beatenIcon[i]->draw = 0;
				if(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].maxCoins == 25)
				{
					INC_ALPHA(options.levelCoinMedal[i],0xff);
					options.levelCoinText[i]->draw = 0;
				}
				else
				{
					options.levelCoinMedal[i]->draw = 0;
					INC_ALPHA(options.levelCoinText[i],0xff);
				}
			}
			else
			{
				options.levelCoinMedal[i]->draw = 0;
				options.beatenIcon[i]->draw = 0;
				INC_ALPHA(options.levelCoinText[i],0xff);
			}
		}
		else
		{
			if(worldVisualData[cWorld].levelVisualData[i].levelBeaten)
			{
				INC_ALPHA(options.beatenIcon[i],254);
			}
			else
				options.beatenIcon[i]->draw = 0;
			
			if(worldVisualData[cWorld].levelVisualData[i].maxCoins == 25)
			{
				INC_ALPHA(options.levelCoinMedal[i],0xff);
				options.levelCoinText[i]->draw = 0;
			}
			else
			{
				options.levelCoinMedal[i]->draw = 0;
				INC_ALPHA(options.levelCoinText[i],0xff);
			}
		}
	}
}

void CheckForExtras()
{
	int i,num = 0;


	for(i = 1;i < NUM_STORY_LEVELS;i++)
		if(worldVisualData[storySequence[i].worldNum].levelVisualData[storySequence[i].levelNum].maxCoins == 25)
			num++;
		

	for(i = NUM_BASIC_EXTRAS;i < NUM_EXTRAS;i++)
	{
		options.extrasAvailable[i] = 0;
		if(options.extras[i])
			options.extras[i]->draw = 0;	
	}

	for(i = 0;i < num;i++)
	{
		if(i < 9)
			worldVisualData[WORLDID_SUPERRETRO].levelVisualData[i + 1].levelOpen = 1;
		else if(i < 13)
			frogPool[i - 9 + 4].active = 1;
		else
			options.extrasAvailable[i - 13 + NUM_BASIC_EXTRAS] = 1;
	}
	if(cheatCombos[CHEAT_OPEN_ALL_EXTRAS].state)
	{
		for(i = NUM_BASIC_EXTRAS;i < NUM_EXTRAS;i++)
		{
			options.extrasAvailable[i] = 1;
//			options.extras[i]->draw = 1;
		}
	}
}

void UpdateCheatText()
{
	if(cheatActivated == -1)
		return;
	
	if (cheatText == NULL)
		cheatText = CreateAndAddTextOverlay(2048,2000,cheatStr,YES,0,font,TEXTOVERLAY_SHADOW);

	GTUpdate(&cheatTimer,-1);

	if(cheatTimer.time >= CHEAT_TEXT_TIME / 2)
	{
		INC_ALPHA(cheatText,255);
	}
	else
	{
		DEC_ALPHA(cheatText);
		if(cheatText->draw == 0)
		{
			SubTextOverlay(cheatText);
			cheatText = NULL;
			cheatActivated = -1;
		}
	}
}
