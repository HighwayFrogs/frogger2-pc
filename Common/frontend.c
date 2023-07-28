/*

	This file is part of Frogger2, (c) 1997 Interactive Studios Ltd.


	File		: frontend.c
	Programmer	: Jim & Dave
	Date		: 30/03/99

----------------------------------------------------------------------------------------------- */

//#include "directx.h"

//#include "ultra64.h"
//#include <stdio.h>
#include "sonylibs.h"
#include "shell.h"
#include "islpsi.h"

#include "frontend.h"
#include "game.h"
#include "define.h"
#include "layout.h"
#include <islfont.h>
#include <islpad.h>
#include <islmem.h>
#include <islutil.h>
#include "textover.h"
#include "backdrop.h"
//#include "objects.h"
#include "menus.h"
#include "frogger.h"
#include "frogmove.h"
#include "overlays.h"
#include "main.h"
#include "newpsx.h"
#include "cam.h"
#include "collect.h"
#include "hud.h"
#include "multi.h"
#include "story.h"
#include "cam.h"
#include "levplay.h"
#include "e3_demo.h"
#include "babyfrog.h"
#include "types.h"
#include "lang.h"
#include "training.h"
#include "fadeout.h"
#include "options.h"

#ifdef PC_VERSION
#include <controll.h>
#include <stdio.h>
#include <banks.h>
#include <pcaudio.h>
#include <pcmisc.h>
#include <pcsprite.h>
#elif PSX_VERSION
extern int cursPos;
#include "audio.h"
#include "temp_psx.h"
#include "psxsprite.h"
#include "memcard.h"
#include <libspu.h>
extern int useMemCard;
#endif

int numExtra;

extern int restartingLevel;
char extraOpenStr[64];
extern int pauseController;
int eolTrackComplete;
int levCompleteState;
enum
{
	LEV_COMPLETE_DROPPING_FROGS,
	LEV_COMPLETE_COUNTING_COINS,
	LEV_COMPLETE_SLIDING_TROPHY,
	LEV_COMPLETE_ENTER_NAME,
	LEV_COMPLETE_MENU,
	LEV_COMPLETE_FADING_OUT,
};

//------ [ GLOBALS ] ---------------------------------------------------------------------------//

extern SPRITEOVERLAY *babyFlash;
int frameCount			= 0;
int timeForLevel;

//int	frameCount2			= 0;
//int dispFrameCount		= 0;

char ActiveController	= 0;
unsigned long levelTime;

long backCharsX[4] = {64,3520,3520,64};
long backCharsY[4] = {420+64,420+64,3360-128-64,3360-128-64};
long backTextX[4] = {64+256+128,3640,3640,64+256+128};
long backTextY[4] = {160+64,160+64,3640,3640};
long backWinsTextX[4] = {64+512,3520-200,3520-200,64+512};
long backWinsTextY[4] = {420+128,420+128,3360-64,3360-64};

TEXTOVERLAY *theEndText = NULL;
TIMER theEndTimer;
void RunTheEndMode()
{
	if(theEndText == NULL)
	{
		FreeTextOverlayLinkedList();
		InitTextOverlayLinkedList(2);
		theEndText = CreateAndAddTextOverlay(2048,2000,GAMESTRING(STR_THE_END),YES,255,font,0);
		GTInit(&theEndTimer,5);
		ScreenFade(0,255,30);
	}
	else if((theEndTimer.time) && (!fadingOut))
	{
		GTUpdate(&theEndTimer,-1);
		if((!theEndTimer.time) || (padData.debounce[0]))
		{
			ScreenFade(255,0,30);
			theEndTimer.time = 0;
		}
	}
	if((!fadingOut) && (theEndTimer.time == 0))
	{
		gameState.mode = FRONTEND_MODE;
		theEndText = NULL;
		InitLevel(player[0].worldNum,player[0].levelNum);
		frameCount = 0;		
	}
}

/*	--------------------------------------------------------------------------------
	Function		: GameLoop
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

extern SOUNDLIST soundList;					// Actual Sound Samples List
void GameLoop(void)
{
	int i;


#ifndef FINAL_MASTER
	if(((padData.debounce[0] & PAD_TRIANGLE) && (padData.digital[0] & PAD_SELECT)) || ((padData.debounce[0] & PAD_SELECT) && (padData.digital[0] & PAD_TRIANGLE)))
		camControlMode = 1 - camControlMode;

	if(camControlMode)
		ControlCamera();
#endif

	UpdateCheatText();


	switch (gameState.mode)
	{
#ifdef PC_VERSION
	case STARTUP_MODE:
		if(modeTimer.time)
		{
			GTUpdate(&modeTimer,-1);
			if(!modeTimer.time)
			{
				ScreenFade(255,0,30);
			}
		}
		else if(!fadingOut)
		{
#ifdef PC_DEMO
			gameState.mode = INGAME_MODE;
#else
			StartVideoPlayback(FMV_HASBRO_LOGO);
			if(quitAllVideo == 0)
			{
				StartVideoPlayback(FMV_BLITZ_LOGO);
				if(quitAllVideo == 0)
					StartVideoPlayback(FMV_INTRO);
			}
			gameState.mode = FRONTEND_MODE;
#endif
			actFrameCount = 0;
			GTInit(&modeTimer,1);
			InitLevel(player[0].worldNum,player[0].levelNum);
			return;
		}
		break;
#endif

	case INGAME_MODE:
		RunGameLoop();

#ifdef PSX_VERSION
		if(padData.present[0] == 0)
		{
			pauseController = 0;
			StartPauseMenu();
		}
#endif
		break;

	case DEMO_MODE:
		RunDemoMode();
		break;

	case THEEND_MODE:
		RunTheEndMode();
		break;

	case FRONTEND_MODE:
		RunFrontendGameLoop();
		break;

	case TRAINING_MODE:
		RunTrainingMode();
#ifdef PSX_VERSION
		if(padData.present[0] == 0)
		{
			pauseController = 0;
			StartPauseMenu();
		}
#endif
		break;

	case LEVELCOMPLETE_MODE:
		RunLevelComplete( );
		break;

	case GAMEOVER_MODE:
		RunGameOver( );
		break;
		
	case CAMEO_MODE:
	case PAUSE_MODE:
		RunPauseMenu();
		break;

	case MULTI_WINBATTLE_MODE:
		RunMultiWinCollect( );
		break;

	case MULTI_WINCOLLECT_MODE:
		RunMultiWinCollect( );
		break;
	case MULTI_WINRACE_MODE:
		RunMultiWinRace( );
		break;

	case ARTVIEWER_MODE:
		RunArtViewer();
		break;
/*
	case CONTROLLERVIEW_MODE:
		RunControllerView();
		break;

#ifdef E3_DEMO
	case E3_LEVELSELECT_MODE:
		RunE3LevelSelect();
		break;
#endif
*/
#ifdef PC_DEMO
	case TEASERSCREEN_MODE:
		RunTeaserScreens( );
		break;
#endif
	}

	frameCount++;

	// ds - what the hell?
	i = NUM_FROGS;
	while(i--)
		if(player[i].inputPause > 0)
			player[i].inputPause = 0;
}



/*	--------------------------------------------------------------------------------
	Function		: CreateOverlaysFromLogo
	Purpose			: Creates a series of sprite overlays from a LOGO, at screen resolution
	Parameters		: LOGO*, left, top
	Returns			: 
	Info			: 
*/

void CreateOverlaysFromLogo(const LOGO* logo, int x, int y)
{
	int i, j;
	char texture[32];
	unsigned short *tex, t;

	tex = (unsigned short*)logo->tiles;

	for (i = 0; i < logo->height; i++)
		for (j = 0; j < logo->width; j++)
		{
			if ( (t=*(tex++)) )
			{
				sprintf(texture, "%s%02d", logo->texname, t);
				CreateAndAddSpriteOverlay( x + (j*32), y + (i*32), texture, 32, 32, 255, 0);
			}
		}
}

/*	--------------------------------------------------------------------------------
	Function		: RunLevelComplete
	Purpose			: What happens at the end of a level
	Parameters		: 
	Returns			: 
	Info			: 
*/

//extern long drawLandscape;
unsigned long createTime;
unsigned long createFrames = 30;

char currentName[64] = "";
char initialName[64] = "";
long waitFrame = 0;
long grade,moreCoins,levelOpened,levelBeaten;
long cOption = 0;
long goFrontend = 1;
TEXTOVERLAY *tText,*coinText,*nameText,*newBestText,*extraText;
TEXTOVERLAY *nText,*oText[2],*oldBestText;
SPRITEOVERLAY *extraIcon,*coinIcon,*bIcon;
char coinStr[64];
char oldBestStr[64];

char levTimeText[64];

char *trophyName[3] = 
{
	"TRPHYGOLD",
	"TRPHYSILV",
	"TRPHYBRONZ",
};



void LevelCompleteProcessController(long pl)
{
	unsigned long changedLevel = 0;
	u16 button[4];

	button[pl] = padData.debounce[pl];
	
	if(button[pl] & PAD_UP)
	{
		if (cOption>0)
		{
			cOption--;
			PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
		}
	}	    
	else if(button[pl] & PAD_DOWN)
	{
		if ((cOption<1) && (oText[1]))
		{
			cOption++;
			PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
		}
	}

	if(button[pl] & (PAD_CROSS))
    {
		PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,SAMPLE_VOLUME,-1);
		showEndLevelScreen = -1;

		levCompleteState = LEV_COMPLETE_FADING_OUT;

		if((grade == 0) || (moreCoins) || (levelOpened) || (levelBeaten))
		{
			SaveGame();
		}

		if (cOption == 1)
			goFrontend = 1;
		else
			goFrontend = 0;
	}
}

int coinsMissed;
int coinCounter = 0;

short numLives[3] = 
{
	10,		//EASY
	5,		//NORMAL
	5		//HARD
};

/*	--------------------------------------------------------------------------------
	Function		: RunGameOver
	Purpose			: Runs the game-over screen
	Parameters		: 
	Returns			: 
*/
void RunGameOver( )
{
	GTUpdate( &modeTimer, -1 );
	if((keepFade == 0) && (!modeTimer.time || padData.debounce[0]))
	{
		ScreenFade(255,0,30);
		keepFade = 1;
	}
	if((keepFade) && (fadingOut == 0))
	{
//		FreeAllLists();
#ifdef E3_DEMO
		StartE3LevelSelect();
#else

#ifdef PC_DEMO
		gameState.mode = TEASERSCREEN_MODE;
		return;
#endif
		gameState.mode = FRONTEND_MODE;

		player[0].lives = numLives[gameState.difficulty];
		player[0].levelNum = LEVELID_FRONTEND1;
		player[0].worldNum = WORLDID_FRONTEND;
		player[0].frogState &= ~FROGSTATUS_ISDEAD;
		InitLevel(player[0].worldNum,player[0].levelNum);

		frameCount = 0;
#endif
	}
}



/*	--------------------------------------------------------------------------------
	Function		: StartGameOver
	Purpose			: Die, froggy! Wuaahahahahahahaha
	Parameters		: 
	Returns			: 
*/
void StartGameOver()
{
	TEXTOVERLAY *gameOverText;
	gameState.mode = GAMEOVER_MODE;
	GTInit( &modeTimer, 9 );

#ifdef PSX_VERSION
	SpuSetKey(SPU_OFF,0xffffff);
#endif

//	FreeAllGameLists();

	gameOverText = CreateAndAddTextOverlay(2048, 1980, GAMESTRING(STR_GAMEOVER), YES, (char)255, font, TEXTOVERLAY_SHADOW);
	gameOverText->g = gameOverText->b = 0;
	frog[0]->draw = 0;
	PrepareSong(AUDIOTRK_GAMEOVER,NO);
}

/*	--------------------------------------------------------------------------------
	Function		: StartGameIntro
	Purpose			: Runs the intro sequence, ISL logos and so on
	Parameters		: 
	Returns			: 

	NOTE! This is a very crude placeholder intro sequence. Replace it!
*/
void StartGameIntro()
{
	// todo? pending animation playback, I suppose.

	player[0].worldNum = WORLDID_FRONTEND;
	player[0].levelNum = LEVELID_FRONTEND1;
	
	gameState.mode = LEVELCOMPLETE_MODE;
	gameState.multi = SINGLEPLAYER;
	GTInit( &modeTimer, 1 );
	showEndLevelScreen = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: Multiplayer win screens
	Purpose			: countdown scores and play animations and stuff
	Parameters		: 
	Returns			: 
*/
typedef struct 
{
	TEXTOVERLAY		*countTextOver[MAX_FROGS];
	char countText[MAX_FROGS][32], current, mode;
	unsigned long countdown;
} MULTIGAMEWIN;

enum
{
	MGW_MOVING,
	MGW_COUNTING,
	MGW_FLASHING,
	MGW_FINISHED
};

MULTIGAMEWIN *mgWin;


void StartMultiWinGame( )
{
	int i;
	SPRITE *cur,*next;

	ScreenFade(255,128,30);
	keepFade = 1;
	fadeText = NO;

	mgWin = (MULTIGAMEWIN *)MALLOC0(sizeof(MULTIGAMEWIN));

	multiHud.centreText->yPos = 1600;
	switch( multiplayerMode )
	{
	case MULTIMODE_RACE:
		gameState.mode = MULTI_WINRACE_MODE;


		for( i=0; i<NUM_FROGS; i++ )
		{
			sprintf( mgWin->countText[i], "%d:%02d:%02d", (mpl[i].timer/3600), ((mpl[i].timer%3600)/60), (mpl[i].timer%60) );
			mgWin->countTextOver[i] = CreateAndAddTextOverlay(5000, 1024+(512*i), mgWin->countText[i], NO, 255, font, TEXTOVERLAY_SHADOW);
			mgWin->countTextOver[i]->speed = 50<<12;
		}

		mgWin->mode = MGW_MOVING;
		break;

	case MULTIMODE_COLLECT:
	case MULTIMODE_BATTLE:
		mgWin->mode = MGW_MOVING;
		GTInit( &modeTimer, 3 );
		if(matchWinner != -1)
		{
			sprintf(countdownString,GAMESTRING(STR_MULTI_WINMATCH),GAMESTRING(STR_CHAR_NAME_1 + player[gameWinner].character));
		}
		else if(gameWinner == MULTI_ROUND_DRAW)
		{
			sprintf(countdownString,GAMESTRING(STR_MULTI_DRAW));
		}
		else
		{
			sprintf(countdownString,GAMESTRING(STR_MULTI_PLAYERWINS),GAMESTRING(STR_CHAR_NAME_1 + player[gameWinner].character));
		}
		multiHud.centreText->a = 0;
		multiHud.centreText->scale = 4096;
		gameState.mode = MULTI_WINCOLLECT_MODE;
		break;
	}


	if (matchWinner != -1)
	{
		drawGame = 0;

		InitTiledBackdrop("LOGO");
		for ( cur = sprList.head.next; ( cur != &sprList.head ); cur = next)
		{
			next = cur->next;
			cur->draw = 0;
		}

	}
}

void SlideSpriteOverlayToPos(SPRITEOVERLAY *s,long x,long y,long fromX,long fromY,long numFrames)
{
	long speed;
	FVECTOR vect;

	vect.vx = x - fromX;
	vect.vy = y - fromY;
	vect.vz = 0;

	speed = MagnitudeF(&vect);
	speed = FMul(speed,gameSpeed);

	vect.vx = x - s->xPos;
	vect.vy = y - s->yPos;

	MakeUnit(&vect);

	if(abs(s->xPos - x) <= speed/numFrames)
		s->xPos = x;
	else
		s->xPos += FMul(vect.vx,speed/numFrames);
	if(abs(s->yPos - y) <= speed/numFrames)
		s->yPos = y;
	else
		s->yPos += FMul(vect.vy,speed/numFrames);

	s->xPosTo = s->xPos;
	s->yPosTo = s->yPos;
}

void SlideTextOverlayToPos(TEXTOVERLAY *s,long x,long y,long fromX,long fromY,long numFrames)
{
	long speed;
	FVECTOR vect;

	vect.vx = x - fromX;
	vect.vy = y - fromY;
	vect.vz = 0;

	speed = MagnitudeF(&vect);
	speed = FMul(speed,gameSpeed);

	vect.vx = x - s->xPos;
	vect.vy = y - s->yPos;

	MakeUnit(&vect);

	if(abs(s->xPos - x) <= speed/numFrames)
		s->xPos = x;
	else
		s->xPos += FMul(vect.vx,speed/numFrames);
	if(abs(s->yPos - y) <= speed/numFrames)
		s->yPos = y;
	else
		s->yPos += FMul(vect.vy,speed/numFrames);

	s->xPosTo = s->xPos;
	s->yPosTo = s->yPos;
}


void RunMultiWinRace( )
{
	SPRITEOVERLAY *s;
	TEXTOVERLAY *t;
	int n = mgWin->current, i,numArrived;
	int j;

#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
	if( mgWin->mode == MGW_MOVING )
	{
		DEC_ALPHA(multiHud.timeTextHSec);
		DEC_ALPHA(multiHud.timeTextSec);
		DEC_ALPHA(multiHud.timeTextMin);
		numArrived = 0;
		for (n=0; n<NUM_FROGS; n++)
		{
			s = multiHud.backChars[n];
			t = mgWin->countTextOver[n];

			SlideSpriteOverlayToPos(s,700,1024 + 512*n-128,backCharsX[n],backCharsY[n],50);
			SlideTextOverlayToPos(multiHud.penaliseText[n],1300,1024 + 512*n,backTextX[n],backTextY[n],50);

			t->xPosTo = 2000;
			if((t->xPos == t->xPosTo))
				numArrived++;
			
		}

		// If overlays have got to proper positions, start counting
		if(numArrived == NUM_FROGS)
		{
			mgWin->mode = MGW_COUNTING;
			mgWin->countdown = 0;
			mgWin->current = 0;
		}
	}
	else if( mgWin->mode == MGW_COUNTING )
	{
		if(padData.digital[0] & PAD_CROSS)
			mgWin->countdown += 5;
		else
			mgWin->countdown += actFrameCount-lastActFrameCount;
		t = mgWin->countTextOver[mgWin->current];
		
		// Every half a second, countdown a penalty point and make a noise
		if( mgWin->countdown > 5 )
		{
			mgWin->countdown = 0;
			if( mpl[n].penalty ) // Countdown another penalty if we can
			{
				mpl[n].penalty -= 60;
				mpl[n].timer += 60;
				sprintf( t->text, "%d:%02d:%02d", (mpl[n].timer/3600), ((mpl[n].timer%3600)/60), (mpl[n].timer%60) );

   				PlaySample( genSfx[GEN_CLOCKTICK], NULL, 0, SAMPLE_VOLUME, -1 );
			}
			else if( n < NUM_FROGS-1 ) // Else if more frogs to go then bring on the next set of overlays
			{
				mgWin->current++;
			}
			else // Else we've finished, so prepare to replay.
			{
				for(i = 0;i < NUM_FROGS;i++)
				{
					for(j = 0;j < mpl[i].wins;j++)
						multiHud.trophies[i][j]->draw = 1;
				}
				mgWin->mode = MGW_FLASHING;
				multiHud.centreText->a = 0;
				multiHud.centreText->yPos = 900;
				GTInit( &modeTimer, 3 );
			}
		} 
	}
	else if( mgWin->mode == MGW_FLASHING )
	{
		GTUpdate( &modeTimer, -1 );
		if((gameWinner == MULTI_ROUND_DRAW) || (!modeTimer.time))
		{
			mgWin->mode = MGW_FINISHED;
			GTInit( &modeTimer, 4 );
		}
		else
		{
			multiHud.backChars[gameWinner]->r = multiHud.penaliseText[gameWinner]->r = mgWin->countTextOver[gameWinner]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
			multiHud.backChars[gameWinner]->g = multiHud.penaliseText[gameWinner]->g = mgWin->countTextOver[gameWinner]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
			multiHud.backChars[gameWinner]->b = multiHud.penaliseText[gameWinner]->b = mgWin->countTextOver[gameWinner]->b = 10;
		}
	}
	else if( mgWin->mode == MGW_FINISHED )
	{
		if(modeTimer.time)
		{
			if( gameWinner != MULTI_ROUND_DRAW )
			{
				multiHud.backChars[gameWinner]->r = multiHud.penaliseText[gameWinner]->r = mgWin->countTextOver[gameWinner]->r = 255;
				multiHud.backChars[gameWinner]->g = multiHud.penaliseText[gameWinner]->g = mgWin->countTextOver[gameWinner]->g = 255;
				multiHud.backChars[gameWinner]->b = multiHud.penaliseText[gameWinner]->b = mgWin->countTextOver[gameWinner]->b = 255;
			}
			if(matchWinner != -1)
			{
				for( i=0; i<NUM_FROGS; i++ )
				{
					SlideSpriteOverlayToPos(multiHud.backChars[i],backCharsX[i],backCharsY[i],700,1024 + 512*n-128,50);
					DEC_ALPHA(multiHud.penaliseText[i]);
					mgWin->countTextOver[i]->xPosTo = 5200;
					DEC_ALPHA( mgWin->countTextOver[i] );
				}
			}
			else
			{
				INC_ALPHA(multiHud.timeTextHSec,255);
				INC_ALPHA(multiHud.timeTextSec,255);
				INC_ALPHA(multiHud.timeTextMin,255);
				for( i=0; i<NUM_FROGS; i++ )
				{
					SlideSpriteOverlayToPos(multiHud.backChars[i],backCharsX[i],backCharsY[i],700,1024 + 512*n-128,50);
					SlideTextOverlayToPos(multiHud.penaliseText[i],backTextX[i],backTextY[i],1300,1024 + 512*n,50);
					mgWin->countTextOver[i]->xPosTo = 5200;
					DEC_ALPHA( mgWin->countTextOver[i] );
				}
			}
			INC_ALPHA(multiHud.centreText,255);
		}

#ifdef E3_DEMO
		if (gameWinner == MULTI_ROUND_DRAW)
		{
			strcpy(countdownString, "DRAW");
			multiHud.centreText->draw = 1;
		}
#else
		if(matchWinner != -1)
		{
			if (gameState.multi == MULTILOCAL)
				sprintf(countdownString,GAMESTRING(STR_MULTI_WINMATCH),GAMESTRING(STR_CHAR_NAME_1 + player[gameWinner].character));
#ifdef PC_VERSION
			else
				sprintf(countdownString,GAMESTRING(STR_MULTI_WINMATCH),NetGetPlayerName(gameWinner));
#endif
		}
		else if( gameWinner == MULTI_ROUND_DRAW )
		{
			sprintf(countdownString,GAMESTRING(STR_MULTI_DRAW));
		}
		else
		{
			if (gameState.multi == MULTILOCAL)
				sprintf(countdownString,GAMESTRING(STR_MULTI_PLAYERWINS),GAMESTRING(STR_CHAR_NAME_1 + player[gameWinner].character));
#ifdef PC_VERSION
			else
				sprintf(countdownString,GAMESTRING(STR_MULTI_PLAYERWINS),NetGetPlayerName(gameWinner));
#endif
		}

		multiHud.centreText->text = countdownString;
		multiHud.centreText->scale = 4096;
#endif

		if(modeTimer.time)
			GTUpdate( &modeTimer, -1 );
		if(!modeTimer.time)
		{
			if(matchWinner != -1)
			{
				DoEndMulti();
				return;
			}


			ScreenFade(128,255,30);
			keepFade = 0;
			fadeText = NO;

			for( i=0; i<NUM_FROGS; i++ )
			{
				SubTextOverlay( mgWin->countTextOver[i] );
			}

//			FreeBackdrop();
//			drawGame = 1;

			FREE( mgWin );
			mgWin = NULL;
			frameCount = 2; // Not 0, because 0 and 1 are used to trigger initialisation stuff elsewhere
			ResetMultiplayer( );
//			EnableMultiHUD();

			gameState.mode = INGAME_MODE;
		}
	}

	UpDateMultiplayerInfo( );
}


void RunMultiWinCollect( )
{
	int i,j,maxScore = 0,numDraw = 0,num = 0;
#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
	switch(mgWin->mode)
	{
		case MGW_FLASHING:
			if(modeTimer.time)
			{
				GTUpdate( &modeTimer, -1 );
				INC_ALPHA(multiHud.centreText,255);
			}
			if( !modeTimer.time )
			{
		//		FreeBackdrop();
		//		drawGame = 1;

				if(matchWinner != -1)
				{
					DoEndMulti();
					return;
				}

				multiHud.centreText->draw = 0;
				multiHud.centreText->yPos = 900;
				ScreenFade(128,255,30);
				keepFade = 0;
				fadeText = NO;

				FREE( mgWin );
				mgWin = NULL;
				frameCount = 2; // Not 0, because 0 and 1 are used to trigger initialisation stuff elsewhere
				ResetMultiplayer( );
		//		EnableMultiHUD();

				gameState.mode = INGAME_MODE;
			}
			break;

		case MGW_MOVING:
		case MGW_COUNTING:
			for(j = 0;j < NUM_FROGS;j++)
			{
				if(mpl[j].score > maxScore)
					maxScore = mpl[j].score;
			}

			for(j = 0;j < NUM_FROGS;j++)
			{
				if(mpl[j].score == maxScore)
					numDraw++;
			}

			if(mgWin->mode == MGW_MOVING)
			{
				for(j = 0;j < NUM_FROGS;j++)
				{
					if(mpl[j].score == maxScore)
					{
						SlideSpriteOverlayToPos(multiHud.backChars[j],2048 - 256 - (numDraw - 1)*256 + num*512,2100,backCharsX[j],backCharsY[j],30);
						num++;
					}
				}
				GTUpdate( &modeTimer, -1 );
				INC_ALPHA(multiHud.centreText,255);
				if(!modeTimer.time)
				{
					for(i = 0;i < NUM_FROGS;i++)
					{
						for(j = 0;j < mpl[i].wins;j++)
							multiHud.trophies[i][j]->draw = 1;
					}
					GTInit(&modeTimer,2);
					mgWin->mode = MGW_COUNTING;
				}
			}
			else
			{
				for(j = 0;j < NUM_FROGS;j++)
				{
					if(mpl[j].score == maxScore)
					{
						SlideSpriteOverlayToPos(multiHud.backChars[j],backCharsX[j],backCharsY[j],2048 - 256 - (numDraw-1)*256 + num*512,2100,30);
						num++;
					}
				}
				GTUpdate( &modeTimer, -1 );
				INC_ALPHA(multiHud.centreText,255);
				if(!modeTimer.time)
				{
					mgWin->mode = MGW_FLASHING;
				}
			}
			break;
	}
}


void RunMultiWinBattle( )
{
	int j;

#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
	switch(mgWin->mode)
	{
		case MGW_FLASHING:
			if(modeTimer.time)
			{
				GTUpdate( &modeTimer, -1 );
				INC_ALPHA(multiHud.centreText,255);
			}
			break;

		case MGW_MOVING:
			for(j = 0;j < NUM_FROGS;j++)
			{
				
			}
			break;
	}
	if( !modeTimer.time )
	{
//		FreeBackdrop();
//		drawGame = 1;


		if(matchWinner != -1)
		{
			DoEndMulti();
			return;
		}


		multiHud.centreText->draw = 0;
		multiHud.centreText->yPos = 900;
		ScreenFade(128,255,30);
		keepFade = 0;
		fadeText = NO;

		FREE( mgWin );
		mgWin = NULL;

		frameCount = 2; // Not 0, because 0 and 1 are used to trigger initialisation stuff elsewhere
		ResetMultiplayer( );
//		EnableMultiHUD();

		gameState.mode = INGAME_MODE;
	}
}


extern int storeFrameCount;
void SetTimeForLevel( )
{
	timeForLevel = (actFrameCount - storeFrameCount)/6;
}

TEXTOVERLAY *menuText[2] = {NULL,NULL};
int menuOption = -1;
int optionSelected = 0;
void DoEndMulti()
{
	int i;

#ifdef E3_DEMO
	StartE3LevelSelect();
#else


	if(menuText[0] == NULL)
	{
//		menuText[0] = CreateAndAddTextOverlay(2048,2400,GAMESTRING(STR_RESTARTLEVEL),YES,0,font,TEXTOVERLAY_SHADOW);
//		menuText[1] = CreateAndAddTextOverlay(2048,2700,GAMESTRING(STR_QUIT),YES,0,font,TEXTOVERLAY_SHADOW);
	}

	if(menuOption == -1)
	{
		menuOption = 0;
		optionSelected = 0;
	}

	if(optionSelected == 0)
	{
		INC_ALPHA(menuText[0],255);
		if(menuText[1])
		{
			INC_ALPHA(menuText[1],255);
		}
		if((menuOption == 1) && (padData.digital[0] & PAD_UP))
			menuOption = 0;
		else if((menuOption == 0) && (padData.digital[0] & PAD_DOWN) && (gameState.multi == MULTILOCAL))
			menuOption = 1;
		else if(padData.digital[0] & PAD_CROSS)
		{
			optionSelected = 1;
		}

		menuText[menuOption]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
		menuText[menuOption]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
		menuText[menuOption]->b = 10;

		if(menuText[1 - menuOption])
			menuText[1 - menuOption]->r = menuText[1 - menuOption]->g = menuText[1 - menuOption]->b = 255;

		return;
	}


	if(fadingOut == 0)
	{
		if(keepFade)
		{
			if(menuOption == 0)
			{
				if(gameState.multi == MULTIREMOTE)
					PostQuitMessage(0);		
				ScreenFade(128,255,30);
				fadeText = NO;
				keepFade = 0;
			}
			else
			{
				ScreenFade(128,0,30);
				keepFade = 0;
			}
		}
		else
		{
			FreeTiledBackdrop();
			drawGame = 1;

			if(menuOption == 0)
			{
#ifdef PC_VERSION
				checkMenuKeys = 0;
#endif
				multiHud.centreText->yPos = 900;
				RestartMulti();
				gameState.mode = INGAME_MODE;
				menuText[0]->draw = 0;
				if(menuText[1])
					menuText[1]->draw = 0;
			}
			else
			{
				player[0].character = FROG_FROGGER;
				NUM_FROGS=1;

				gameState.multi = SINGLEPLAYER;

				gameState.mode = FRONTEND_MODE;
				FreeMultiplayer( );
				InitLevel(WORLDID_FRONTEND,LEVELID_FRONTEND1);

				frameCount = 0;
			}
			menuOption = -1;
		}
	}
	else
	{
		if(menuOption == 1)
		{
			if(multiHud.centreText)
				DEC_ALPHA(multiHud.centreText);
			if(multiHud.timeTextHSec)
				DEC_ALPHA(multiHud.timeTextHSec);
			if(multiHud.timeTextSec)
				DEC_ALPHA(multiHud.timeTextSec);
			if(multiHud.timeTextMin)
				DEC_ALPHA(multiHud.timeTextMin);
			for(i = 0;i < NUM_FROGS;i++)
			{
				DEC_ALPHA(multiHud.backChars[i]);
				DEC_ALPHA(multiHud.penaliseText[i]);
			}
		}
		DEC_ALPHA(menuText[0]);
		if(menuText[1])
		{
			DEC_ALPHA(menuText[1]);
		}
	}
#endif
}

SVECTOR frogPos[6] = 
{
	{500,0,1000},
	{300,0,1200},
	{0,0,1500},
	{-300,0,1200},
	{-500,0,1000},
	{0,0,850},
};

int babyFrogY[5] = 
{
	3500,3800,4096,3800,3500
};
int babyFrogW[5] = 
{
	-1000,-400,0,400,1000
};


int frogY = 1000;
int babyY = 1000;
int frogYStep = 500;
int babyscale = 1400;
int froganimnum = FROG_ANIM_FALL;
int froganimspeed = 100;
int froganimnum2 = FROG_ANIM_FALLLAND;
int froganimspeed2 = 150;
int froganimnum3 = FROG_ANIM_DANCE1;
int froganimspeed3 = 100;
int froganimnum4 = FROG_ANIM_BREATHE;
int froganimspeed4 = 100;
TEXTOVERLAY *levName;
#define MAX_SPARKLES 15

SPRITEOVERLAY *sparkles[MAX_SPARKLES];
SPRITEOVERLAY *enterNameButton[5];

void StartLevelComplete()
{
	int i,num,w;
	ACTOR2 *c;
	SPRITE *cur,*next;
	char tempChar;

	arcadeHud.coinZoom->draw = 0;
#ifdef PSX_VERSION
	SpuSetKey(SPU_OFF,0xffffff);
#endif


#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
//	PrepareSong(AUDIOTRK_LEVELCOMPLETE,NO);

	UndoChangeModel( frog[0]->actor );
	eolTrackComplete = 0;
	if(train)
	{
		if(train->bg)
		{
			train->bg->draw = 0;
			train->bg = 0;
		}
		if(train->txtover[0])
		{
			train->txtover[0]->draw = 0;
			train->txtover[0] = 0;
		}
		if(train->txtover[1])
		{
			train->txtover[1]->draw = 0;
			train->txtover[1] = 0;
		}
	}


	coinsMissed = coinCounter = 0;
	arcadeHud.collectText->draw = 0;

	FreeAmbientSoundList();
	ScreenFade(0,255,20);
	keepFade = NO;
	flashScreen = YES;

	levelOpened = 0;
	levelBeaten = 0;
	arcadeHud.collectText->draw = 0;

	if(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelCompleted == 0)
		levelOpened = 1;

	if((gameState.difficulty != DIFFICULTY_HARD) || (arcadeHud.timeOutText->draw == 0))
		worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelCompleted = 1;

	if((gameState.single == STORY_MODE) && (gameState.storySequenceLevel < NUM_STORY_LEVELS - 1) && ((gameState.difficulty != DIFFICULTY_HARD) || (arcadeHud.timeOutText->draw == 0)))
	{
		gameState.storySequenceLevel++;
		if(worldVisualData[storySequence[gameState.storySequenceLevel].worldNum].worldOpen == WORLD_CLOSED)
			levelOpened = 1;
		worldVisualData[storySequence[gameState.storySequenceLevel].worldNum].worldOpen = WORLD_OPEN;
		if(worldVisualData[storySequence[gameState.storySequenceLevel].worldNum].levelVisualData[storySequence[gameState.storySequenceLevel].levelNum].levelOpen == LEVEL_CLOSED)
			levelOpened = 1;
		worldVisualData[storySequence[gameState.storySequenceLevel].worldNum].levelVisualData[storySequence[gameState.storySequenceLevel].levelNum].levelOpen = LEVEL_OPEN;
	}

	cOption = 0;
	DisableHUD();

	InitTiledBackdrop ("LOGO");
	
	for(i = 0;i< numBabies;i++)
		babyIcons[i]->draw = 0;
	babyFlash->draw = 0;


	if(!train)
	{
#ifdef PSX_VERSION
		PsxNameEntryInit();
#else
		PcNameEntryInit();
#endif
	}

	for (c = actList; c; c = c->next)
		c->draw = 0;
	
	
	for ( cur = sprList.head.next; ( cur != &sprList.head ); cur = next)
	{
		next = cur->next;
		cur->draw = 0;
	}

	createTime = actFrameCount + createFrames;

	moreCoins = 0;

	arcadeHud.coinsOver->xPos = 3400 + 4096;
	coinText = NULL;
	coinIcon = NULL;
	extraText = NULL;
	extraIcon = NULL;
	if(garibList.maxCoins)
	{
		if(player[0].numSpawn == garibList.maxCoins)
		{
			coinText = CreateAndAddTextOverlay(2048+4096,850,GAMESTRING(STR_GOT_ALL_COINS),YES,255,font,TEXTOVERLAY_SHADOW);
			coinText->xPosTo = 2048;
			coinIcon = CreateAndAddSpriteOverlay(2048 - 256 - 4096,850+300,"COINMEDAL",512,512,255,0);
			coinIcon->xPosTo = 2048-256;
			if(player[0].numSpawn > worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].maxCoins)
			{
				worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].maxCoins = player[0].numSpawn;
				moreCoins = 1;
				for(i = 1,numExtra = 0;i < NUM_STORY_LEVELS;i++)
				{
					if(worldVisualData[storySequence[i].worldNum].levelVisualData[storySequence[i].levelNum].maxCoins == 25)
						numExtra++;
				}
#ifndef PC_DEMO
				if(numExtra <= 9)
				{
			 		extraText = CreateAndAddTextOverlay(2048+4096+4096,850+750,extraOpenStr,YES,255,font,TEXTOVERLAY_SHADOW);
					sprintf(extraOpenStr,GAMESTRING(STR_OPENED_EXTRA_1),GAMESTRING(worldVisualData[WORLDID_SUPERRETRO].levelVisualData[numExtra].description_str));
				}
				else
			 		extraText = CreateAndAddTextOverlay(2048+4096+4096,850+750,GAMESTRING(STR_OPENED_EXTRA_1 + numExtra - 9),YES,255,font,TEXTOVERLAY_SHADOW);
				extraText->xPosTo = 2048;
				extraIcon = CreateAndAddSpriteOverlay(2048-256-4096-4096,850+1050,storySequence[numExtra].iconName,512,512,255,0);
				extraIcon->xPosTo = 2048-256;
#endif
			}
			arcadeHud.coinsOver->draw = 0;
		}
		else
		{
			sprintf(coinStr,GAMESTRING(STR_MISSED_COINS),0);//,garibList.maxCoins - player[0].numSpawn);
			coinText = CreateAndAddTextOverlay(2048+4096,850,coinStr,YES,255,font,TEXTOVERLAY_SHADOW);
			coinText->xPosTo = 2048;
			coinIcon = NULL;
	 		extraText = CreateAndAddTextOverlay(2048+4096+4096,850+300,GAMESTRING(STR_NO_BONUS),YES,255,font,TEXTOVERLAY_SHADOW);
			extraText->xPosTo = 2048;
			extraIcon = NULL;
			arcadeHud.coinsOver->draw = 1;
			arcadeHud.coinsOver->xPosTo = 3400;
			arcadeHud.coinsOver->yPosTo = arcadeHud.coinsOver->yPos = 850;
		}
	}

	if(player[0].numSpawn > worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].maxCoins)
	{
		worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].maxCoins = player[0].numSpawn;
		moreCoins = 1;
	}

	if(train)
	{
		grade = 3;
		coinText = CreateAndAddTextOverlay(2048+4096,980+300,GAMESTRING(STR_TRAINING_COMPLETE),YES,255,font,TEXTOVERLAY_SHADOW);
		coinText->xPosTo = 2048;
	}
	else
	{
		if((gameState.difficulty == DIFFICULTY_HARD) && (worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelBeaten == 0) && (arcadeHud.timeOutText->draw == 0))
			worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].levelBeaten = levelBeaten = 1;
		if(timeForLevel < worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime)
			grade = 0;
		else if(timeForLevel < (worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime * 3)/2)
			grade = 1;
		else
			grade = 2;
		if(grade < 2)
		{
			for (i=0; i<MAX_SPARKLES; i++)
			{
				sparkles[i] = CreateAndAddSpriteOverlay( 256,1535,"FLASH",200,200,(UBYTE)Random(0xff),SPRITE_ADDITIVE);
				if(sparkles[i])
				{
					sparkles[i]->num = 1;
					sparkles[i]->draw = 0;
					sparkles[i]->a = Random(256);
				}
			}
		}
	}

	if (grade==0)
	{
		newBestText = CreateAndAddTextOverlay(2048+4096,850+800,GAMESTRING(STR_NEW_BEST_TIME),YES,255,font,TEXTOVERLAY_SHADOW);
		newBestText->r = 255;
		newBestText->g = 255;
		newBestText->b = 0;
		newBestText->xPosTo = 2048;
	}
	else
		newBestText = NULL;

	if(train)
		sprintf(levTimeText,"");
	else
		sprintf(levTimeText,GAMESTRING(STR_YOUTOOKTIMEMIN),((int)timeForLevel/600)%600,((int)timeForLevel/10)%60,((int)timeForLevel)%10);

	
	levName = CreateAndAddTextOverlay(2048, 200+210,
		GAMESTRING(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].description_str),
		YES, (char)0xFF, font, TEXTOVERLAY_SHADOW);

	levName->r = 0;
	levName->g = 255;
	levName->b = 255;
	
	if(grade < 3)
	{
		tText = CreateAndAddTextOverlay(2048+4096, 850, levTimeText, YES, (char)0xFF, font, TEXTOVERLAY_SHADOW);
		tText->xPosTo = 2048;
		bIcon = CreateAndAddSpriteOverlay(2048-256+4096,1150,trophyName[grade],512,512,255,0);
		bIcon->xPosTo = 2048 - 256;
		oldBestText = CreateAndAddTextOverlay(2048 + 4096,1950,oldBestStr,YES,255,font,TEXTOVERLAY_SHADOW);
		oldBestText->xPosTo = 2048;
		sprintf(oldBestStr,GAMESTRING(STR_RECORD),worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parName,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime/600)%600,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime/10)%60,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime)%10);
	}
	else
	{
		tText = NULL;
		bIcon = NULL;
	}

	if((gameState.single == STORY_MODE) && (arcadeHud.timeOutText->draw == 0))
	{
		oText[0] = CreateAndAddTextOverlay(2048+4096, 3420, GAMESTRING(STR_PRESS_X_TO_CONTINUE), YES, (char)0xFF, font, TEXTOVERLAY_SHADOW);
		oText[0]->xPosTo = 2048;
		oText[1] = NULL;
	}
	else
	{
		oText[0] = CreateAndAddTextOverlay(2048+4096, 3220, GAMESTRING(STR_RESTARTLEVEL), YES, (char)0xFF, font, TEXTOVERLAY_SHADOW);
		oText[1] = CreateAndAddTextOverlay(2048-4096, 3610, GAMESTRING(STR_QUIT), YES, (char)0xFF, font, TEXTOVERLAY_SHADOW);
		oText[0]->xPosTo = 2048;
		oText[1]->xPosTo = 2048;
	}
	nText = NULL;
	
	if (grade==0)
	{
		sprintf(currentName,"%s %s",GAMESTRING(STR_ENTER_NAME),textString);
		nText = CreateAndAddTextOverlay(4096, 3350, currentName, NO, (char)0xFF, font, TEXTOVERLAY_SHADOW);
#ifdef PSX_VERSION
		w = fontExtentWScaled(nText->font,GAMESTRING(STR_ENTER_NAME),4096)*4 + fontExtentWScaled(nText->font,"AAAAAA",4096)*4;
		nText->xPosTo = 2048 - w;
		nText->xPos = nText->xPosTo + 4096;
#elif PC_VERSION
		w = (float)(CalcStringWidth(nText->text,(MDX_FONT *)nText->font,1)) * 3.2;
		nText->xPosTo = 2048 - w;
		nText->xPos = nText->xPosTo + 4096;
#endif

#ifdef PSX_VERSION
		enterNameButton[1] = CreateAndAddSpriteOverlay(nText->xPosTo + w*2 + 100,3350,"BUT_RIGH",4096,0,0,0);
		enterNameButton[4] = CreateAndAddSpriteOverlay(nText->xPosTo + w*2 + 600,3350,"BUT_CROS",4096,0,0,0);
		w = fontExtentWScaled(nText->font,GAMESTRING(STR_ENTER_NAME),4096)*8;
		enterNameButton[0] = CreateAndAddSpriteOverlay(nText->xPosTo + w - 200,3350,"BUT_LEFT",4096,0,0,0);
		tempChar = textString[cursPos];
		textString[cursPos] = 0;
		enterNameButton[2] = CreateAndAddSpriteOverlay(nText->xPosTo + w + 24 + fontExtentWScaled(nText->font,textString,4096)*8,3050,"BUT_UP",4096,0,0,0);
		enterNameButton[3] = CreateAndAddSpriteOverlay(nText->xPosTo + w + 24 + fontExtentWScaled(nText->font,textString,4096)*8,3650,"BUT_DOWN",4096,0,0,0);
		textString[cursPos] = tempChar;
		for(i = 0;i < 5;i++)
			enterNameButton[i]->draw = 0;
#endif
	}
	else
		nText = NULL;

	actorAnimate(frog[0]->actor,FROG_ANIM_DANCE1,YES,NO,80,0);

#ifdef PC_DEMO
	StopSample( FindSample(UpdateCRC("lp_music")) );
	PlaySample( FindSample(UpdateCRC("lp_eolmusic")), NULL, 0, SAMPLE_VOLUME/2, -1 );
#endif

//	if((grade != 0) && ((moreCoins) || (levelOpened)))
//		SaveGame();

	levCompleteState = LEV_COMPLETE_DROPPING_FROGS;

	currCamSource.vx = 0;
	currCamSource.vy = 500<<12;
	currCamSource.vz = 0;

	currCamTarget.vx = 0;
	currCamTarget.vy = 0;
	currCamTarget.vz = 3000<<12;

	camVect.vx = 0;
	camVect.vy = 4096;
	camVect.vz = 0;

	SetCamFF(currCamSource,currCamTarget);

	for(i = 0;i < numBabies;i++)
	{
		babyList[i].baby->draw = 1;
		babyList[i].baby->depthShift = 0;
		SetVectorSS(&babyList[i].baby->actor->position,&frogPos[i]);
		babyList[i].baby->actor->position.vy = frogY+babyY+i*frogYStep;
		babyList[i].baby->actor->qRot.x = 0;
		babyList[i].baby->actor->qRot.y = babyFrogY[i];
		babyList[i].baby->actor->qRot.z = 0;
		babyList[i].baby->actor->qRot.w = babyFrogW[i];

		babyList[i].baby->actor->size.vx = babyList[i].baby->actor->size.vy = babyList[i].baby->actor->size.vz = babyscale;

		actorAnimate(babyList[i].baby->actor,BABY_ANIM_COLLECTHOLD,YES,NO,100,NO);
	}

	SetVectorSS(&frog[0]->actor->position,&frogPos[5]);
	frog[0]->actor->position.vy = frogY;
	frog[0]->actor->qRot.x = 0;
	frog[0]->actor->qRot.y = 4096;
	frog[0]->actor->qRot.z = 0;
	frog[0]->actor->qRot.w = 0;
	frog[0]->draw = 1;
	actorAnimate(frog[0]->actor,froganimnum,NO,NO,froganimspeed,NO);
	drawLandscape = 0;
}

int dropSpeed = 40;
int babyanimnum = 1;
int babyanimspeed = 180;
int babyanimspeed2 = 100;
#ifdef PSX_VERSION
int babyanimy = 100;
#else
int babyanimy = 300;
#endif

#define NUM_CELEBRATE_ANIMS 7

int celebrateAnim[NUM_CELEBRATE_ANIMS] = 
{
	FROG_ANIM_HOPWHOOHOO,
	FROG_ANIM_DANCE1,
	FROG_ANIM_DANCE2,
	FROG_ANIM_DANCE3,
	FROG_ANIM_DANCE4,
	FROG_ANIM_HANDSPRING,
	FROG_ANIM_VICTORY
};

TIMER eolTimer;


void CheckEOLLoopTrack()
{
#ifndef PC_DEMO
	if(IsSongPlaying())
	{
		if(eolTrackComplete == 0)
			eolTrackComplete = 1;
	}
	else if(eolTrackComplete == 1)
	{
		PrepareSong(AUDIOTRK_LEVELCOMPLETELOOP,YES);
		eolTrackComplete = 2;
	}
#endif
}


void RunLevelComplete()
{
	int i,dropped = FMul(dropSpeed,gameSpeed),w;
	SPRITEOVERLAY *coinOver;
	char tempChar;

	sprintf(oldBestStr,GAMESTRING(STR_RECORD),worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parName,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime/600)%600,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime/10)%60,((int)worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime)%10);
	drawLandscape = 0;

#ifdef PSX_VERSION
	if(saveInfo.saveFrame)
	{
		frog[0]->draw = 0;
		for(i = 0;i < numBabies;i++)
			babyList[i].baby->draw = 0;
		skipTextOverlaysSpecFX = YES;
		if(!fadingOut)
			ChooseLoadSave();
		return;
	}
	skipTextOverlaysSpecFX = NO;
#endif

	CheckEOLLoopTrack();
//	if(padData.digital[0] & PAD_CROSS)
//		StartLevelComplete();

	switch(levCompleteState)
	{
		case LEV_COMPLETE_DROPPING_FROGS:
			if(fadingOut)
				return;
			if(frog[0]->actor->position.vy > 0)
				frog[0]->actor->position.vy -= dropped;
			if(frog[0]->actor->position.vy <= 0)
			{
				frog[0]->actor->position.vy = 0;
				if((GetAnimNum(frog[0]) == froganimnum) && (ReachedEndOfAnim(frog[0])))
				{
					actorAnimate(frog[0]->actor,froganimnum2,NO,NO,froganimspeed2,NO);
					actorAnimate(frog[0]->actor,froganimnum3 + Random(4),NO,YES,froganimspeed3,NO);
					actorAnimate(frog[0]->actor,froganimnum4,YES,YES,froganimspeed4,NO);
				}
			}

			
			if(arcadeHud.timeOutText->draw == 0)
			{
				for(i = 0;i < numBabies;i++)
				{
					if(babyList[i].baby->actor->position.vy > 0)
					{
						babyList[i].baby->actor->position.vy -= dropped;

						if((babyList[i].baby->actor->position.vy <= babyanimy) && (babyList[i].baby->actor->position.vy + dropped > babyanimy))
						{
							actorAnimate(babyList[i].baby->actor,BABY_ANIM_COLLECT,NO,NO,-babyanimspeed,NO);
							actorAnimate(babyList[i].baby->actor,BABY_ANIM_HOP,YES,YES,babyanimspeed2,NO);
						}
					}
				}
			}

			break;

		case LEV_COMPLETE_COUNTING_COINS:
			if(coinText)
				coinText->speed = 4096*75;
			if(coinIcon)
				coinIcon->speed = 4096*75;
			if(extraText)
				extraText->speed = 4096*75;
			if(extraIcon)
				extraIcon->speed = 4096*75;
			arcadeHud.coinsOver->speed = 4096*75;


			if(coinText->xPos == coinText->xPosTo)
			{
				if(player[0].numSpawn != garibList.maxCoins)
				{
					if(coinsMissed < garibList.maxCoins - player[0].numSpawn)
					{
						if(padData.digital[0] & PAD_CROSS)
							coinCounter += 10;
						else
							coinCounter += actFrameCount-lastActFrameCount;
			
						if(coinCounter > 10)
						{
							coinsMissed++;
							sprintf(coinStr,GAMESTRING(STR_MISSED_COINS),coinsMissed);//,garibList.maxCoins - player[0].numSpawn);
							coinCounter = 0;
							PlaySample(genSfx[GEN_COLLECT_COIN],NULL,0,SAMPLE_VOLUME,-1);
							
							if(cheatCombos[CHEAT_MAD_GARIBS].state)
								coinOver = CreateAndAddSpriteOverlay(arcadeHud.coinsOver->xPos,arcadeHud.coinsOver->yPos,"RGARIB01",205,273,0xff,0);
							else
								coinOver = CreateAndAddSpriteOverlay(arcadeHud.coinsOver->xPos,arcadeHud.coinsOver->yPos,"SCOIN0001",205,273,0xff,0);
							coinOver->xPosTo += 10000;
							coinOver->speed = 4096*50;
						}
					}
					else
					{
						arcadeHud.coinsOver->draw = 0;
						if(eolTimer.time == 0)
							GTInit(&eolTimer,2);
					}
				}
				else if(eolTimer.time == 0)
					GTInit(&eolTimer,4);
			}

			if(((extraText) && (extraText->xPos == extraText->xPosTo)) || ((extraText == NULL) && (coinText->xPos == coinText->xPosTo)))
			{
				if(eolTimer.time)
				{
					GTUpdate(&eolTimer,-1);
					if((!eolTimer.time) || (padData.debounce[0] & PAD_CROSS))
					{
						eolTimer.time = 0;
						if(train)
							levCompleteState = LEV_COMPLETE_MENU;
						else
						{
							levCompleteState = LEV_COMPLETE_SLIDING_TROPHY;
							GTInit(&eolTimer,3);
							if(coinText)
							{
								coinText->speed = 4096*75;
								coinText->xPosTo = -4096;
							}
							if(coinIcon)
							{
								coinIcon->speed = 4096*75;
								coinIcon->xPosTo = -4096;
							}
							if(extraText)
							{
								extraText->speed = 4096*75;
								extraText->xPosTo = -4096;
							}
							if(extraIcon)
							{
								extraIcon->speed = 4096*75;
								extraIcon->xPosTo = -4096;
							}
						}
					}
				}
			}

			break;

		case LEV_COMPLETE_SLIDING_TROPHY:
			if(tText)
				tText->speed = 4096*75;
			if(bIcon)
				bIcon->speed = 4096*75;
			if(oldBestText)
				oldBestText->speed = 4096*75;

#ifndef PC_DEMO
			if(newBestText)
				newBestText->speed = 4096*75;
			if(nText)
			{
				nText->speed = 4096*75;

				if(nText->xPos == nText->xPosTo)
				{
					levCompleteState = LEV_COMPLETE_ENTER_NAME;
					textEntry = NAME_LENGTH;
				}
			}

			if((!nText) && ((tText == NULL) || (tText->xPosTo == tText->xPos)))
			{
				if(eolTimer.time)
				{
					GTUpdate(&eolTimer,-1)
					if((!eolTimer.time) || (padData.debounce[0] & PAD_CROSS))
					{
						levCompleteState = LEV_COMPLETE_MENU;
						eolTimer.time = 0;
					}
				}
			}
#else
			if(eolTimer.time)
			{
				GTUpdate(&eolTimer,-1)
				if((!eolTimer.time) || (padData.debounce[0] & PAD_CROSS))
				{
					levCompleteState = LEV_COMPLETE_MENU;
					eolTimer.time = 0;
				}
			}
#endif

			break;

		case LEV_COMPLETE_ENTER_NAME:
			if(nText)
			{
#ifdef PSX_VERSION
				PsxNameEntryFrame();	//sets texEntry to 0 when done
				for(i = 0;i < 5;i++)
				{
					if(((i != 0) || (cursPos > 0)) && ((i != 1) || (cursPos < 2)))
					{
						INC_ALPHA(enterNameButton[i],255);
					}
					else
					{
						DEC_ALPHA(enterNameButton[i]);
					}
					w = fontExtentWScaled(nText->font,nText->text,4096)*4;
					enterNameButton[1]->xPos = nText->xPosTo + w*2 + 40;
					enterNameButton[4]->xPos = nText->xPosTo + w*2 + 400;
					w = fontExtentWScaled(nText->font,GAMESTRING(STR_ENTER_NAME),4096)*8;
					enterNameButton[0]->xPos = nText->xPosTo + w - 200;
					tempChar = textString[cursPos];
					textString[cursPos] = 0;
					enterNameButton[2]->xPos = nText->xPosTo + w + 40 + fontExtentWScaled(nText->font,textString,4096)*8;
					enterNameButton[3]->xPos = nText->xPosTo + w + 40 + fontExtentWScaled(nText->font,textString,4096)*8;
					textString[cursPos] = tempChar;
				}
#endif
				if(textEntry == 0)
				{
					nText->draw = 0;
					nText = NULL;
					cOption = 0;

					if(grade==0)
					{
						strcpy(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parName,textString);
						worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime = timeForLevel;
//						SaveGame();
					}
					levCompleteState = LEV_COMPLETE_MENU;
#ifdef PSX_VERSION
					for(i = 0;i < 5;i++)
					{
						enterNameButton[i]->draw = 0;
					}
#endif
				}
				sprintf(currentName,"%s %s",GAMESTRING(STR_ENTER_NAME),textString);
			}
			break;

		case LEV_COMPLETE_MENU:
			if(oText[0])
				oText[0]->speed = 4096*75;
			if(oText[1])
				oText[1]->speed = 4096*75;

			if(oText[0]->xPos == oText[0]->xPosTo)
				LevelCompleteProcessController(0);

			for (i=0; i<2; i++)
			{
				if(oText[i])
				{
					oText[i]->r = 128;
					oText[i]->g = 128;
					oText[i]->b = 128;
				}
			}

			oText[cOption]->r = 127+((rsin(actFrameCount*4200)+4096)*64)/4096;
			oText[cOption]->g = 127+((rcos(actFrameCount*4300)+4096)*64)/4096;
			oText[cOption]->b = 127+((rcos(actFrameCount*4400)+4096)*64)/4096;
			break;

		case LEV_COMPLETE_FADING_OUT:
			if(fadingOut == 0)
			{
				if(keepFade == 0)
				{
					ScreenFade(255,0,30);
					keepFade = YES;
					break;
				}
				if(train)
				{
					FREE(train);
					train = 0;
				}

				FreeTiledBackdrop();
#ifdef PC_DEMO
				if (goFrontend || player[0].worldNum == WORLDID_HALLOWEEN)
				{
					gameState.mode = TEASERSCREEN_MODE;
					return;
				}
				else
				{
					player[0].worldNum = WORLDID_HALLOWEEN;
					player[0].levelNum = LEVELID_HALLOWEEN2;

					gameState.mode = INGAME_MODE;
				}
#else
				if (goFrontend)
				{
					gameState.mode = FRONTEND_MODE;
					player[0].worldNum = WORLDID_FRONTEND;
					player[0].levelNum = LEVELID_FRONTEND1;
					player[0].character = FROG_FROGGER;
				}
				else if ((gameState.single == STORY_MODE) && (arcadeHud.timeOutText->draw == 0))
				{
					gameState.mode = INGAME_MODE;

					if((player[0].worldNum == storySequence[NUM_STORY_LEVELS - 1].worldNum) && (player[0].levelNum == storySequence[NUM_STORY_LEVELS - 1].levelNum))
					{
						StartVideoPlayback(FMV_VICTORY);
						gameState.storySequenceLevel = 0;
						player[0].worldNum = WORLDID_FRONTEND;
						player[0].levelNum = LEVELID_FRONTEND1;
						gameState.mode = THEEND_MODE;
					}
					else
					{
						player[0].worldNum = storySequence[gameState.storySequenceLevel].worldNum;
						player[0].levelNum = storySequence[gameState.storySequenceLevel].levelNum;
					}
				}
				else
				{
					// todo: place Frogger 
					gameState.mode = INGAME_MODE;
					restartingLevel = TRUE;
				}
#endif
//				FreeAllLists();
				frameCount = 0;

				if(gameState.mode != THEEND_MODE)
					InitLevel(player[0].worldNum,player[0].levelNum);

				showEndLevelScreen = 1; // Normal level progression is default
				frameCount = 0;		
				return;
			}
			break;
	}
#ifdef PSX_VERSION
		if((levCompleteState == LEV_COMPLETE_FADING_OUT) && (useMemCard) && ((grade == 0) || (moreCoins) || (levelOpened)))
		{
			frog[0]->draw = 0;
			for(i = 0;i < numBabies;i++)
				babyList[i].baby->draw = 0;
			skipTextOverlaysSpecFX = YES;
			return;
		}
#endif

	if(bIcon)
	{
		if(grade < 2)
		{
		
			for(i = 0;i < MAX_SPARKLES;i++)
			{
				if(sparkles[i])
				{
					if(sparkles[i]->draw == 0)
					{
						sparkles[i]->xPos = bIcon->xPos - sparkles[i]->width/2 + Random(bIcon->width);
						sparkles[i]->yPos = bIcon->yPos - sparkles[i]->height/2 + Random(bIcon->height);
						sparkles[i]->r = 64 + Random(192);
						sparkles[i]->g = 64 + Random(192);
						sparkles[i]->b = 64 + Random(192);
						sparkles[i]->a = 200 + Random(56);
						sparkles[i]->draw = 1;
					}
					else
					{
						if(Random(50) < 20)
							DEC_ALPHA(sparkles[i]);
					}
				}
			}
		}
	}

	for(i = 0;i < numBabies;i++)
	{
		babyList[i].baby->draw = 1;
		babyList[i].baby->actor->qRot.x = 0;
		babyList[i].baby->actor->qRot.y = babyFrogY[i];
		babyList[i].baby->actor->qRot.z = 0;
		babyList[i].baby->actor->qRot.w = babyFrogW[i];

		if(babyList[i].baby->actor->position.vy <= 0)
		{
			if((i == 4) && (levCompleteState == LEV_COMPLETE_DROPPING_FROGS))
			{
				if(player[0].worldNum == WORLDID_SUPERRETRO)
				{
					levCompleteState = LEV_COMPLETE_SLIDING_TROPHY;
					GTInit(&eolTimer,3);
				}
				else
				{
					levCompleteState = LEV_COMPLETE_COUNTING_COINS;
					GTInit(&eolTimer,0);
				}
			}
			babyList[i].baby->actor->position.vy = 0;

			if(GetAnimNum(babyList[i].baby) == BABY_ANIM_HOP)
			{
				if(GetAnimLoop(babyList[i].baby))
				{
					if(Random(1000) <= 20)
					{
						SetAnimLoop(babyList[i].baby,0);
						actorAnimate(babyList[i].baby->actor,BABY_ANIM_WAVE + Random(7),NO,YES,100,NO);
						actorAnimate(babyList[i].baby->actor,BABY_ANIM_HOP,YES,YES,babyanimspeed2,NO);
					}
				}
			}
		}
	}
	frog[0]->draw = 1;
	if(frog[0]->actor->position.vy <= 0)
	{
		if((arcadeHud.timeOutText->draw) && (levCompleteState == LEV_COMPLETE_DROPPING_FROGS))
		{
			levCompleteState = LEV_COMPLETE_MENU;
			actorAnimate(frog[0]->actor,froganimnum2,NO,NO,froganimspeed2,NO);
			actorAnimate(frog[0]->actor,froganimnum4,YES,YES,froganimspeed4,NO);
		}
		if((numBabies == 0) && (levCompleteState == LEV_COMPLETE_DROPPING_FROGS))
		{
			levCompleteState = LEV_COMPLETE_COUNTING_COINS;
			GTInit(&eolTimer,0);
			// ds - do animations from above somewhere
			actorAnimate(frog[0]->actor,froganimnum2,NO,NO,froganimspeed2,NO);
			actorAnimate(frog[0]->actor,froganimnum3 + Random(4),NO,YES,froganimspeed3,NO);
			actorAnimate(frog[0]->actor,froganimnum4,YES,YES,froganimspeed4,NO);
		}
		
		if((arcadeHud.timeOutText->draw == 0) && (GetAnimNum(frog[0]) == FROG_ANIM_BREATHE))
		{
			if(GetAnimLoop(frog[0]))
			{
				if(Random(1000) <= 20)
				{
					SetAnimLoop(frog[0],0);
					switch(Random(3))
					{
						case 0:
							PlayVoice(0,"frogokay");
							break;
						case 1:
							PlayVoice(0,"frogletsgo");
							break;
						case 2:
							PlayVoice(0,"frogcroak");
							break;
					}
					actorAnimate(frog[0]->actor,celebrateAnim[Random(NUM_CELEBRATE_ANIMS)],NO,YES,70,NO);
					actorAnimate(frog[0]->actor,FROG_ANIM_BREATHE,YES,YES,75,NO);
				}
			}
		}
	}
}


int GetAnimNum(ACTOR2 *actor)
{
#ifdef PSX_VERSION
	return actor->actor->animation.currentAnimation;
#elif PC_VERSION
 	return((MDX_ACTOR *)actor->actor->actualActor)->animation->currentAnimation;
#endif
}

int GetAnimLoop(ACTOR2 *actor)
{
#ifdef PSX_VERSION
	return actor->actor->animation.loopAnimation;
#elif PC_VERSION
 	return((MDX_ACTOR *)actor->actor->actualActor)->animation->loopAnimation;
#endif
}

void SetAnimLoop(ACTOR2 *actor,int loop)
{
#ifdef PSX_VERSION
	actor->actor->animation.loopAnimation = loop;
#elif PC_VERSION
 	((MDX_ACTOR *)actor->actor->actualActor)->animation->loopAnimation = loop;
#endif
}

int ReachedEndOfAnim(ACTOR2 *actor)
{
#ifdef PSX_VERSION
	return actor->actor->animation.reachedEndOfAnimation;
#elif PC_VERSION
 	return ((MDX_ACTOR *)actor->actor->actualActor)->animation->reachedEndOfAnimation;
#endif
}
