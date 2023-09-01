/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: hud.c
	Programmer	: James Healey
	Date		: 31/03/99

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2

#include <isltex.h>
#include <islfont.h>
#include <islutil.h>
#include "ultra64.h"
#include "hud.h"
#include "frogger.h"
#include "game.h"
#include "textover.h"
#include "layout.h"
#include "overlays.h"
#include "main.h"
#include "multi.h"
#include "collect.h"
#include "enemies.h"
#include "babyfrog.h"
#include "lang.h"
#include "frontend.h"
#include "story.h"
#include "menus.h"

#ifdef PSX_VERSION
#include "textures.h"
#include "audio.h"
#else
#include <stdio.h>
#include <Main.h>
#include <mdx.h>
#include <pcaudio.h>
#include <pcmisc.h>
#endif

int handAngle;
extern TIMER endLevelTimer;
char scoreText[32]	= "10000000";
char livesText[8]	= "xxxx";
char timeText[32]	= "00:00      ";
char garibText[8], creditText[8];

char timeTemp[6];
char goStr[64];

char countdownTimer	= 1;
char displayFullScreenPoly = 0;
long coinZoomX,coinZoomY;

TIMER goTimer;

TEXTOVERLAY	*babySavedText;


TEXTOVERLAY *keyCollected;

TEXTOVERLAY *controllerText;
TEXTOVERLAY *continueText;
char pauseTitleString[32];
TEXTOVERLAY *xselectText, *pauseTitleText;
TEXTOVERLAY *restartText;
TEXTOVERLAY *quitText;
TEXTOVERLAY *garibCount, *creditCount;
TEXTOVERLAY *time;
//TEXTOVERLAY *build;
TEXTOVERLAY *posText;
TEXTOVERLAY *levelnameText;

TEXTOVERLAY *wholeKeyText = NULL;

char levelString[] = "Levelname Goes in here";
char posString[] = "-----------------------";

extern long numSprites;

///////////////////////////////////////////////////////////////////////////////////////

ARCADE_HUD arcadeHud;
MULTI_HUD multiHud;
char timeStringMin[8]	= "00:";
char timeStringSec[8]	= "00:";
char timeStringHSec[8]	= "00";

char mStartString[32];

char speedrunTimerString[16] = "00:00:00";

char collectString[32] = "";
char coinsText[5] = "00";
//char coinsText2[5] = "00";

char penalString[4][8] = {"00","00","00","00"};
char winsString[4][8] = {"0","0","0","0"};

char countdownString[64] = "00";

//TEXTOVERLAY   *polysText;
char polyString[256] = "";

int timeBarWidth = 1600;
int timeBarStep = 16;
char timeBarStr[16];

// -----------------------------------------------
int storeFrameCount = 0;

void InitArcadeHUD(void)
{
	int i;


//	if(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime < worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime)
//		worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime;

	arcadeHud.timeBar = CreateAndAddSpriteOverlay(1200,3600,NULL,0,100,255,0);
	arcadeHud.timeBar->num = 1;
	arcadeHud.timeBak = CreateAndAddSpriteOverlay(1200-8,3600-17,NULL,timeBarWidth+24,100+34,255,SPRITE_SUBTRACTIVE);
	arcadeHud.timeBak->a = 254;
	arcadeHud.timeBak->r = 128;
	arcadeHud.timeBak->g = 128;
	arcadeHud.timeBak->b = 128;
	sprintf(timeBarStr,"%d:%02d.%d0",worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime/600,(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime/10)%60,(worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime)%10);
	arcadeHud.timeBarText = CreateAndAddTextOverlay(2048,3300,timeBarStr,YES,255,font,TEXTOVERLAY_SHADOW);
	arcadeHud.speedrunTimer = CreateAndAddTextOverlay(0, 0, speedrunTimerString, NO, 255, fontSmall, TEXTOVERLAY_NORMAL);
	arcadeHud.speedrunTimer->b = 0;
	arcadeHud.speedrunTimer->draw = showIGT && ((gameState.single == STORY_MODE));

	if((gameState.mode == FRONTEND_MODE) || (gameState.mode == DEMO_MODE))
	{
		GTInit(&goTimer,0);
	}
	else
	{
		GTInit(&goTimer,5);
	}

	if((gameState.difficulty != DIFFICULTY_HARD) || (player[0].worldNum == WORLDID_FRONTEND))
	{
		arcadeHud.timeBar->draw = 0;
		arcadeHud.timeBak->draw = 0;
		arcadeHud.timeBarText->draw = 0;
	}

#ifdef PSX_VERSION
	arcadeHud.livesOver =		CreateAndAddSpriteOverlay(100,3132,frogPool[player[0].character].icon,4096,546,0xff,0);

	if( gameState.single != STORY_MODE )
	{
		arcadeHud.timeFaceOver = CreateAndAddSpriteOverlay(3300,2932,"FROGWATCH01",4096,546,0xff,0);
		arcadeHud.timeFaceOver->num = 0;
		arcadeHud.timeHandOver = CreateAndAddSpriteOverlay(3300,3000,"CLK_HAND",600,700,0xff,0);
		arcadeHud.timeHandOver->num = 1;
		arcadeHud.timeHeadOver = CreateAndAddSpriteOverlay(3300,3000,"CLK_HEAD",600,700,0xff,0);
		arcadeHud.timeHeadOver->num = 2;

		arcadeHud.timeTextMin =		CreateAndAddTextOverlay(2800,3600,timeStringMin,NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		arcadeHud.timeTextSec =		CreateAndAddTextOverlay(2800+200,3600,timeStringSec,NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		arcadeHud.timeTextHSec =	CreateAndAddTextOverlay(2800+450,3600,timeStringHSec,NO,255,fontSmall,TEXTOVERLAY_SHADOW);
		arcadeHud.timeTextHSec->scale = 3072;
	}

#ifdef E3_DEMO
	arcadeHud.helpOver =		CreateAndAddSpriteOverlay(3370,1200,"HELPOVER",4096,(128*4096)/480,180,0);
#endif

	arcadeHud.autoHopOver =		CreateAndAddSpriteOverlay( 200, 2900, "AHOVER", 400, 400, 0, 0 );
	arcadeHud.autoHopOver->draw = 0;
	arcadeHud.quickHopOver =	CreateAndAddSpriteOverlay( 650, 2900, "QHOVER", 400, 400, 0, 0 );
	arcadeHud.quickHopOver->draw = 0;

	arcadeHud.coinsBack =		CreateAndAddSpriteOverlay(3400,300,"HUD_BGR",4096,460,0xff,0);
	arcadeHud.livesText =		CreateAndAddTextOverlay(370,3550,livesText,NO,255,0,TEXTOVERLAY_SHADOW);
	if(cheatCombos[CHEAT_MAD_GARIBS].state)
		arcadeHud.coinsOver =		CreateAndAddSpriteOverlay(3524,290,"RGARIB01",205,273,0xff,0);
	else
		arcadeHud.coinsOver =		CreateAndAddSpriteOverlay(3524,290,"SCOIN0001",205,273,0xff,0);
	arcadeHud.coinsOver->num = 1;
	if(cheatCombos[CHEAT_MAD_GARIBS].state)
		arcadeHud.coinZoom =		CreateAndAddSpriteOverlay(3524,290,"RGARIB01",205,273,0xff,0);
	else
		arcadeHud.coinZoom =		CreateAndAddSpriteOverlay(3524,290,"SCOIN0001",205,273,0xff,0);
	arcadeHud.coinZoom->draw = 0;
	arcadeHud.coinZoom->num = 1;

	//CreateTextureAnimation ( "SCOIN000", arcadeHud.coinsOver->tex, 8 );

	for ( i = 0; i < numBabies; i++ )
	{
		arcadeHud.babiesBack [ i ] = CreateAndAddSpriteOverlay ( ( 200 + ( i * 200 ) ), 310, "HUD_BGR",(4096*32)/640,(4096*32)/480,0xff,0);
		arcadeHud.babiesBack[i]->r = arcadeHud.babiesBack[i]->g = arcadeHud.babiesBack[i]->b = 64;
	}



#else

	
	
	arcadeHud.livesOver =		CreateAndAddSpriteOverlay(100,3332,frogPool[player[0].character].icon,410,546,0xff,0);

	if( gameState.single != STORY_MODE )
	{
		arcadeHud.timeFaceOver = CreateAndAddSpriteOverlay(3600,3432,"CLK_FACE",410,546,0xff,0);
		arcadeHud.timeFaceOver->num = 0;
		arcadeHud.timeHandOver = CreateAndAddSpriteOverlay(3600,3432,"CLK_HAND",410,546,0xff,0);
		arcadeHud.timeHandOver->num = 1;
		arcadeHud.timeHeadOver = CreateAndAddSpriteOverlay(3600,3432,"CLK_HEAD",410,546,0xff,0);
		arcadeHud.timeHeadOver->num = 2;

		arcadeHud.timeTextMin =		CreateAndAddTextOverlay(3100,3700,timeStringMin,NO,255,0,0);
		arcadeHud.timeTextSec =		CreateAndAddTextOverlay(3100+200,3700,timeStringSec,NO,255,0,0);
		arcadeHud.timeTextHSec =	CreateAndAddTextOverlay(3100+450,3700,timeStringHSec,NO,255,0,0);
		arcadeHud.timeTextHSec->scale = 3072;
	}

#ifdef E3_DEMO
	arcadeHud.helpOver =		CreateAndAddSpriteOverlay(3424,2130,"HELPOVER",(128*4096)/640,(128*4096)/480,180,0);
#endif

	arcadeHud.autoHopOver =		CreateAndAddSpriteOverlay( 100, 3200, "AHOVER", 180, 180, 0, 0 );
	arcadeHud.autoHopOver->draw = 0;
	arcadeHud.quickHopOver =	CreateAndAddSpriteOverlay( 350, 3200, "QHOVER", 180, 180, 0, 0 );
	arcadeHud.quickHopOver->draw = 0;

	arcadeHud.coinsBack =		CreateAndAddSpriteOverlay(3520,130,"HUD_BGR",450,460,0xff,0);
	arcadeHud.coinsBack->num = 0;
	arcadeHud.livesText =		CreateAndAddTextOverlay(520,3650,livesText,NO,255,0,TEXTOVERLAY_SHADOW);
	if(cheatCombos[CHEAT_MAD_GARIBS].state)
		arcadeHud.coinsOver =		CreateAndAddSpriteOverlay(3624,130,"RGARIB01",205,273,0xff,0);
	else
		arcadeHud.coinsOver =		CreateAndAddSpriteOverlay(3624,130,"SCOIN0001",205,273,0xff,0);
	arcadeHud.coinsOver->num = 1;
	if(cheatCombos[CHEAT_MAD_GARIBS].state)
		arcadeHud.coinZoom =		CreateAndAddSpriteOverlay(3624,130,"RGARIB01",205,273,0xff,0);
	else
		arcadeHud.coinZoom =		CreateAndAddSpriteOverlay(3624,130,"SCOIN0001",205,273,0xff,0);
	arcadeHud.coinZoom->draw = 0;
	arcadeHud.coinZoom->num = 1;

	for(i=0; i<numBabies; i++)
	{
		arcadeHud.babiesBack[i] = CreateAndAddSpriteOverlay( (200+(i*200)),160,"HUD_BGR",(4096*32)/640,(4096*32)/480,0xff,0);
		arcadeHud.babiesBack[i]->r = arcadeHud.babiesBack[i]->g = arcadeHud.babiesBack[i]->b = 64;
	}
#endif

	

	arcadeHud.collectText =		CreateAndAddTextOverlay(4096+2048, 2048, collectString,YES, 255, font, TEXTOVERLAY_SHADOW );
	arcadeHud.collectText->b = 0;
	arcadeHud.collectText->g = 128;
	arcadeHud.collectText->draw = 1;
	GTInit(&pauseTimer,0);
	pauseFlag = 0;


	if(((gameState.difficulty == DIFFICULTY_HARD) || (gameState.single == ARCADE_MODE)) && (player[0].worldNum != WORLDID_FRONTEND))
  		arcadeHud.goText = CreateAndAddTextOverlay(2048,2048,goStr,YES,255,0,TEXTOVERLAY_SHADOW);
	else
	{
		if((player[0].worldNum == storySequence[NUM_STORY_LEVELS - 1].worldNum) && (player[0].levelNum == storySequence[NUM_STORY_LEVELS - 1].levelNum))
			arcadeHud.goText = CreateAndAddTextOverlay(2048,2048, "",YES,255,0,0);
		else
			arcadeHud.goText = CreateAndAddTextOverlay(2048,2048, GAMESTRING(STR_COLLECTBABIES),YES,255,0,TEXTOVERLAY_SHADOW);
	}
	
// allow a few extra pixels height on the overlays for overscan on the PSX (and DC?)

	//arcadeHud.backLeftExtra =	CreateAndAddSpriteOverlay(0,3600,"PRC_WATRD1",800,560,0xff,SPRITE_ADDITIVE);
	//arcadeHud.backLeft =		CreateAndAddSpriteOverlay(0,3600,"WBACK2",800,560,0x80,0);

	//arcadeHud.backRightExtra =	CreateAndAddSpriteOverlay(3000,3600,"PRC_WATRT1",1096,560,0xff,SPRITE_ADDITIVE);
	//arcadeHud.backRight =		CreateAndAddSpriteOverlay(3000,3600,"WBACK2",1096,560,0x80,0);

	//arcadeHud.backCentre =		CreateAndAddSpriteOverlay(800,3800,"WBACK2",4096-800-1096,360,0x80,0);

#ifdef PSX_VERSION
//	arcadeHud.coinsText2	=	CreateAndAddTextOverlay(3350,500,coinsText2,NO,255,fontSmall,0);
	arcadeHud.coinsText		=	CreateAndAddTextOverlay(3580,570,coinsText,NO,255,font,TEXTOVERLAY_SHADOW);
#else
//	arcadeHud.coinsText2	=	CreateAndAddTextOverlay(3450,350,coinsText2,NO,255,fontSmall,0);
	arcadeHud.coinsText		=	CreateAndAddTextOverlay(3680,370,coinsText,NO,255,font,TEXTOVERLAY_SHADOW);
#endif

//	arcadeHud.coinsText->r = 255;
//	arcadeHud.coinsText->g = 0;
//	arcadeHud.coinsText->b = 255;

//	if (gameState.mode == INGAME_MODE)
//		polysText = CreateAndAddTextOverlay(200,600,polyString,NO,255,fontSmall,0);

	arcadeHud.timeOutText = CreateAndAddTextOverlay(2048,1900,GAMESTRING(STR_OUTOFTIME),YES,255,0,TEXTOVERLAY_SHADOW);
	arcadeHud.timeOutText->r = 0xff;
	arcadeHud.timeOutText->g = 0;
	arcadeHud.timeOutText->b = 0;
	arcadeHud.timeOutText->a = 0;
	DisableTextOverlay(arcadeHud.timeOutText);

	arcadeHud.timedOut = 0;

	if(((gameState.difficulty != DIFFICULTY_HARD) && (gameState.single != ARCADE_MODE)) || (player[0].worldNum == WORLDID_FRONTEND))
	{
		for (i=0; i<MAX_HUD_SPARKLES; i++)
		{
			arcadeHud.goSparkles[i] = CreateAndAddSpriteOverlay(900-Random(1800)+2048,1900+Random(350),"FLASH2",100,100,0xff,SPRITE_ADDITIVE);
			arcadeHud.goSparkles[i]->num = 1;
			if((player[0].worldNum == storySequence[NUM_STORY_LEVELS - 1].worldNum) && (player[0].levelNum == storySequence[NUM_STORY_LEVELS - 1].levelNum))
				arcadeHud.goSparkles[i]->draw = 0;
		}
	}
	if(gameState.mode == FRONTEND_MODE)
	{
		DisableHUD();
	}
	if(player[0].worldNum == WORLDID_SUPERRETRO)
	{
		arcadeHud.coinsBack->draw = 0;
		arcadeHud.coinsOver->draw = 0;
		arcadeHud.coinsText->draw = 0;
//		arcadeHud.coinsText2->draw = 0;
	}
}

long xPos_multi[4] = {64, 4096-1088, 4096-1088, 64};
long yPos_multi[4] = {80, 80, 3840-800, 3840-800};
long spSpeeds[20] = {8,3,6,7,8,3,4,6,8,7,9,6,3,9,8,6,3,8,7,6};

extern TEXTOVERLAY *menuText[2];


void InitMultiHUD()
{
	int i,j;
	char overlayName[32];

	switch( multiplayerMode )
	{
	case MULTIMODE_BATTLE:
		overlayName[0] = 0;
		multiHud.timeTextMin = NULL;
		multiHud.timeTextSec = NULL;
		multiHud.timeTextHSec = NULL;
		break;
	case MULTIMODE_COLLECT:
		strcpy(overlayName,"00BABYIC");
		multiHud.timeTextMin = NULL;
		multiHud.timeTextSec = NULL;
		multiHud.timeTextHSec = NULL;
		break;
	case MULTIMODE_RACE:
		strcpy(overlayName,"HUD_PENALTY");
		multiHud.timeTextMin = CreateAndAddTextOverlay(1536-102,160+64,timeStringMin,NO,255,font,TEXTOVERLAY_SHADOW);
		multiHud.timeTextSec = CreateAndAddTextOverlay(1536+410-102,160+64,timeStringSec,NO,255,font,TEXTOVERLAY_SHADOW);
		multiHud.timeTextHSec = CreateAndAddTextOverlay(1536+410+410-102,160+64,timeStringHSec,NO,255,font,TEXTOVERLAY_SHADOW);
		multiHud.timeTextHSec->scale = 3072;
		break;
	}

	if(gameState.multi == MULTIREMOTE)
	{
		menuText[0] = CreateAndAddTextOverlay(2048,2400,GAMESTRING(STR_QUIT),YES,0,font,TEXTOVERLAY_SHADOW);
		menuText[1] = NULL;
	}
	else
	{
		menuText[0] = CreateAndAddTextOverlay(2048,2400,GAMESTRING(STR_RESTARTLEVEL),YES,0,font,TEXTOVERLAY_SHADOW);
		menuText[1] = CreateAndAddTextOverlay(2048,2700,GAMESTRING(STR_QUIT),YES,0,font,TEXTOVERLAY_SHADOW);
		menuText[1]->draw = 0;
	}
	menuText[0]->draw = 0;	

	multiHud.centreText = CreateAndAddTextOverlay(2048,900,countdownString,YES,255,font,TEXTOVERLAY_SHADOW);

	for(i = 0;i < NUM_FROGS;i++)
	{
		multiHud.penaliseText[i] = CreateAndAddTextOverlay(backTextX[i],backTextY[i],penalString[i],NO,255,font,TEXTOVERLAY_SHADOW);
		if(overlayName[0])
		{
#ifdef PSX_VERSION
			multiHud.penalOver[i] = CreateAndAddSpriteOverlay(backTextX[i]-256-64,backTextY[i],overlayName,4096,256,255,0);
#else
			multiHud.penalOver[i] = CreateAndAddSpriteOverlay(backTextX[i]-256-64,backTextY[i],overlayName,256,256,255,0);
#endif
			if(multiplayerMode == MULTIMODE_COLLECT)
			{
				multiHud.penalOver[i]->r = 60;
				multiHud.penalOver[i]->g = 255;
				multiHud.penalOver[i]->b = 60;
			}
		}
		else
			multiHud.penalOver[i] = NULL;
		multiHud.backChars[i] = CreateAndAddSpriteOverlay(backCharsX[i],backCharsY[i],frogPool[player[i].character].icon,512,512,0xff,0);
		for(j = 0;j < 3;j++)
		{
			if((i == 0) || (i == 3))
				multiHud.trophies[i][j] = CreateAndAddSpriteOverlay(backWinsTextX[i] + j*220,backWinsTextY[i],"TRPHYGOLD",200,200,255,0);
			else
				multiHud.trophies[i][j] = CreateAndAddSpriteOverlay(backWinsTextX[i] - j*220,backWinsTextY[i],"TRPHYGOLD",200,200,255,0);
			multiHud.trophies[i][j]->draw = 0;
		}
	}
}

void DisableMultiHUD( )
{
	int i,j;

	if(multiplayerMode == MULTIMODE_RACE)
	{
		DisableTextOverlay( multiHud.timeTextMin );
		DisableTextOverlay( multiHud.timeTextSec );
		DisableTextOverlay( multiHud.timeTextHSec );
	}
	DisableTextOverlay( multiHud.centreText );

	for (i = 0; i<NUM_FROGS; i++)
	{
		DisableSpriteOverlay( multiHud.backChars[i] );
		if(multiHud.penalOver[i])
			DisableSpriteOverlay( multiHud.penalOver[i] );
		DisableTextOverlay( multiHud.penaliseText[i] );
		for(j = 0;j < 3;j++)
			DisableSpriteOverlay(multiHud.trophies[i][j]);
	}
}

void EnableMultiHUD( )
{
	int i,j;

	if(multiplayerMode == MULTIMODE_RACE)
	{
		EnableTextOverlay( multiHud.timeTextMin );
		EnableTextOverlay( multiHud.timeTextSec );
		EnableTextOverlay( multiHud.timeTextHSec );
	}

	if( multiTimer.time > 1)
		EnableTextOverlay( multiHud.centreText );

	for (i = 0; i<NUM_FROGS; i++)
	{
		EnableSpriteOverlay( multiHud.backChars[i] );
		if(multiHud.penalOver[i])
			EnableSpriteOverlay( multiHud.penalOver[i] );
		EnableTextOverlay( multiHud.penaliseText[i] );
		multiHud.penaliseText[i]->a = 255;
		multiHud.penaliseText[i]->xPos = multiHud.penaliseText[i]->xPosTo = backTextX[i];
		multiHud.penaliseText[i]->yPos = multiHud.penaliseText[i]->yPosTo = backTextY[i];
		for(j = 0;j < mpl[i].wins;j++)
			EnableSpriteOverlay(multiHud.trophies[i][j]);
	}
}

void InitHUD(void)
{
	storeFrameCount = 0;
	handAngle = 0;
	if (gameState.multi == SINGLEPLAYER)
		InitArcadeHUD();		
	else
		InitMultiHUD();	
}

void SparkleCoins(void)
{
	arcadeHud.sX = 1100;
	arcadeHud.sY = 3800;
	arcadeHud.sW = 640;
	arcadeHud.sH = 192;
	arcadeHud.sTime = actFrameCount + 30;
}

void SparkleBabies(void)
{
	arcadeHud.sX = (3000-(numBabies*192));
	arcadeHud.sY = 3864;
	arcadeHud.sW = (numBabies-1)*192;
	arcadeHud.sH = 128;
	arcadeHud.sTime = actFrameCount + 30;
}

void DisableHUD(void)
{
	int i;

	if( gameState.multi != SINGLEPLAYER )
	{
		DisableMultiHUD();
		return;
	}

	for (i=0; i<numBabies; i++)
		arcadeHud.babiesBack[i]->draw = 0;

	if(((gameState.difficulty != DIFFICULTY_HARD) && (gameState.single != ARCADE_MODE)) || (player[0].worldNum == WORLDID_FRONTEND))
	{
		for (i=0; i<MAX_HUD_SPARKLES; i++)
			arcadeHud.goSparkles[i]->draw = 0;
	}

	arcadeHud.coinsBack->draw = 0;
	arcadeHud.coinsOver->draw = 0;
	arcadeHud.goText->draw = arcadeHud.coinsBack->draw = 0; /*arcadeHud.helpOver->draw = */ 
	arcadeHud.livesOver->draw = arcadeHud.coinsOver->draw = arcadeHud.livesText->draw = arcadeHud.coinsText->draw = /*arcadeHud.coinsText2->draw =*/ 0;
	arcadeHud.collectText->draw = 0;
	arcadeHud.autoHopOver->draw = arcadeHud.quickHopOver->draw = 0;
	arcadeHud.speedrunTimer->draw = 0;
	if( gameState.single != STORY_MODE )
	{
		arcadeHud.timeHeadOver->draw = arcadeHud.timeHandOver->draw = arcadeHud.timeFaceOver->draw = 0;
		arcadeHud.timeTextMin->draw = arcadeHud.timeTextSec->draw = /*arcadeHud.timeOutText->draw = */arcadeHud.timeTextHSec->draw = 0;
	}

	for(i=0; i<numBabies; i++)
		babyIcons[i]->draw = 0;

	if((gameState.difficulty == DIFFICULTY_HARD) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		arcadeHud.timeBar->draw = arcadeHud.timeBak->draw = arcadeHud.timeBarText->draw = 0;
	}
}

void EnableHUD(void)
{
	int i;

	//if( screenshotEnable )
	//	return;

	if( gameState.multi != SINGLEPLAYER )
	{
		EnableMultiHUD();
		return;
	}
	
	for (i=0; i<numBabies; i++)
		arcadeHud.babiesBack[i]->draw = 1;
	
	if((((gameState.difficulty != DIFFICULTY_HARD) && (gameState.single != ARCADE_MODE)) || (player[0].worldNum == WORLDID_FRONTEND)) && (player[0].worldNum != storySequence[NUM_STORY_LEVELS - 1].worldNum) && (player[0].levelNum != storySequence[NUM_STORY_LEVELS - 1].levelNum))
//	if(((gameState.difficulty == DIFFICULTY_EASY) || (player[0].worldNum == WORLDID_FRONTEND)) && (player[0].worldNum != storySequence[NUM_STORY_LEVELS - 1].worldNum) && (player[0].levelNum != storySequence[NUM_STORY_LEVELS - 1].levelNum))
	{
		for (i=0; i<MAX_HUD_SPARKLES; i++)
			arcadeHud.goSparkles[i]->draw = 1;
	}

	arcadeHud.coinsBack->draw = 1;
	arcadeHud.coinsOver->draw = 1;
	arcadeHud.goText->draw = arcadeHud.coinsBack->draw = 1;	/*arcadeHud.helpOver->draw =*/
	arcadeHud.livesOver->draw = arcadeHud.coinsOver->draw = arcadeHud.livesText->draw = arcadeHud.coinsText->draw = /*arcadeHud.coinsText2->draw =*/ 1;
	arcadeHud.collectText->draw = 1;
	arcadeHud.autoHopOver->draw = arcadeHud.quickHopOver->draw = 1;

	if((gameState.single != STORY_MODE ) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		arcadeHud.timeHeadOver->draw = arcadeHud.timeHandOver->draw = arcadeHud.timeFaceOver->draw = 1;
		arcadeHud.timeTextMin->draw = arcadeHud.timeTextSec->draw = 1;
	}

	if((gameState.single == STORY_MODE) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		arcadeHud.speedrunTimer->draw = showIGT;
	}

	for(i=0; i<numBabies; i++)
		babyIcons[i]->draw = 1;
	if(player[0].worldNum == WORLDID_SUPERRETRO)
	{
		arcadeHud.coinsBack->draw = 0;
		arcadeHud.coinsOver->draw = 0;
		arcadeHud.coinsText->draw = 0;
//		arcadeHud.coinsText2->draw = 0;
	}
	if((gameState.difficulty == DIFFICULTY_HARD) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		arcadeHud.timeBar->draw = arcadeHud.timeBak->draw = 1;
		if(arcadeHud.timeBarText->a)
			arcadeHud.timeBarText->draw = 1;
	}
}

void UpDateMultiplayerInfo( void )
{
	long timeFrames=0,i,roundNum;

//	multiHud.centreText->a = 0xff;
	if( multiTimer.time>1)
	{
		if( multiTimer.time<5)
		{
			sprintf(countdownString,"%lu",multiTimer.time-1);
			multiHud.centreText->scale = 6144;
			switch (multiTimer.time)
			{
				case 4:
					multiHud.centreText->r = 0xff;
					multiHud.centreText->g = 0;
					multiHud.centreText->b = 0;
					break;
				case 3:
					multiHud.centreText->r = 0xff;
					multiHud.centreText->g = 180;
					multiHud.centreText->b = 30;
					break;
				case 2:
					multiHud.centreText->r = 0;
					multiHud.centreText->g = 0xff;
					multiHud.centreText->b = 0;
					break;
			}
		}
		else
		{
			roundNum = 1;
			for(i = 0;i < NUM_FROGS;i++)
				roundNum += mpl[i].wins;
			sprintf(countdownString, GAMESTRING(STR_ROUND_NUM),roundNum);
			multiHud.centreText->scale = 4096;
		}
		multiHud.centreText->draw = 1;
	}
	else
	{
		multiHud.centreText->r = multiHud.centreText->b = multiHud.centreText->g = 0xff;
		 			
//		multiHud.centreText->scale = 4096;
		if (multiTimer.time==1)
			sprintf(countdownString, GAMESTRING(STR_GO));
		else if(gameWinner == -1)
			multiHud.centreText->draw = 0;
	}
	
	switch( multiplayerMode )
	{
	case MULTIMODE_RACE:
		for (i=0; i<NUM_FROGS; i++)
		{
			sprintf(winsString[i],"%i",mpl[i].wins);
			sprintf(penalString[i],"%02i",((int)(mpl[i].penalty/60)));
			if (mpl[i].timer>timeFrames)
				timeFrames=mpl[i].timer;
		}
		sprintf(timeStringHSec,"%02i",((int)(timeFrames*100)/60)%100);
		sprintf(timeStringSec,"%02i:",((int)timeFrames/60)%60);
		sprintf(timeStringMin,"%2i:",((int)timeFrames/(60*60))%60);	

#ifdef PSX_VERSION
		multiHud.timeTextHSec->xPos = multiHud.timeTextSec->xPos + fontExtentWScaled(multiHud.timeTextSec->font,multiHud.timeTextSec->text,multiHud.timeTextSec->scale)*8 + 2*8;
		multiHud.timeTextMin->xPos = multiHud.timeTextSec->xPos - fontExtentWScaled(multiHud.timeTextMin->font,multiHud.timeTextMin->text,multiHud.timeTextMin->scale)*8 - 2*8;
#elif PC_VERSION
		multiHud.timeTextHSec->xPos = multiHud.timeTextSec->xPos + (float)(CalcStringWidth(multiHud.timeTextSec->text,(MDX_FONT *)multiHud.timeTextSec->font,((float)multiHud.timeTextSec->scale)/4096.0) + 2) * 6.4;
		multiHud.timeTextMin->xPos = multiHud.timeTextSec->xPos - (float)(CalcStringWidth(multiHud.timeTextMin->text,(MDX_FONT *)multiHud.timeTextMin->font,((float)multiHud.timeTextMin->scale)/4096.0) + 2) * 6.4;
#endif
		break;
	case MULTIMODE_BATTLE:
	case MULTIMODE_COLLECT:
		for (i=0; i<NUM_FROGS; i++)
		{
			sprintf(winsString[i],"%i",mpl[i].wins);
			if(multiplayerMode == MULTIMODE_BATTLE)
				sprintf(penalString[i],"",mpl[i].score);
			else
				sprintf(penalString[i],"%02i",mpl[i].score);
		}

		break;
	}
}

void UpDateOnScreenInfo ( void )
{
	long i;
	long xPos,yPos;
	long r,g,b,a;
	long timeFrames = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime * 6;
	long hardTimeFrames = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime * 6;
	long frameCheck = 0;
	int oldTime;
	int timeForLevel = (actFrameCount - storeFrameCount)/6;

	if (gameState.mode == INGAME_MODE)
	{
		long alpha = actFrameCount-100;

		if(arcadeHud.speedrunTimer->draw)
		{
									//   min  sec  milli
			sprintf(speedrunTimerString,"%02i:%02i.%i0",((int)timeForLevel/600)%600,((int)timeForLevel/10)%60,((int)timeForLevel)%10);
		}
		
		if(arcadeHud.coinZoom->draw)
		{
			SlideSpriteOverlayToPos(arcadeHud.coinZoom,arcadeHud.coinsOver->xPos,arcadeHud.coinsOver->yPos,coinZoomX,coinZoomY,20);
			if((arcadeHud.coinZoom->xPos == arcadeHud.coinsOver->xPos) && (arcadeHud.coinZoom->yPos == arcadeHud.coinsOver->yPos))
				arcadeHud.coinZoom->draw = 0;
		}

		if((gameState.difficulty == DIFFICULTY_HARD) && (player[0].worldNum != WORLDID_FRONTEND))
		{
			if(goTimer.time == 0)
			{
				arcadeHud.goText->draw = arcadeHud.goText->a = 0;
			}
		}
		else
		{
			if (actFrameCount>100)
			{
				alpha *= 3;
				if (alpha>255)
				{
					alpha = 255;
					arcadeHud.goText->draw = 0;
				}
				
				arcadeHud.goText->a = 0xff - alpha;
			}
		}

		// Show and update powerup icons
		if( player[0].autohop.time )
		{
			if( player[0].autohop.time > 3 )
			{
				INC_ALPHA( arcadeHud.autoHopOver, 0xff );
				arcadeHud.autoHopOver->draw = 1;
			}
			else
			{
				arcadeHud.autoHopOver->a = 127+((rsin(actFrameCount*4000)+4096)<<6)>>12;//Aabs(rsin( actFrameCount<<6 )>>4)-1;
			}
		}
		else
		{
			DEC_ALPHA( arcadeHud.autoHopOver );
			arcadeHud.autoHopOver->draw = arcadeHud.autoHopOver->a = 0;
		}

		if( player[0].quickhop.time )
		{
			if( player[0].quickhop.time > 5 )
			{
				INC_ALPHA( arcadeHud.quickHopOver, 0xff );
				arcadeHud.quickHopOver->draw = 1;
			}
			else
			{
				arcadeHud.quickHopOver->a = 127+((rsin(actFrameCount*4000)+4096)<<6)>>12;//Aabs(rsin( actFrameCount<<6 )>>4)-1;
			}
		}
		else
		{
			DEC_ALPHA( arcadeHud.quickHopOver );
			arcadeHud.quickHopOver->draw = 0;
		}

		// Sparklies
		if(((gameState.difficulty != DIFFICULTY_HARD) && (gameState.single != ARCADE_MODE)) || (player[0].worldNum == WORLDID_FRONTEND))
		{
			for (i=0; i<MAX_HUD_SPARKLES; i++)
			{	
				long a = arcadeHud.goSparkles[i]->a * 4096;
				a -= gameSpeed * spSpeeds[i] * 4;
				if (a<0)
				{
					if (arcadeHud.goText->a>100)
					{
						arcadeHud.goSparkles[i]->a = arcadeHud.goText->a;
						arcadeHud.goSparkles[i]->xPos = 1000-Random(2000)+2048;
						arcadeHud.goSparkles[i]->yPos = 1900+Random(350);
					}
					else
						arcadeHud.goSparkles[i]->draw = 0;

				}
				else
					arcadeHud.goSparkles[i]->a = a/4096;
			}
		}

	//	arcadeHud.goText->r = Random(0xff);
	//	arcadeHud.goText->g = Random(0xff);
	//	arcadeHud.goText->b = Random(0xff);

//#ifndef FINAL_MASTER
//#ifdef PC_VERSION
//		if( screenshotEnable )
//			polysText->draw = 0;
//		else
//		{
//#ifndef E3_DEMO
//			sprintf(polyString,"%lu %lu %lu",totalFacesDrawn,numObjectsDrawn,numObjectsTransformed);//,numPixelsDrawn/1000,(numPixelsDrawn*100/307200));
//#else
//			sprintf(polyString,"",totalFacesDrawn);//,numPixelsDrawn/1000,(numPixelsDrawn*100/307200));
//#endif

//		}
//		if (totalFacesDrawn>2000)
//			if (totalFacesDrawn>2500)
//				if (totalFacesDrawn>2800)
//					frameCheck = 3;
//				else
//					frameCheck = 2;
//			else
//				frameCheck = 1;

//		totalFacesDrawn = 0;
//		numObjectsDrawn = 0;
//		numObjectsTransformed = 0;

//#endif
//#endif		
		switch(frameCheck)
		{
			case 0:
				r=g=b=0xff;
				a = 100;
				break;
			case 1:
				r=255;
				g=180;
				b=0;
				a = 200;
				break;
			case 2:
				r=255;
				g=0;
				b=0;
				a = 255;
				break;
			case 3:
				r=255;
				g=0;
				b=0;
				a = 127+ ((4096+rsin(actFrameCount * 150)) / (4096 / 64));			
				break;
		}

//		polysText->r = r;
//		polysText->g = g;
//		polysText->b = b;
//		polysText->a = a;
		
	}

	timeFrames -=actFrameCount;
	hardTimeFrames -=actFrameCount;
	if(((gameState.difficulty == DIFFICULTY_HARD) || (gameState.single == ARCADE_MODE)) && (player[0].worldNum != WORLDID_FRONTEND) && (gameState.mode != DEMO_MODE))
	{
		if(goTimer.time)
		{
			if(goTimer.time > 1)
			{
				storeFrameCount = actFrameCount;
				player[0].canJump = 0;
			}
			oldTime = goTimer.time;
			GTUpdate2(&goTimer,-1);
			if(oldTime != goTimer.time)
			{
				switch(goTimer.time)
				{
					case 4:
					case 3:
					case 2:
						PlaySample( FindSample(utilStr2CRC("racehorn")), NULL, 0, SAMPLE_VOLUME, -1 );
						break;

					case 1:
						player[0].canJump = 1;
						PlaySample( FindSample(utilStr2CRC("racehorngo")), NULL, 0, SAMPLE_VOLUME, -1 );
						break;
				}
			}
			switch(goTimer.time)
			{
				case 3:
					sprintf(goStr,"2");
					break;
				case 2:
					sprintf(goStr,"1");
					break;
				case 1:
				case 0:
					sprintf(goStr,GAMESTRING(STR_GO));
					break;
				default:
					sprintf(goStr,"3");
					break;
			}
		}
		timeFrames += storeFrameCount;
		hardTimeFrames += storeFrameCount;
	}
	

	if((gameState.single != STORY_MODE ) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		static int lastSecs = 0;


		if (timeFrames<0)
		{
			timeFrames = 0;
			if (!arcadeHud.timedOut)
			{
				lastSecs = 0;
//				arcadeHud.timeOutText->a = 0xff;
//				arcadeHud.timeOutText->draw = 1;
				arcadeHud.timeTextHSec->draw = 0;

				PlaySample( FindSample(utilStr2CRC("alarm")), NULL, 0, SAMPLE_VOLUME, -1 );
			}
		
//			if( (arcadeHud.timeOutText->a<<12) > gameSpeed )
//				arcadeHud.timeOutText->a -= max(gameSpeed>>11, 1);

			arcadeHud.timedOut = 1;
			arcadeHud.timeTextMin->r = arcadeHud.timeTextSec->r = 255;
			arcadeHud.timeTextMin->g = arcadeHud.timeTextSec->g = 0;
			arcadeHud.timeTextMin->b = arcadeHud.timeTextSec->b = 0;
			arcadeHud.timeTextSec->a = 127+ ((4096+rsin(actFrameCount * 150)) / (4096 / 64));			
			arcadeHud.timeTextSec->scale = 4096;			
			arcadeHud.timeTextSec->xPos = 3500;
			arcadeHud.timeTextSec->yPos = 3700;

			sprintf(timeStringHSec,"%02i",((int)(timeFrames*100)/60)%100);
			sprintf(timeStringSec,"0",((int)timeFrames/60)%60);
			sprintf(timeStringMin,"",((int)timeFrames/(60*60))%60);	

			arcadeHud.timeHandOver->angle /= 2;
			arcadeHud.timeHeadOver->angle /= 2;
		}
		else
		{
			if(gameState.mode != DEMO_MODE)
				arcadeHud.timeTextHSec->draw = 1;
			if ((timeFrames/60)<10)
			{
				int secs;
				long value = (timeFrames % 60);
				long alpha = (value * 0xff)/60;
				long scale = 4096 + (4096 - ((value * 4096)/60));
				long move = (400 - ((value * 400)/60));
				arcadeHud.timeTextSec->scale = scale;

//				arcadeHud.timeTextSec->xPos = 3400 - move;
//				arcadeHud.timeTextSec->yPos = 3700 - move;
				
				arcadeHud.timeTextMin->r = arcadeHud.timeTextSec->r = 255;
				arcadeHud.timeTextMin->g = arcadeHud.timeTextSec->g = 0;
				arcadeHud.timeTextMin->b = arcadeHud.timeTextSec->b = 0;
				
				arcadeHud.timeTextSec->a = alpha;

				secs = ((int)timeFrames/60)%60;
				if( lastSecs != secs )
				{
					PlaySample( genSfx[GEN_CLOCKTICK], NULL, 0, SAMPLE_VOLUME, -1 );
					lastSecs = secs;
				}

				sprintf(timeStringHSec,"%02i",((int)(timeFrames*100)/60)%100);
				sprintf(timeStringSec,"%02i",secs);
				sprintf(timeStringMin,"",((int)timeFrames/(60*60))%60);	
			
			}
			else
			{
				sprintf(timeStringHSec,"%02i",((int)(timeFrames*100)/60)%100);
				sprintf(timeStringSec,"%02i",((int)timeFrames/60)%60);
				if ((((int)timeFrames/(60*60))%60)>0)
					sprintf(timeStringMin,"%2i:",((int)timeFrames/(60*60))%60);	
				else
					sprintf(timeStringMin,"",((int)timeFrames/(60*60))%60);			
			}

			if(goTimer.time <= 1)
			{
				handAngle += gameSpeed;
				if(handAngle > 4096*60)
					handAngle -= 4096*60;
				arcadeHud.timeHandOver->angle = handAngle/60;
//				if( arcadeHud.timeHandOver->angle > 4096 )
//					arcadeHud.timeHandOver->angle -= 4096;

				arcadeHud.timeHeadOver->angle = rsin(arcadeHud.timeHandOver->angle)>>2;
			}
			if(hardTimeFrames < 0)
				hardTimeFrames = 0;
		}
#ifdef PSX_VERSION
//		arcadeHud.timeTextHSec->xPos = arcadeHud.timeTextSec->xPos + fontExtentWScaled(arcadeHud.timeTextSec->font,arcadeHud.timeTextSec->text)*8 + 2*8;
		arcadeHud.timeTextMin->xPos = arcadeHud.timeTextSec->xPos - fontExtentWScaled(arcadeHud.timeTextMin->font,arcadeHud.timeTextMin->text,arcadeHud.timeTextMin->scale)*8 - 2*8;
#elif PC_VERSION
//		arcadeHud.timeTextHSec->xPos = arcadeHud.timeTextSec->xPos + (float)(CalcStringWidth(arcadeHud.timeTextSec->text,(MDX_FONT *)arcadeHud.timeTextSec->font,arcadeHud.timeTextSec->scale) + 2) / OVERLAY_X;
		arcadeHud.timeTextMin->xPos = arcadeHud.timeTextSec->xPos - (float)(CalcStringWidth(arcadeHud.timeTextMin->text,(MDX_FONT *)arcadeHud.timeTextMin->font,((float)arcadeHud.timeTextMin->scale)/4096.0) + 2) * 6.4;
#endif
	}	
	
	sprintf(livesText,"%lu",player[0].lives);	
	sprintf(coinsText,"%2i",player[0].numSpawn);
//	sprintf(coinsText2,"%2i",player[0].numSpawn);

	if((gameState.difficulty == DIFFICULTY_HARD) && (player[0].worldNum != WORLDID_FRONTEND))
	{
		if(goTimer.time > 1)
		{
			if(arcadeHud.timeBar->width < timeBarWidth - FMul(gameSpeed,timeBarStep))
				arcadeHud.timeBar->width += FMul(gameSpeed,timeBarStep);
			else
				arcadeHud.timeBar->width = timeBarWidth;
		}
		else
		{
			DEC_ALPHA(arcadeHud.timeBarText);
			hardTimeFrames += 6*worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime - worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].parTime*6;
			if(hardTimeFrames > 0)
				arcadeHud.timeBar->width = max(0,(hardTimeFrames*timeBarWidth)/(6*worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].difficultTime));
			else
			{
				player[0].canJump = 0;
				arcadeHud.timeOutText->a = 255;
				arcadeHud.timeOutText->draw = 1;
				GTInit(&endLevelTimer,4);
				SetTimeForLevel();
				PlaySample( FindSample(utilStr2CRC("alarm")), NULL, 0, SAMPLE_VOLUME, -1 );
				PrepareSong(AUDIOTRK_GAMEOVER,0);
			}
		}
	}
}


/*
void UpDateOnScreenInfo ( void )
{
	int i;
	static char tickTock = 0;
	static long lastCount = 0;

//	if( player[0].worldNum == WORLDID_SUPERRETRO ) // credits are lives in super retro
//	{
//		sprintf(livesText,"*%lu",player[0].numCredits);	
//		sprintf(scoreText,"%lu",player[0].score);
//	}
//	else
//	{
		sprintf(livesText,"%lu",player[0].lives);	
		sprintf(scoreText,"%lu",player[0].score);
		sprintf(garibText,"%lu",player[0].numSpawn);	
		sprintf(creditText,"%lu",player[0].numCredits);
//	}

	if(scoreTextOver->a && !(gameState.mode==LEVELCOMPLETE_MODE && modeTimer.time) )
	{
		scoreTextOver->a -= 8;
		if(scoreTextOver->a < 8)
			scoreTextOver->a = 0;
	}

	if(countdownTimer)
	{
		GTUpdate( &scoreTimer, -1 );

		if( !scoreTimer.time )
		{
			countdownTimer = 0;
//			PlaySample(GEN_TIME_OUT,0,0,0);
			sprintf(timeText,"NO BONUS !");
			if (gameState.multi==SINGLEPLAYER)
				timeTextOver->draw = 50;
		}

		if( (scoreTimer.time < 11) && !(scoreTimer.time%2) )
		{
			if( tickTock )
			{
//				PlaySample(GEN_CLOCK_TOCK,0,0,0);
				tickTock = 0;
			}
			else
			{
//				PlaySample(GEN_CLOCK_TICK,0,0,0);
				tickTock = 1;
			}
		}
								   
		if( scoreTimer.time ) sprintf(timeText,"%02lu",scoreTimer.time);
	}
	else
	{
		if(timeTextOver->draw)
			timeTextOver->draw-=gameSpeed;
		timeTextOver->a -= 16*gameSpeed;
		timeTextOver->a &= 255;
	}

	i = 3;
	while(i--)
	{
		if ( frog[0] )
		{
			if(player[0].healthPoints <= i)
				sprHeart[i]->a = 64;
			else
				sprHeart[i]->a = 255;
		}
	}
}
*/

/*	--------------------------------------------------------------------------------
	Function		: UpdateScore
	Purpose			: updates the current score
	Parameters		: ACTOR2 *,long
	Returns			: void
	Info			: 
*/
//void UpdateScore(ACTOR2 *act,long scoreUpdate)
//{
//	if (gameState.multi == SINGLEPLAYER)
//	{
//		player[0].score += scoreUpdate;
//		scoreTextOver->a = 255;
//	}
//}


/*	--------------------------------------------------------------------------------
	Function		: InitInGameTextOverlays
	Purpose			: creates and initialises in game text overlay messages
	Parameters		: unsigned long,unsigned long
	Returns			: void
	Info			: 
*/
void InitInGameTextOverlays(unsigned long worldID,unsigned long levelID)
{
//	pauseTitle		= CreateAndAddTextOverlay ( 50, 70, "pause", YES, NO, 255, 255, 255, 255, font, 0, 0, 0 );

	xselectText	= CreateAndAddTextOverlay ( 2048, 3400, GAMESTRING(STR_X_SELECT_OPTION), YES, 255, fontSmall, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED );
	sprintf( pauseTitleString, GAMESTRING(STR_PAUSE_MODE) );
	pauseTitleText	= CreateAndAddTextOverlay ( 2048, 400, pauseTitleString, YES, 255, fontSmall, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED );
	continueText	= CreateAndAddTextOverlay ( 2048, 1540, GAMESTRING(STR_CONTINUE), YES, 255, 0, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED );
	//controllerText	= CreateAndAddTextOverlay ( 0, 1540, "Controls", YES, 255, 0, 0,0 );
	restartText		= CreateAndAddTextOverlay ( 2048, 1860, GAMESTRING(STR_RESTARTLEVEL), YES, 255, 0,TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
#ifdef PC_VERSION
	if(gameState.mode == FRONTEND_MODE)
		quitText		= CreateAndAddTextOverlay ( 2048, 2180, GAMESTRING(STR_QUIT_TO_WINDOWS), YES, 255, 0, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
	else
		quitText		= CreateAndAddTextOverlay ( 2048, 2180, GAMESTRING(STR_QUIT), YES, 255, 0, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
#else
	quitText		= CreateAndAddTextOverlay ( 2048, 2180, GAMESTRING(STR_QUIT), YES, 255, 0, TEXTOVERLAY_SHADOW | TEXTOVERLAY_PAUSED);
#endif

	posText			= CreateAndAddTextOverlay ( 2048, 640, posString, YES, 255, fontSmall, 0);
	levelnameText	= CreateAndAddTextOverlay ( 2048, 800, levelString, YES, 255, fontSmall, 0);

	//	DisableTextOverlayF ( pauseTitle );

	DisableTextOverlay ( controllerText );
	DisableTextOverlay ( continueText );
	DisableTextOverlay ( xselectText );
	DisableTextOverlay ( pauseTitleText );
	DisableTextOverlay ( restartText );
	DisableTextOverlay ( quitText );
	DisableTextOverlay ( posText );
	DisableTextOverlay ( levelnameText );

	/* Lives and score */

	InitHUD();
//	livesTextOver	= CreateAndAddTextOverlay(50,205,livesText,NO,255,font,0,0);
//	scoreTextOver	= CreateAndAddTextOverlay(230,20,scoreText,NO,255,font,0,0);
//	timeTextOver	= CreateAndAddTextOverlay(25,20,timeText,NO,255,font,0,0);


	/* Garib and Credit counters */

	garibCount = CreateAndAddTextOverlay ( 100, 205, garibText, NO, 255, 0, 0 );
	creditCount = CreateAndAddTextOverlay ( 150, 205, creditText, NO, 255, 0, 0 );

	/* Baby display */

	babySavedText = CreateAndAddTextOverlay(2048,110,"BABY SAVED",YES,255,0,0);
	DisableTextOverlay(babySavedText);

	//if( screenshotEnable ) 
		//DisableHUD( );
}
