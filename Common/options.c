/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: overlays.c
	Programmer	: Matthew Cloy
	Date		: 20/04/99 10:24:40

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
#include "tongue.h"

#ifdef PC_VERSION
#include <pcmisc.h>
#include <stdio.h>
#include <pcaudio.h>
#include <Main.h>
#include <banks.h>
#include <backdrop.h>
#include <video.h>
#include <controll.h>
#elif PSX_VERSION
#include "backdrop.h"
#include "Textures.h"
#include "temp_psx.h"
#include "audio.h"
#include "temp_psx.h"
#include "main.h"
#include "memcard.h"
#include "font.h"
#include <libspu.h>
#endif
#include "layout.h"

extern GAMETILE *lastTitleTile;
extern FVECTOR storeCurrCamSource;
extern FVECTOR storeCurrCamTarget;
extern FVECTOR storeCamDist;
extern FVECTOR storeCurrCamDist;
extern FVECTOR storeCamOffset;
extern FVECTOR storeCurrCamOffset;
extern FVECTOR storeCamVect;

extern int lastArcade;

#ifdef PSX_VERSION
extern RECT clipBox1;
extern RECT clipBox2;
#endif

char warnStr[256];
char diffStr[128];
char playingFMV = NO;
char onOffStr[NUM_EXTRAS][64];
short pageDir;
// JH: A list of CRCs that define the char textures.
unsigned long frogTexturePool[FROG_NUMFROGS] = 
{
	0,			   
	295725604,	   
	3015291346,	   
	2473894757,
	4155454119,	   
	1018896364,	   
	4089408853,
	11947923,	   
};														

int soundSwapTime;
int gameSelected = 0;
int confirmMode = 0;
int lastFace = 0;
int staticFlash = 0;
TIMER chestTimer;
int maxPlayers;
int goingToArtViewer = 0;
int currentArt;


#define E_HEIGHT 200

char chapterStr[2][32];

long soundCount = 0;
long numFMVOpen = 1;
long numExtrasAvailable = NUM_BASIC_EXTRAS;

long SOUND_SCALE = 15;

int chestOpenAnim = 1;
TIMER artTimer;

OPTIONSOBJECTS options = 
{
	0,-1,0,0,0,"","",0,0,NULL,NULL,NULL,NULL,0,0,0,0,0,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,		//NUM_EXTRAS
	0,0,0,0,0,0,0,0,						//NUM_EXTRAS
	0,0,0,0,0,0,0,0,						//NUM_EXTRAS
	0,0,0,0,0,0,0,0,						//NUM_EXTRAS
	NULL,NULL,"","",0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	0,
};

int NUM_ARCADE_WORLDS = 8;

#define MAX_CREDITS 250
TEXTOVERLAY *creditsText[MAX_CREDITS] = {NULL};

int camStill;
long creditsRunning = 0;
long endingCredits = 0;
long picOffset = 230;

long creditsY = 0;

#define GREEN 60,255,0
#define WHITE 255,255,255
#define RED 255,0,0

CREDIT_DATA creditData[] = 
{
	0,GREEN,
	3,RED,
	0,GREEN,

	3,RED,		//founded by
	0,GREEN,

	3,RED,		//technical director
	0,GREEN,

	3,RED,		//project manager
	0,GREEN,

	3,RED,		//programming
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//design
	0,GREEN,
	0,GREEN,

	3,RED,		//level design and editing
	0,RED,		//level design and editing
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//concept art and illustration
	0,RED,		//concept art and illustration
	0,GREEN,

	3,RED,		//character building
	0,RED,		//and animation
	0,GREEN,

	3,RED,		//additional animation
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//textures
	0,GREEN,

	3,RED,		//additional textures
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,


	3,RED,		//3D modelling
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//music and sound
	0,GREEN,

	3,RED,		//video
	0,GREEN,

	3,RED,		//video producer
	0,GREEN,

	3,RED,		//video animation modelling and lighting
	0,RED,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//video modelling
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//voices
	0,GREEN,

	3,RED,		//voice artist
	0,GREEN,

	3,RED,		//voice engineer
	0,GREEN,

	3,RED,		//special thanks
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//for Hasbro Interactive Europe
	0,RED,

	3,RED,		//producer
	0,GREEN,

	3,RED,		//executive producer
	0,GREEN,

	3,RED,		//european development
	0,RED,
	0,GREEN,

	3,RED,		//senior product manager
	0,GREEN,

	3,RED,		//QA Manager
	0,GREEN,

	3,RED,		//QA Supervisor
	0,GREEN,

	3,RED,		//lead tester
	0,GREEN,

	3,RED,		//testers
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//hardware compatibility testing lab
	0,RED,
	0,GREEN,
	0,GREEN,
	
	3,RED,		//localisation
	0,RED,		//localisation
	0,GREEN,

	3,RED,		//art director
	0,GREEN,

	3,RED,		//manual manager
	0,GREEN,

	3,RED,		//documentation writer
	0,GREEN,

	3,RED,		//strategic marketing director
	0,RED,		
	0,GREEN,

	3,RED,		//commercial director international
	0,RED,		
	0,GREEN,

	3,RED,		//special thanks
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//for hasbro interactive US
	0,RED,		

	3,RED,		//CEO
	0,GREEN,

	3,RED,		//COO
	0,GREEN,

	3,RED,		//Product Manager
	0,GREEN,

	3,RED,		//Senior Product Manager
	0,RED,
	0,GREEN,

	3,RED,		//Chief Creative Officer
	0,GREEN,

	3,RED,		//Senior VP Research and Development
	0,RED,
	0,GREEN,

	3,RED,		//VP of Technology
	0,GREEN,

	3,RED,		//VP of External Development
	0,RED,
	0,GREEN,

	3,RED,		//VP of Studios
	0,GREEN,

	3,RED,		//VP of Development Operations
	0,RED,
	0,GREEN,

	3,RED,		//Director of Quality Assurance
	0,RED,
	0,GREEN,

	3,RED,		//QA Certification Manager
	0,RED,
	0,GREEN,

	3,RED,		//QA Certification Lead
	0,RED,
	0,GREEN,

	3,RED,		//QA Testing Managers
	0,RED,
	0,GREEN,
	0,GREEN,

	3,RED,		//testers
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,RED,		//General Manager
	0,GREEN,

	3,RED,		//Director of Marketing
	0,GREEN,

	3,RED,		//Director of Marketing
	0,GREEN,

	3,RED,		//Director of Marketing
	0,GREEN,

	3,RED,		//VP of Marketing Services
	0,RED,
	0,GREEN,

	3,RED,		//Chief Visual Officer
	0,GREEN,

	3,RED,		//manager of Creative Services
	0,RED,
	0,GREEN,

	3,RED,		//Manager of Editorial/Documentation Services
	0,RED,
	0,RED,
	0,GREEN,

	3,RED,		//Marketing Services Manager
	0,RED,
	0,GREEN,

	3,RED,		//Senior Graphic Designer
	0,GREEN,

	3,RED,		//Graphic Designer
	0,GREEN,

	3,RED,		//Graphic Designer
	0,GREEN,

	3,RED,		//Documentation Writer
	0,GREEN,

	3,RED,		//Copywriter
	0,GREEN,

	3,RED,		//Channel Marketing Director
	0,RED,
	0,GREEN,

	3,RED,		//VP of Public Relations
	0,RED,
	0,GREEN,

	3,RED,		//Manager of Public Relations
	0,RED,
	0,GREEN,

	3,RED,		//Manager of Technical Services
	0,RED,
	0,GREEN,

	3,RED,		//VP of Administration and Operations
	0,RED,
	0,GREEN,

	3,RED,		//Operations and specila projects manager
	0,RED,
	0,RED,
	0,GREEN,

	3,RED,		//legal and finance
	0,GREEN,
	0,GREEN,
	0,GREEN,
	0,GREEN,

	3,WHITE,	//separator
	1,WHITE,
	0,WHITE,
	1,WHITE,
	0,WHITE,
	0,WHITE,
	0,WHITE,
	1,WHITE,
	1,WHITE,
	0,WHITE,

};

#ifdef PSX_VERSION
int CREDIT_SPACING = 20;
#else
int CREDIT_SPACING = 300;
#endif

u16 optionsLastButton = -1;

// The titles
char *numberText[4] = {"1","2","3","4"};

// Some control functions
CONTROLFUNC controlsList[OP_TOTAL][C_TOTAL] =
{
	// Up,down,left,right,select,back
	{SoundUp,SoundDown,SoundLeft,SoundRight,NULL,OptionBack}, // Sound
	{ExtraUp,ExtraDown,NULL,NULL,ExtraSelect,OptionBack}, // Extra
	{NULL,NULL,NULL,NULL,NULL,OptionBack}, // Control
	{NULL,MenuBack,MenuLeft,MenuRight,MenuSelect,MenuBack}, //Main menu
	{MenuBack,NumPSelect,NumPLeft,NumPRight,NumPSelect,MenuBack}, //Multiplayer numplayer select
	{MPLevBack,NULL,MPLevLeft,MPLevRight,MPLevSelect,MPLevBack}, //Multiplayer level select
	{MPCharBack,NULL,MPCharLeft,MPCharRight,MPCharSelect,MPCharBack}, //Multiplayer char select
	{NULL,BookDown,BookLeft,BookRight,BookSelect,BookDown}, // book
	{ArcadeUp,ArcadeDown,ArcadeLeft,ArcadeRight,ArcadeSelect,ArcadeBack}, // arcade mode
	{SPCharUp,SPCharDown,NULL,NULL,SPCharSelect,SPCharBack}, //single player char select
	{FMVUp,FMVDown,NULL,NULL,FMVSelect,FMVBack}, //fmv menu
};

FVECTOR mpCharSelCamSource[] = 
{
	{-10091755,4149992,21120782},
	{ -9406870,4149992,21662182},
	{-10177212,4120894,21747884},
	{-10303569,4099387,20813268},
	{ -9499646,4035047,20305478},
	{ -8242643,4041855,21015220},
	{ -9515788,4074510,21300772},
	{ -9529414,4088749,20709322},
};

FVECTOR mpCharSelCamTarget[] = 
{
	{ -9118355,4149992,25098982},
	{-12314170,4149992,24544182},
	{-14269212,4120894,21676484},
	{-13507269,4099387,18264868},
	{-10662746,4035047,16378678},
	{ -7028143,4041855,17104020},
	{ -5541588,4074510,20328372},
	{ -6328414,4088749,23255522},
};

// Points to look at in the global menu
FVECTOR optTargets[NUM_OPTIONS] = 
{
	{-7502023,6134937,-4187053},
	{-8539837,5777338,-3717867},
};

#define NUM_MULTI_LEVELS 8

long multiLevelNum[NUM_MULTI_LEVELS] = 
{
	3,0,3,4,5,10,11,12
};


long multiWorldNum[NUM_MULTI_LEVELS] = 
{
	WORLDID_ANCIENT,WORLDID_CITY,WORLDID_LABORATORY,WORLDID_LABORATORY,WORLDID_LABORATORY,
	WORLDID_SUPERRETRO,WORLDID_SUPERRETRO,WORLDID_SUPERRETRO,
};

#define NUM_MULTICHARS 8

SVECTOR portraitPos[NUM_MULTICHARS] = 
{
	{-2170,1050,6370},
	{-3150,1060,6130},
	{-3780,1050,5270},
	{-3450,1040,4330},
	{-2660,1050,3810},
	{-1680,1060,4030},
	{-1120,1050,4870},
	{-1370,1040,5810},
};

#ifdef PC_VERSION
ACTOR2 *portraitActor[NUM_MULTICHARS];
#endif

#ifdef PSX_VERSION
FMA_MESH_HEADER *portraitActor[NUM_MULTICHARS];
#endif

//-----------------------------------------------------------------------------------------------------------
// Global Right control
//-----------------------------------------------------------------------------------------------------------
void MenuRight(void)
{
	if(options.selection)
	{
		options.selection--;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Global Left control
//-----------------------------------------------------------------------------------------------------------
void MenuLeft(void)
{
	if(options.selection == 0)
	{
		options.selection++;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Global Left control
//-----------------------------------------------------------------------------------------------------------
void NumPLeft(void)
{
	if(camStill)
	{
		options.numPlayers--;
		if (options.numPlayers<2)
			options.numPlayers = maxPlayers;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Global Left control
//-----------------------------------------------------------------------------------------------------------
void NumPSelect(void)
{
	int i;

	if((camStill) && (maxPlayers > 1))
	{
		options.mode=OP_CHARSEL;	
		PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
		options.mode = OP_CHARSEL;
		options.multiSelection = 0;
		options.currentPlayer = 0;
		for(i = 0;i < options.numPlayers;i++)
			options.playerNum[i]->draw = 1;
	}
}


void MPCharLeft()
{
	if(camStill)
	{
		options.multiSelection = (options.multiSelection - 1 + NUM_MULTICHARS) MOD NUM_MULTICHARS;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void MPCharRight()
{
	if(camStill)
	{
		options.multiSelection = (options.multiSelection + 1) MOD NUM_MULTICHARS;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void MPStart()
{
	options.mode = -1;
	options.numPText->draw = options.numPText2->draw = 0;
	GoMulti();
}

void MPCharSelect()
{
	int i,allowed = TRUE;

	if(camStill)
	{
		for(i = 0;i < options.currentPlayer;i++)
		{
			if(options.playerChar[i] == options.multiSelection)
			{
				allowed = FALSE;
				break;
			}
		}

		if((allowed) && (frogPool[options.multiSelection].active))
		{
			PlayVoice(0, "frogokay" );

#ifdef PC_VERSION
			SetActorGouraudValues(portraitActor[options.multiSelection],32,32,32);
#endif

#ifdef PSX_VERSION
			SetActorGouraudValues(portraitActor[options.multiSelection],32,32,32);
#endif

			options.playerNum[options.currentPlayer]->draw = 0;
			options.playerChar[options.currentPlayer] = options.multiSelection;
			
			options.multiFace[options.multiSelection]->draw = 1;
			options.multiFace[options.multiSelection]->r = options.multiFace[options.multiSelection]->g = options.multiFace[options.multiSelection]->b = options.multiFace[options.multiSelection]->a = 255;
			options.multiFace[options.multiSelection]->xPos = options.multiFace[options.currentPlayer]->xPosTo = backCharsX[options.currentPlayer];
			options.multiFace[options.multiSelection]->yPos = options.multiFace[options.currentPlayer]->yPosTo = backCharsY[options.currentPlayer];

			if (options.currentPlayer < options.numPlayers - 1)
			{
				options.currentPlayer++;
			}
			else
			{
				options.currentPlayer++;
				options.mode = OP_LEVELSEL;				
//				ScreenFade(255,0,30);
//				keepFade = 1;
//				gameSelected = 1;
			}
		}
		else
		{
			PlayVoice(0, "frogannoyed" );
		}
	}
}

void MPCharBack()
{
	int i;

	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	if(options.currentPlayer)
	{
		options.currentPlayer--;
		options.multiSelection = options.playerChar[options.currentPlayer];
		options.multiFace[options.playerChar[options.currentPlayer]]->draw = 0;
		options.playerNum[options.currentPlayer]->draw = 1;
#ifdef PC_VERSION
		SetActorGouraudValues(portraitActor[options.playerChar[options.currentPlayer]],255,255,255);
#endif
#ifdef PSX_VERSION
		SetActorGouraudValues(portraitActor[options.playerChar[options.currentPlayer]],128,128,128);
#endif
	}
	else
	{
		options.mode = OP_MULTIPLAYERNUMBER;
		for(i = 0;i < options.numPlayers;i++)
			options.playerNum[i]->draw = 0;
	}
}


void MPLevLeft()
{
	int j;

	if(camStill)
	{
		for(j = (options.levelSelection - 1 + NUM_MULTI_LEVELS) MOD NUM_MULTI_LEVELS;j != options.levelSelection;j = (j - 1 + NUM_MULTI_LEVELS) MOD NUM_MULTI_LEVELS)
		{
			if(worldVisualData[multiWorldNum[j]].levelVisualData[multiLevelNum[j]].levelOpen)
			{
				PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
				options.levelSelection = j;
				break;
			}
		}
	}
}

void MPLevRight()
{
	int j;

	if(camStill)
	{
		for(j = (options.levelSelection + 1 + NUM_MULTI_LEVELS) MOD NUM_MULTI_LEVELS;j != options.levelSelection;j = (j + 1 + NUM_MULTI_LEVELS) MOD NUM_MULTI_LEVELS)
		{
			if(worldVisualData[multiWorldNum[j]].levelVisualData[multiLevelNum[j]].levelOpen)
			{
				PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
				options.levelSelection = j;
				break;
			}
		}
	}
}

void MPLevSelect()
{
	int i;

	if(camStill)
	{
		ScreenFade(255,0,30);
		keepFade = 1;
		gameSelected = 1;
		lastArcade = TILENUM_MULTI;
		SetVectorFF(&storeCamSource,&camSource);
		SetVectorFF(&storeCurrCamSource,&currCamSource);
		SetVectorFF(&storeCamTarget,&camTarget);
		SetVectorFF(&storeCurrCamTarget,&currCamTarget);
		SetVectorFF(&storeCamDist,&camDist);
		SetVectorFF(&storeCurrCamDist,&currCamDist);
		SetVectorFF(&storeCamOffset,&camOffset);
		SetVectorFF(&storeCamOffset,&currCamOffset);
		SetVectorFF(&storeCamVect,&camVect);

//		options.mode = OP_CHARSEL;
//		options.multiSelection = 0;
//		options.currentPlayer = 0;
//		PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
//		for(i = 0;i < options.numPlayers;i++)
//			options.playerNum[i]->draw = 1;
	}
}


void MPLevBack()
{
	options.mode = OP_CHARSEL;
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	MPCharBack();
}

//-----------------------------------------------------------------------------------------------------------
// Multi Right control
//-----------------------------------------------------------------------------------------------------------
void NumPRight(void)
{
	if(camStill)
	{
		options.numPlayers++;
		if (options.numPlayers>maxPlayers)
			options.numPlayers = 2;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Global Back control
//-----------------------------------------------------------------------------------------------------------

extern long reachedPoint;
void MenuBack(void)
{
	fog.max = fogStore;

	if(options.mode == OP_MULTIPLAYERNUMBER)
	{
		SetVectorFF(&currCamSource,&storeCamSource);
		SetVectorFF(&currCamTarget,&storeCamTarget);
		SetVectorFF(&camSource,&storeCamSource);
		SetVectorFF(&camTarget,&storeCamTarget);
		SetVectorFF(&currCamOffset,&storeCurrCamOffset);
		SetVectorFF(&camOffset,&storeCurrCamOffset);
	}

	if(options.mode == OP_GLOBALMENU)
	{
		player[0].extendedHopDir = MOVE_LEFT;
		prevCamFacing[0] = camFacing[0];
//		player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPL;
		frogFacing[0] = (camFacing[0] + MOVE_LEFT) & 3;
		switch(camFacing[0])
		{
			case 0:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPL;
				utilPrintf("L\n\n");
				break;
			case 1:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPU;
				utilPrintf("U\n\n");

				break;
			case 2:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPR;
				utilPrintf("R\n\n");
				break;
			case 3:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
				utilPrintf("D\n\n");

				break;
		}
	}
	else
	{
		player[0].extendedHopDir = MOVE_RIGHT;
		switch(camFacing[0])
		{
			case 0:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPU;
				break;
			case 1:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPR;
				break;
			case 2:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
				break;
			case 3:
				player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPL;
				break;
		}
		frogFacing[0] = (camFacing[0] + MOVE_RIGHT) & 3;
	}
	player[0].canJump = 0;
	player[0].isSuperHopping = 1;
	player[0].hasJumped = 1;

	// Hide all the overlays
	DoneOptionsMenu();

	// update player idleTime (Maybe not needed?)
	player[0].idleTime = MAX_IDLE_TIME;

	fixedPos = 0;
	fixedDir = 0;
	reachedPoint = 0;
}

//-----------------------------------------------------------------------------------------------------------
// Global Select control
//-----------------------------------------------------------------------------------------------------------
void MenuSelect(void)
{
	int i;

	
	options.mode = options.selection;
	if(options.mode == OP_EXTRA)
	{
		if(options.chestLid)
		{
			GTInit(&chestTimer,4);
			actorAnimate(options.chestLid->actor,chestOpenAnim,NO,NO,40,NO);
//			actorAnimate(options.chestLid->actor,2,YES,YES,40,NO);
		}
		for(i = 0;i < numExtrasAvailable;i++)
		{
			options.extras[i]->a = 0;

//			if(options.extrasState[i])
//				options.extras[i]->g = options.extras[i]->b = 0;
//			else
//				options.extras[i]->g = options.extras[i]->b = 255;
		}
	}
	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}

//-----------------------------------------------------------------------------------------------------------
// Multiplayer select control
//-----------------------------------------------------------------------------------------------------------

void GoMulti(void)
{
	int i;

	fog.max = fogStore;
	NUM_FROGS = options.numPlayers;
	lastActFrameCount = actFrameCount;

	player[0].worldNum = multiWorldNum[options.levelSelection];
	player[0].levelNum = multiLevelNum[options.levelSelection];

	gameState.mode = INGAME_MODE;
	gameState.single = INVALID_MODE;
	if(gameState.multi != MULTIREMOTE)
		gameState.multi = MULTILOCAL;

	multiplayerMode = multiGameTypes[player[0].worldNum];

	for(i = 0;i < options.currentPlayer;i++)
		player[i].character = options.playerChar[i];
	InitLevel(player[0].worldNum, player[0].levelNum);
}

void SoundUp(void)
{
	if(options.soundSelection > 0)
	{
		options.soundIcons[0]->r = options.soundIcons[0]->g = options.soundIcons[0]->b = 255;
		options.soundIcons[1]->r = options.soundIcons[1]->g = options.soundIcons[1]->b = 64;

		options.soundBar[0]->r = 255;
		options.soundBar[0]->g = options.soundBar[0]->b = 0;
		options.soundBar[1]->r = options.soundBar[1]->g = options.soundBar[1]->b = 255;

		options.soundSelection--;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
		soundSwapTime = actFrameCount;
	}
#ifdef DREAMCAST_VERSION
	else if( !options.soundSelection )
	{
		// Disable both the sliders
		options.soundIcons[0]->r = options.soundIcons[0]->g = options.soundIcons[0]->b = 64;
		options.soundIcons[1]->r = options.soundIcons[1]->g = options.soundIcons[1]->b = 64;
		options.soundBar[0]->r = options.soundBar[0]->g = options.soundBar[0]->b = 255;
		options.soundBar[1]->r = options.soundBar[1]->g = options.soundBar[1]->b = 255;

		options.stereoSelectText[0]->a = options.stereoSelectText[1]->a = 255;

		options.soundSelection--;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
		soundSwapTime = actFrameCount;
	}
#endif
}


void SoundDown(void)
{
	if(options.soundSelection < 1)
	{
#ifdef DREAMCAST_VERSION
		options.stereoSelectText[0]->a = options.stereoSelectText[1]->a = 128;
#endif

		options.soundSelection++;

		options.soundIcons[1-options.soundSelection]->r = options.soundIcons[1-options.soundSelection]->g = options.soundIcons[1-options.soundSelection]->b = 64;
		options.soundIcons[options.soundSelection]->r = options.soundIcons[options.soundSelection]->g = options.soundIcons[options.soundSelection]->b = 255;

		options.soundBar[1-options.soundSelection]->r = options.soundBar[1-options.soundSelection]->g = options.soundBar[1-options.soundSelection]->b = 255;
		options.soundBar[options.soundSelection]->r = 255;
		options.soundBar[options.soundSelection]->g = options.soundBar[options.soundSelection]->b = 0;

		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
		soundSwapTime = actFrameCount;
	}
}


void SoundLeft(void)
{
	switch(options.soundSelection)
	{
#ifdef DREAMCAST_VERSION
		case -1:
			ToggleStereo( );
			break;
#endif
		case 0:
			if(globalMusicVol)
			{
				globalMusicVol--;
				SetMusicVolume();
			}
			break;

		case 1:
			if(globalSoundVol)
			{
				globalSoundVol--;
				if(soundCount <= 0)
				{
					PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
					soundCount = 4096*10;
				}
			}
			break;
	}
}

void SoundRight(void)
{
	switch(options.soundSelection)
	{
#ifdef DREAMCAST_VERSION
		case -1: 
			ToggleStereo( );
			break;
#endif
		case 0:
			if(globalMusicVol < MAX_SOUND_VOL)
			{
				globalMusicVol++;
				SetMusicVolume();
			}
			break;

		case 1:
			if(globalSoundVol < MAX_SOUND_VOL)
			{
				globalSoundVol++;
				if(soundCount <= 0)
				{
					PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
					soundCount = 4096*10;
				}
			}
			break;
	}
}

//-----------------------------------------------------------------------------------------------------------
// Extras Up
//-----------------------------------------------------------------------------------------------------------

void ExtraUp(void)
{
	if(!creditsRunning)
	{
		if(confirmMode)
			options.extraSelection = 1 - options.extraSelection;
		else
			options.extraSelection = (options.extraSelection - 1 + numExtrasAvailable) MOD numExtrasAvailable;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Extras Down
//-----------------------------------------------------------------------------------------------------------
void ExtraDown(void)
{
	if(!creditsRunning)
	{
		if(confirmMode)
			options.extraSelection = 1 - options.extraSelection;
		else
			options.extraSelection = (options.extraSelection + 1) MOD numExtrasAvailable;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

//-----------------------------------------------------------------------------------------------------------
// Extras Select
//-----------------------------------------------------------------------------------------------------------
char fmvStr[NUM_FMV_SEQUENCES][64];
void ExtraSelect(void)
{
	int i,j,switchVal;

	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	if(creditsRunning)
		endingCredits = YES;
	else
	{
		if(options.extrasToggle[options.extraSelection])
		{
			options.extrasState[options.extraSelection] = !options.extrasState[options.extraSelection];
			sprintf(onOffStr[options.extraSelection],GAMESTRING(STR_EXTRAS_1 + options.extraSelection),GAMESTRING(STR_OFF - options.extrasState[options.extraSelection]));
		}

//		if(options.extrasState[options.extraSelection])
//		{
//			options.extras[options.extraSelection]->g = options.extras[options.extraSelection]->b = 0;
//			options.extras[options.extraSelection]->r = 255;
//		}
//		else
			options.extras[options.extraSelection]->r = options.extras[options.extraSelection]->g = options.extras[options.extraSelection]->b = 255;

		if(confirmMode)
			switchVal = confirmMode;
		else
			switchVal = options.extraSelection;
		switch(switchVal)
		{
			case EXTRA_DIFFICULTY:
				gameState.difficulty = (gameState.difficulty + 1) MOD 3;
				sprintf(diffStr,GAMESTRING(STR_EXTRAS_1),GAMESTRING(STR_DIFFICULTY_1 + gameState.difficulty));
				break;

			case EXTRA_VIEW_FMV:
				options.mode = OP_FMV;
				for(i = 0;i < numExtrasAvailable;i++)
					options.extras[i]->draw = 0;
				numFMVOpen = 2 + options.maxPageAllowed;
				if(worldVisualData[storySequence[NUM_STORY_LEVELS - 1].worldNum].levelVisualData[storySequence[NUM_STORY_LEVELS - 1].levelNum].levelCompleted)
					numFMVOpen++;
				for(i = 0;i < numFMVOpen;i++)
				{
					if(i == 0)
						sprintf(fmvStr[i],GAMESTRING(STR_FMV_1));
					else if(i == NUM_FMV_SEQUENCES - 1 - FMV_INTRO)
						sprintf(fmvStr[i],GAMESTRING(STR_FMV_2));
					else
						sprintf(fmvStr[i],"%d - %s %s",i,GAMESTRING(STR_CHAPTER_1a + (i-1)*2),GAMESTRING(STR_CHAPTER_1b + (i-1)*2));
					options.fmvText[i] = CreateAndAddTextOverlay(2048,(2048-(numFMVOpen/2)*E_HEIGHT)+(i*E_HEIGHT),fmvStr[i],YES,255,fontSmall,TEXTOVERLAY_SHADOW);
					options.fmvText[i]->r = options.fmvText[i]->g = options.fmvText[i]->b = 200;
				}
				break;


			case EXTRA_VIEW_ART:
				ScreenFade(255,0,30);
				keepFade = YES;
				goingToArtViewer = YES;
				break;

			case EXTRA_CREDITS:
				StartCredits();
				break;


			case EXTRA_RESET_BEST_TIMES:
				if(confirmMode == NO)
				{
					confirmMode = EXTRA_RESET_BEST_TIMES;
					options.extraSelection = 1;
					options.yesNoText[0]->yPos = options.yesNoText[0]->yPosTo = options.extras[EXTRA_RESET_BEST_TIMES]->yPos + 300;
					options.yesNoText[1]->yPos = options.yesNoText[1]->yPosTo = options.yesNoText[0]->yPos + 200;
				}
				else 
				{
					if(options.extraSelection == 0)
					{
						for(i = 0;i < NUM_WORLDS;i++)
						{
							for(j = 0;j < worldVisualData[i].numLevels;j++)
							{
								worldVisualData[i].levelVisualData[j].parTime = origWorldVisualData[i].levelVisualData[j].parTime;
								strcpy(worldVisualData[i].levelVisualData[j].parName,origWorldVisualData[i].levelVisualData[j].parName);
								worldVisualData[i].levelVisualData[j].levelBeaten = 0;
							}
						}
						SaveGame();
					}
					options.extraSelection = confirmMode;
					confirmMode = NO;
				}
				break;

			case EXTRA_RESET_STORY_MODE:
				if(confirmMode == NO)
				{
					confirmMode = EXTRA_RESET_STORY_MODE;
					options.extraSelection = 1;
					options.yesNoText[0]->yPos = options.yesNoText[0]->yPosTo = options.extras[EXTRA_RESET_STORY_MODE]->yPos + 300;
					options.yesNoText[1]->yPos = options.yesNoText[1]->yPosTo = options.yesNoText[0]->yPos + 200;
				}
				else
				{
					if(options.extraSelection == 0)
					{	
						options.levelNum = 0;
						cWorld = 0;
						staticFlash = 5;
						cheatCombos[CHEAT_OPEN_ALL_LEVELS].state = 0;
						options.maxPageAllowed = options.pageNum = 0;
						if(options.page)
							grabToTexture = 1;
						else
						{
							options.chapterPic[0]->tex = options.chapterPic[1]->tex = FindTexture(chapterPic[options.pageNum]);
							sprintf(chapterStr[0],GAMESTRING(STR_CHAPTER),options.pageNum + 1);
							options.chapterNameText[0][0]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2);
							options.chapterNameText[0][1]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2 + 1);
						}

						for(i = 4;i < 8;i++)
						{
							frogPool[i].active = 0;
#ifdef PSX_VERSION
							CopyTexture ( textureFindCRCInAllBanks ( frogTexturePool[i] ),
											textureFindCRCInAllBanks ( utilStr2CRC ( "MULTIHIDDEN" ) ), 1 );
#elif PC_VERSION
							((MDX_ACTOR *)portraitActor[i]->actor->actualActor)->overrideTex = GetTexEntryFromCRC(UpdateCRC("multihidden.bmp"));
#endif

						}

						for(i = 0;i < NUM_WORLDS;i++)
						{
							worldVisualData[i].worldOpen = origWorldVisualData[i].worldOpen;
							for(j = 0;j < worldVisualData[i].numLevels;j++)
							{
								worldVisualData[i].levelVisualData[j].maxCoins = 0;
								worldVisualData[i].levelVisualData[j].levelCompleted = origWorldVisualData[i].levelVisualData[j].levelCompleted;
								worldVisualData[i].levelVisualData[j].levelOpen = origWorldVisualData[i].levelVisualData[j].levelOpen;
							}
						}
						SaveGame();
						CheckForExtras();
					}
					options.extraSelection = confirmMode;
					confirmMode = NO;
				}
				break;

			case EXTRA_FEEDING_FRENZY:
				eatEverythingMode = options.extrasState[options.extraSelection];
				break;

			case EXTRA_MINI:
				if(options.extrasState[options.extraSelection])
					globalFrogScale = FMul(2457,1500);
				else
					globalFrogScale = 2457;
			 	frog[0]->actor->size.vx = globalFrogScale;
			 	frog[0]->actor->size.vy = globalFrogScale;
			 	frog[0]->actor->size.vz = globalFrogScale;
				frog[0]->actor->shadow->radius = ToFixed(globalFrogScale/8);

				break;

			default:
				break;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
// Global options back control
//-----------------------------------------------------------------------------------------------------------
void OptionBack(void)
{
	int i;
	// Back to the menu
	if(creditsRunning)
	{
		endingCredits = YES;
	}
	else
	{
		if(confirmMode)
		{
			options.extraSelection = confirmMode;
			confirmMode = NO;
		}
		else
		{
			if(options.mode == OP_EXTRA)
			{
				if(options.chestLid)
				{
					actorAnimate(options.chestLid->actor,chestOpenAnim,NO,NO,-40,NO);
	//				actorAnimate(options.chestLid->actor,0,YES,YES,40,NO);
				}
			}

			options.mode = OP_GLOBALMENU;

			// Disable all overlays (Yes... even ones not for this menu, but not that slow and neater.)
			for (i=0; i<2; i++)
			{
				options.soundIcons[i]->draw = options.soundIcons[i]->a = 0;
				options.soundBar[i]->draw = options.soundBar[i]->a = 0;
				options.soundBak[i]->draw = options.soundBak[i]->a = 0;

#ifdef DREAMCAST_VERSION
				options.stereoSelectText[i]->draw = 0;
#endif
			}

			for (i=0; i<numExtrasAvailable; i++)
			{
				options.extras[i]->draw = 0;	
			}
		}
	}
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	titleHud[0]->a = titleHud[0]->draw = 0;
}

//-----------------------------------------------------------------------------------------------------------
// Create all required objects
//-----------------------------------------------------------------------------------------------------------


void CreateOptionsObjects(void)
{
	long i;
	ACTOR2 *actor;

	for(i = 0;i < FROG_NUMFROGS;i++)
	{
		options.multiFace[i] = CreateAndAddSpriteOverlay(1500,i*300 + 1000,frogPool[i].icon,512,512,255,0);
		options.multiFace[i]->num = 2;
		options.multiFace[i]->draw = 0;
	}

	options.yesNoText[0] = CreateAndAddTextOverlay(2048,2000,GAMESTRING(STR_YES),YES,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.yesNoText[1] = CreateAndAddTextOverlay(2048,2400,GAMESTRING(STR_NO),YES,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.yesNoText[0]->draw = options.yesNoText[1]->draw = 0;


	// Main title
	options.title = CreateAndAddTextOverlay(2048,270,options.titleStr,YES,0,font,TEXTOVERLAY_SHADOW);
	options.title->b = 0;
	options.title->draw = 0;

	// Sub text, initially for multiplayer, but maybe for options too.
	options.subTitle = CreateAndAddTextOverlay(2048,4096-400,options.subTitleStr,YES,0,fontSmall,TEXTOVERLAY_SHADOW);
	options.subTitle->draw = 0;
	options.mpText = CreateAndAddTextOverlay(2048,550,options.mpStr,YES,255,fontSmall,TEXTOVERLAY_SHADOW);
	
	// Number of players text
	options.numPText = CreateAndAddTextOverlay(2048,2048,options.numPStr,YES,255,font,0);
	options.numPText2 = CreateAndAddTextOverlay(2048,2198,options.numPStr2,YES,255,font,0);
	
	// Sound options
	options.soundIcons[0] = CreateAndAddSpriteOverlay(900,1750-256,"MUSVOL0",512,512,0,0);
	options.soundIcons[0]->draw = 0;

	options.soundIcons[1] = CreateAndAddSpriteOverlay(900,2550-256,"SFXVOL0",512,512,0,0);
	options.soundIcons[1]->draw = 0;

	options.soundBak[0] = CreateAndAddSpriteOverlay(1600-16,1750-17,NULL,100*SOUND_SCALE+32,134,0,SPRITE_SUBTRACTIVE);
	options.soundBak[0]->draw = 0;

	options.soundBar[0] = CreateAndAddSpriteOverlay(1600,1750,NULL,globalMusicVol*SOUND_SCALE,100,0,0);
	options.soundBar[0]->draw = 0;
	options.soundBar[0]->num = 1;

	options.soundBak[1] = CreateAndAddSpriteOverlay(1600-16,2550-17,NULL,100*SOUND_SCALE+32,134,0,SPRITE_SUBTRACTIVE);
	options.soundBak[1]->draw = 0;


	options.soundBar[1] = CreateAndAddSpriteOverlay(1600,2550,NULL,globalSoundVol*SOUND_SCALE,100,0,0);
	options.soundBar[1]->draw = 0;
	options.soundBar[1]->num = 1;

	options.soundBar[options.soundSelection]->r = 255;
	options.soundBar[options.soundSelection]->g = options.soundBar[options.soundSelection]->b = 0;
	options.soundBar[1-options.soundSelection]->r = options.soundBar[1-options.soundSelection]->g = options.soundBar[1-options.soundSelection]->b = 255;
	options.soundIcons[options.soundSelection]->r = options.soundIcons[options.soundSelection]->g = options.soundIcons[options.soundSelection]->b = 255;
	options.soundIcons[1-options.soundSelection]->r = options.soundIcons[1-options.soundSelection]->g = options.soundIcons[1-options.soundSelection]->b = 64;

	options.sfxText[0] = CreateAndAddTextOverlay(2350,1400,GAMESTRING(STR_MUS_VOL),YES,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.sfxText[1] = CreateAndAddTextOverlay(2350,2220,GAMESTRING(STR_SFX_VOL),YES,255,fontSmall,TEXTOVERLAY_SHADOW);

	options.sfxText[0]->draw = 0;
	options.sfxText[1]->draw = 0;

	options.soundInstrText[0] = CreateAndAddTextOverlay(1700,3070,GAMESTRING(STR_SOUND_INSTR_2),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.soundInstrText[1] = CreateAndAddTextOverlay(1700,3370,GAMESTRING(STR_SOUND_INSTR_1),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.soundInstrText[0]->draw = 0;
	options.soundInstrText[1]->draw = 0;
#ifdef DREAMCAST_VERSION
	options.stereo = 0;
	options.stereoSelectText[0] = CreateAndAddTextOverlay(1650,1024,GAMESTRING(STR_SOUNDMONO),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.stereoSelectText[1] = CreateAndAddTextOverlay(2250,1024,GAMESTRING(STR_SOUNDSTEREO),NO,255,fontSmall,TEXTOVERLAY_SHADOW);
	options.stereoSelectText[0]->draw = 0;
	options.stereoSelectText[1]->draw = 0;

	options.stereoSelectText[1-options.stereo]->r = options.stereoSelectText[1-options.stereo]->g = options.stereoSelectText[1-options.stereo]->b = 255;
	options.stereoSelectText[options.stereo]->r = 255;
	options.stereoSelectText[options.stereo]->g = options.stereoSelectText[options.stereo]->b = 0;
#endif

	for(i = 0;i < 4;i++)
	{
		options.warnText[i] = CreateAndAddTextOverlay(2048,800+i*200,NULL,YES,0,fontSmall,TEXTOVERLAY_SHADOW);
		options.warnText[i]->draw = 0;
	}

	for (i=0; i<NUM_EXTRAS; i++)
	{
		if(i == 0)
		{
			sprintf(diffStr,GAMESTRING(STR_EXTRAS_1),GAMESTRING(STR_DIFFICULTY_1 + gameState.difficulty));
			options.extras[i] = CreateAndAddTextOverlay(2048,(2048-(NUM_EXTRAS/2)*E_HEIGHT)+(i*E_HEIGHT),diffStr,YES,255,fontSmall,TEXTOVERLAY_SHADOW);
		}
		else
		{
			if(options.extrasToggle[i])
			{
				sprintf(onOffStr[i],GAMESTRING(STR_EXTRAS_1 + i),GAMESTRING(STR_OFF - options.extrasState[i]));
				options.extras[i] = CreateAndAddTextOverlay(2048,(2048-(NUM_EXTRAS/2)*E_HEIGHT)+(i*E_HEIGHT),onOffStr[i],YES,255,fontSmall,TEXTOVERLAY_SHADOW);
			}
			else
				options.extras[i] = CreateAndAddTextOverlay(2048,(2048-(NUM_EXTRAS/2)*E_HEIGHT)+(i*E_HEIGHT),GAMESTRING(i + STR_EXTRAS_1),YES,255,fontSmall,TEXTOVERLAY_SHADOW);
		}
		options.extras[i]->draw = 0;
	}

	for(i = 0;i < 4;i++)
	{
		options.playerNum[i] = CreateAndAddTextOverlay(backCharsX[i]+200,backCharsY[i]+100,numberText[i],NO,255,font,0);
		options.playerNum[i]->draw = 0;
	}


	// Add more here later


#ifdef PC_VERSION
	for(i = 0;i < NUM_MULTICHARS;i++)
	{
		actor = actList;
		while(actor)
		{
			if((actor->actor->position.vx == portraitPos[i].vx) && (actor->actor->position.vy == portraitPos[i].vy) && (actor->actor->position.vz == portraitPos[i].vz))
			{
				portraitActor[i] = actor;

				if(frogPool[i].active == 0)
					((MDX_ACTOR *)portraitActor[i]->actor->actualActor)->overrideTex = GetTexEntryFromCRC(UpdateCRC("multihidden.bmp"));

			}
			actor = actor->next;
		}
	}
#endif

#ifdef PSX_VERSION

	// JH: This copies the hidden texture over any char textures that are hidden.
	// JH: frogTexturePool, is defined above this function, it's just a list a CRCs.
	for(i = 0;i < NUM_MULTICHARS;i++)
	{
		SCENICOBJ *cur;
		FMA_MESH_HEADER **mesh;
		for ( cur = scenicObjList.head.next; cur != &scenicObjList.head; cur = cur->next)
		{
			if ( cur->fmaObj )
			{
				mesh = ADD2POINTER(cur->fmaObj,sizeof(FMA_WORLD));

				if( ( cur->matrix.t[0] == portraitPos[i].vx ) &&
					  ( cur->matrix.t[1] == portraitPos[i].vy ) &&
						( cur->matrix.t[2] == portraitPos[i].vz ) )
				{
					portraitActor [ i ] = *mesh;
				}
				// ENDIF

			}
		}

		if(frogPool[i].active == 0)
				CopyTexture ( textureFindCRCInAllBanks ( frogTexturePool[i] ),
											textureFindCRCInAllBanks ( utilStr2CRC ( "MULTIHIDDEN" ) ), 1 );
	}
#endif
}

//-----------------------------------------------------------------------------------------------------------
// Init options menu, initialises the options system to default states, and set's up any required objects.
//-----------------------------------------------------------------------------------------------------------
void InitOptionsMenu(void)
{
	//Default to sound setup, and put us in the global menu (Sound, Extra and Controller choice!)
	if (!options.init)
	{		
		options.currentPlayer = 0;
		options.numPlayers = 2;
		options.selection = OP_SOUND;
		options.soundVol = DEFAULT_VOLUME;
		options.musicVol = DEFAULT_VOLUME;
		options.init =1;
		options.extraSelection = 0;
		options.soundSelection = 0;

		options.extrasToggle[EXTRA_VIEW_FMV] = NO;
		options.extrasToggle[EXTRA_CREDITS] = NO;
		options.extrasToggle[EXTRA_RESET_BEST_TIMES] = NO;
		options.extrasToggle[EXTRA_RESET_STORY_MODE] = NO;
		options.extrasToggle[EXTRA_FEEDING_FRENZY] = YES;
		options.extrasToggle[EXTRA_VIEW_ART] = NO;
		options.extrasToggle[EXTRA_MINI] = YES;
		CreateOptionsObjects();
		gameSelected = 0;
	}
	else	
	{
		CreateOptionsObjects();
		options.numPText->draw = options.numPText2->draw = 0;
		options.mpText->draw = 0;
		gameSelected = 0;
	}
	options.mode = -1;
}

//-----------------------------------------------------------------------------------------------------------
// Shutdown the options menu... Doesn't free objects... done in FreeAllLists when the frontend exits.
//-----------------------------------------------------------------------------------------------------------
void DoneOptionsMenu(void)

{
	// Sub text, initially for multiplayer, but maybe for options too.
	options.mpText->draw = 0;
	
	// Number of players text
	options.numPText->draw = options.numPText2->draw = 0; 

	if(options.mode != OP_MULTIPLAYERNUMBER)
		SubVectorFFF(&currCamOffset,&currCamSource,&currCamTarget);
	options.mode = -1;
}


//sbond - put in for GOLDCD
#ifdef FINAL_MASTER
int properMultiSelect = 1;
#else
int properMultiSelect = 0;
#endif
//-----------------------------------------------------------------------------------------------------------
// Process the controller stuff.
//-----------------------------------------------------------------------------------------------------------
void OptionsProcessController(void)
{
	u16 button;

	if((camControlMode) || (fadingOut))
		return;

	if((options.mode == OP_ARCADE) && ((!CheckCamStill()) || (options.arcadeText->a < 200)))
		return;

	if((options.mode == OP_GLOBALMENU) && (!CheckCamStill()))
		return;
	
	if((properMultiSelect) && (options.mode == OP_CHARSEL))
		button = padData.digital[options.currentPlayer];
	else
		button = padData.digital[0];
	

		
// Call correct controller funcs.
	if((button & PAD_UP) && !(optionsLastButton & PAD_UP) && options.controls[C_UP])
	{
		options.controls[C_UP]();
		optionsLastButton = button;
		return;
	}

	if((button & PAD_RIGHT) && ((!(optionsLastButton & PAD_RIGHT)) || (options.mode == OP_SOUND && options.soundSelection != -1)) && options.controls[C_RIGHT])
	{
		options.controls[C_RIGHT]();
		optionsLastButton = button;
		return;
	}

	if((button & PAD_DOWN) && !(optionsLastButton & PAD_DOWN) && options.controls[C_DOWN])
	{
		options.controls[C_DOWN]();
		optionsLastButton = button;
		return;
	}

	if((button & PAD_LEFT) && ((!(optionsLastButton & PAD_LEFT)) || (options.mode == OP_SOUND && options.soundSelection != -1)) && options.controls[C_LEFT])
	{
		options.controls[C_LEFT]();
		optionsLastButton = button;
		return;
	}

	// Currently unimplemented
	if((button & PAD_CROSS) && !(optionsLastButton & PAD_CROSS) && options.controls[C_SELECT])
	{
		options.controls[C_SELECT]();
		optionsLastButton = button;
		return;
	}

  if((button & PAD_TRIANGLE) && !(optionsLastButton & PAD_TRIANGLE) && options.controls[C_BACK])
	{
		options.controls[C_BACK]();
		optionsLastButton = button;
		return;
	}
		optionsLastButton = button;

}

//-----------------------------------------------------------------------------------------------------------
// Look at waterfall
//-----------------------------------------------------------------------------------------------------------
void LookatWaterfall(void)
{
/*
	camSource.vx = -8567230;
	camSource.vy = 5633144;
	camSource.vz = -4819483;

	camTarget.vx = -8960030;
	camTarget.vy = 5381824;
	camTarget.vz = -5493003;	
*/
	camSource.vx = -8196622;
	camSource.vy = 4762946;
	camSource.vz = -8277231;

	camTarget.vx = -8877516;
	camTarget.vy = 4456028;
	camTarget.vz = -9244387;	
}

//FVECTOR bookCamSource = {14477086,6133174,2384419};
//FVECTOR bookCamTarget = {14494723,6097578,2394398};
//fixed bookcamspeed = 4096*5;


fixed mpcamspeed = 4096*70;
int optAnim = 0;
int soundwobble = 40;
int soundwobblefac = 400;
char *warnPtr;
//-----------------------------------------------------------------------------------------------------------
// Actually do the options stuff
//-----------------------------------------------------------------------------------------------------------

char levNameColR[] = {255,255,0,  0,  0,  255,255,255};
char levNameColG[] = {0,  0  ,255,255,255,255,255,255};
char mpLevNameStr[128];
char *mpLevNameArray[2];
void RunOptionsMenu(void)
{
	int i;
	char textBuff[64];
	FVECTOR temp;
	SVECTOR tempVect = {-4371,321,-1777};
	SVECTOR tempVect2;
	SPECFX *fx;

	int oldTime;

#ifdef PC_VERSION
	checkMenuKeys = 1;
#endif
	options.numPText->text = options.numPStr;
	options.numPText2->text = options.numPStr2;

	options.numPText->yPos = 2048;
	options.numPText->font = font;
	options.numPText2->font = font;

	options.sfxText[0]->draw = 0;
	options.sfxText[1]->draw = 0;

	options.numPText->r = options.numPText->g = options.numPText->b = 255;
	options.numPText2->r = options.numPText2->g = options.numPText2->b = 255;
#ifdef PSX_VERSION
	if(saveInfo.saveFrame)
	{
		skipTextOverlaysSpecFX = YES;
		ChooseLoadSave();
		return;
	}
	skipTextOverlaysSpecFX = NO;
#endif

	camStill = FALSE;

	numExtrasAvailable = NUM_BASIC_EXTRAS;
	for(i = 2;i < NUM_EXTRAS;i++)
		if(options.extrasAvailable[i])
			numExtrasAvailable++;

	// Sort out the title
	strcpy(options.titleStr,GAMESTRING(options.selection + STR_OPTIONS_1));
	strcpy(options.subTitleStr,"");

	// Setup appropriate controls
	for (i=0; i<C_TOTAL; i++)
		options.controls[i] = controlsList[options.mode][i];
	
	if((options.mode != OP_BOOK))// && (options.mode != OP_ARCADE) && (options.mode != OP_SP_CHARSEL))
	{
		fixedPos = 1;
		fixedDir = 1;
	}

	options.mpText->draw = 0;
	// Do what we need to
	if(options.mode != OP_EXTRA)
		options.yesNoText[0]->draw = options.yesNoText[1]->draw = 0;

	options.soundInstrText[0]->draw = 0;
	options.soundInstrText[1]->draw = 0;

	if(!confirmMode)
	{
		for(i = 0;i < 4;i++)
			DEC_ALPHA(options.warnText[i]);
	}

	switch (options.mode)
	{
		case OP_SOUND:			
			options.soundInstrText[0]->draw = 1;
			options.soundInstrText[1]->draw = 1;
			options.sfxText[0]->draw = 1;
			options.sfxText[1]->draw = 1;

#ifdef DREAMCAST_VERSION
			if( options.stereo > 1 || options.stereo < 0 )
				options.stereo = 0;
#endif

			options.sfxText[0]->r = options.soundBar[0]->r;
			options.sfxText[0]->g = options.soundBar[0]->g;
			options.sfxText[0]->b = options.soundBar[0]->b;
			options.sfxText[1]->r = options.soundBar[1]->r;
			options.sfxText[1]->g = options.soundBar[1]->g;
			options.sfxText[1]->b = options.soundBar[1]->b;


			DEC_ALPHA(options.selectText);
#ifdef PC_VERSION
			if( options.soundSelection > 0 )
			{
				options.soundIcons[options.soundSelection]->angle = FMul(rsin((actFrameCount - soundSwapTime)*soundwobble),soundwobblefac);
				options.soundIcons[1 - options.soundSelection]->angle *= 4;
				options.soundIcons[1 - options.soundSelection]->angle /= 5;
			}
#endif
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
			LookatWaterfall();
			UpdateSoundOptions();
			for (i=0; i<2; i++)
			{
				INC_ALPHA(options.soundIcons[i],255);
				INC_ALPHA(options.soundBar[i],255);
				INC_ALPHA(options.soundBak[i],128);
#ifdef DREAMCAST_VERSION
				options.stereoSelectText[i]->draw = 1;
#endif
			}
			break;

		case OP_CONTROL:
			break;

		case OP_EXTRA:
			INC_ALPHA(options.selectText,0xff);
			if(chestTimer.time)
			{
				oldTime = chestTimer.time;
				GTUpdate2(&chestTimer,-1);
				
				if((chestTimer.time == 2) && (oldTime == 3))
				{
					FVECTOR up;
					SetVectorFF( &up, &currTile[0]->normal );
					PlaySample(genSfx[GEN_COLLECT_COIN],NULL,0,SAMPLE_VOLUME,-1);
					for(i = 0;i < 5;i++)
					{
						tempVect2.vx = tempVect.vx + Random(101) - 50;
						tempVect2.vy = tempVect.vy;
						tempVect2.vz = tempVect.vz + Random(101) - 50;
						fx = CreateSpecialEffectDirect(FXTYPE_SPARKLYTRAIL,&tempVect2,&up,16384*12,16384/2,0,16384);
						if(fx)
							SetFXColour(fx,Random(256),Random(256),Random(256));
					}
				}
				return;
			}
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			LookatWaterfall();
			if(creditsRunning)
			{
				DoCredits();
				DEC_ALPHA(options.subTitle);
				DEC_ALPHA(options.statusBak);
				DEC_ALPHA(options.selectText);
			}
			else
			{
				INC_ALPHA(options.title,255);
				INC_ALPHA(options.titleBak,128);
				INC_ALPHA(options.subTitle,255);
				INC_ALPHA(options.statusBak,128);
				INC_ALPHA(options.title,255);
				if(confirmMode)
				{
					for(i = 0;i < 256;i++)
						warnStr[i] = 0;
					fontFitToWidth(fontSmall,450,GAMESTRING(STR_WARNING_1 + confirmMode - 3),warnStr);
					warnPtr = warnStr;
					for(i = 0;i < 4;i++)
					{
						options.warnText[i]->text = warnPtr;
						warnPtr += strlen(warnPtr) + 1;
						INC_ALPHA(options.warnText[i],255);
					}
					for (i=0; i<numExtrasAvailable; i++)
					{
						options.extras[i]->draw = 1;
						if (i == confirmMode)
						{
							INC_ALPHA(options.extras[i],255);
						}
						else
						{
							DEC_ALPHA(options.extras[i]);
						}
					}
					for(i = 0;i < 2;i++)
					{
						if(i == options.extraSelection)
						{
							INC_ALPHA(options.yesNoText[i],255);
							options.yesNoText[i]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
							options.yesNoText[i]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
							options.yesNoText[i]->b = 10;
						}
						else
						{
							options.yesNoText[i]->r = options.yesNoText[i]->g = options.yesNoText[i]->b = 255;
							if(options.yesNoText[i]->a <= 150)
							{
								INC_ALPHA(options.yesNoText[i],150);
							}
							else
							{
								DEC_ALPHA(options.yesNoText[i]);
							}
						}
					}
				}
				else
				{
					for(i = 0;i < 2;i++)
						DEC_ALPHA(options.yesNoText[i]);
					for (i=0; i<numExtrasAvailable; i++)
					{
						options.extras[i]->draw = 1;
						INC_ALPHA(options.extras[i],255);
						if (i==options.extraSelection)
						{
							options.extras[i]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
							options.extras[i]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
							options.extras[i]->b = 10;
						}
						else
						{
//							if(options.extrasState[i])
//							{
//								options.extras[i]->g = options.extras[i]->b = 0;
//								options.extras[i]->r = 255;
//							}
//							else
								options.extras[i]->r = options.extras[i]->g = options.extras[i]->b = 255;
//							if(options.extras[i]->a <= 150)
//							{
//								INC_ALPHA(options.extras[i],150);
//							}
//							else
//							{
//								DEC_ALPHA(options.extras[i]);
//							}
						}
					}
				}
			}
			break;

		case OP_GLOBALMENU:						
			options.selectText->text = GAMESTRING(STR_X_SELECT_OPTION);
			INC_ALPHA(options.selectText,0xff);
			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			DEC_ALPHA(options.subTitle);
			DEC_ALPHA(options.statusBak);
			options.numPText->draw = options.numPText2->draw = 0;
			// Some random value ;)

			camSource.vx = -7641900;
			camSource.vy = 6178404;
			camSource.vz = -3401826;

			// Look at the options objects
			SetVectorFF(&camTarget,&optTargets[options.selection]);			
			SlideVectorToVectorFF(&currCamTarget,&camTarget,4096*2);
			SlideVectorToVectorFF(&currCamSource,&camSource,4096*2);
			camSpeed3 = 4096*10;
			break;

		case OP_MULTIPLAYERNUMBER:
			maxPlayers = 4;
#ifdef PSX_VERSION
			for(i = 0;i < 4;i++)
			{
				if(padData.present[i] == 0)
				{
					maxPlayers = i;
					break;
				}
			}
#endif
			strcpy(options.titleStr,GAMESTRING(STR_MULTIPLAYER));
			strcpy(options.mpStr,GAMESTRING(STR_SELECT_NUM_PLAYERS));
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			if(maxPlayers <= 1)
			{
				options.numPText->font = fontSmall;
				options.numPText2->font = fontSmall;
				sprintf(options.numPStr,GAMESTRING(STR_NEED_PADS1));
				sprintf(options.numPStr2,GAMESTRING(STR_NEED_PADS2));
			}
			else
			{
				options.numPlayers = min(maxPlayers,max(2,options.numPlayers));
				sprintf(options.numPStr,"%d %s",options.numPlayers,GAMESTRING(STR_PLAYERS));
				options.numPStr2[0] = 0;
			}

			
			temp.vx = -9890938;
			temp.vy = 9046405; 
			temp.vz = 21231148; 
//			temp.vx = -10259420;
//			temp.vy = 12857946; 
//			temp.vz = 19851922; 

			camStill += SlideVectorToVectorFF(&camSource,&temp,mpcamspeed);
			SetVectorFF(&currCamSource,&camSource);
			
//			temp.vx = -10259420 + (7400)*100;
//			temp.vy = 9046405;
//			temp.vz = 19851922 + (39926)*100;

			temp.vx = -9890938 + (7617)*100;
			temp.vy = 9046405;
			temp.vz = 21231148 + (41020)*100;


			camStill += SlideVectorToVectorFF(&camTarget,&temp,mpcamspeed);

			camTarget.vy = camSource.vy;

			SetVectorFF(&currCamTarget,&camTarget);

			camStill = (camStill == 2);

			options.numPText->draw = options.numPText2->draw = camStill;
			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
			options.mpText->draw = 1;
			if(maxPlayers <= 1)
				options.numPText->yPos = 1948;
			else
				options.numPText->yPos = 2048;
	
			break;
		
		case OP_LEVELSEL:
			if((gameSelected) && (fadingOut == 0))
			{
				MPStart();
				return;
			}
			strcpy(options.titleStr,GAMESTRING(STR_MULTIPLAYER));
			strcpy(options.mpStr,GAMESTRING(STR_SELECT_LEVEL));
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));



			StringWrap(GAMESTRING(worldVisualData[multiWorldNum[options.levelSelection]].levelVisualData[multiLevelNum[options.levelSelection]].description_str), 1800, mpLevNameStr, 1024, mpLevNameArray, 2, font);
			options.numPText->text = mpLevNameArray[0];
			options.numPText2->text = mpLevNameArray[1];
//			strcpy(options.numPStr, GAMESTRING(worldVisualData[multiWorldNum[options.levelSelection]].levelVisualData[multiLevelNum[options.levelSelection]].description_str));
	
			if(mpLevNameArray[1][0] == 0)
			{
				options.numPText->yPos = 2048;
			}
			else
			{
				options.numPText->yPos = 2048-120;
				options.numPText2->yPos = 2048+120;
			}
			options.numPText->r = options.numPText2->r = levNameColR[options.levelSelection];
			options.numPText->g = options.numPText2->g = levNameColG[options.levelSelection];
			options.numPText->b = options.numPText2->b = 0;
			
			temp.vx = -9790938;
//			temp.vy = 8859623;  
			temp.vy = -500000;  
			temp.vz = 21231148; 

			camStill += SlideVectorToVectorFF(&camSource,&temp,mpcamspeed);
			SetVectorFF(&currCamSource,&camSource);
			
			temp.vx = -9790938 + (7617)*100;
//			temp.vy = 8859623;
			temp.vy = -500000;
			temp.vz = 21231148 + (41020)*100;

			camStill += SlideVectorToVectorFF(&camTarget,&temp,mpcamspeed);

			camTarget.vy = camSource.vy;

			SetVectorFF(&currCamTarget,&camTarget);

			camStill = (camStill == 2);

			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
			options.mpText->draw = 1;
			options.numPText->draw = options.numPText2->draw = camStill;
	
			break;

		case OP_CHARSEL:
			for(i = 0;i < 4;i++)
			{
				if(i == options.currentPlayer)
				{
					options.playerNum[options.currentPlayer]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
					options.playerNum[options.currentPlayer]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
					options.playerNum[options.currentPlayer]->b = 10;                                           
				}
				else
				{
					options.playerNum[i]->r = 255;
					options.playerNum[i]->g = 255;
					options.playerNum[i]->b = 255;
				}
			}
			strcpy(options.titleStr,GAMESTRING(STR_MULTIPLAYER));
			sprintf(textBuff,"@X - %s %d %s",GAMESTRING(STR_PLAYER),options.currentPlayer + 1,GAMESTRING(STR_SELECT_CHAR));
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			strcpy(options.mpStr,textBuff);
			if(frogPool[options.multiSelection].active)
				strcpy(options.numPStr,GAMESTRING(STR_CHAR_NAME_1 + options.multiSelection));
			else
				strcpy(options.numPStr,"???");

			camStill += SlideVectorToVectorFF(&camSource,&mpCharSelCamSource[options.multiSelection],mpcamspeed);
			camStill += SlideVectorToVectorFF(&camTarget,&mpCharSelCamTarget[options.multiSelection],mpcamspeed);

			camTarget.vy = camSource.vy;

			SetVectorFF(&currCamSource,&camSource);
			SetVectorFF(&currCamTarget,&camTarget);

			camStill = (camStill == 2);

			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
			options.mpText->draw = 1;
			options.numPText->draw = options.numPText2->draw = camStill;
			options.numPText->yPos = 3300;
			options.numPText2->draw = 0;

			break;

		case OP_BOOK:
			strcpy(options.titleStr,GAMESTRING(STR_STORYMODE));
			options.selectText->text = GAMESTRING(STR_X_SELECT_OPTION);
			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.selectText,255);
			if((fadingOut == 0) && (gameSelected))
			{
				BookStart();
				return;
			}

			if(options.page)
			{
#ifdef PC_VERSION
				if (((MDX_ACTOR *)(options.page->actor->actualActor))->animation->reachedEndOfAnimation)
				{
					if(options.pageDir)
					{
						if(options.pageDir > 0)
							grabToTexture = 3;
						else if(options.pageDir < 0)
							grabToTexture = 1;
						options.pageDrawCount = 2;
						options.pageDir = 0;
					}
					options.page->actor->visible = 0;
				}
#endif
			}
			else
			{
				if((DistanceBetweenPointsFF(&currCamSource,&camSource) <= 4096) && (DistanceBetweenPointsFF(&currCamTarget,&camTarget) <= 4096))
				{
					INC_ALPHA(options.chapterPic[0],255);
#ifdef PSX_VERSION
					INC_ALPHA(options.chapterPic[1],255);
					if(pageDir < 0)
					{
						if(clipBox1.x <= 60)
							clipBox1.x += 4;
						if(clipBox2.w <= 60)
							clipBox2.w += 4;
					}
					else if(pageDir > 0)
					{
						if(clipBox1.x >= 4)
							clipBox1.x -= 4;
						if(clipBox2.w >= 4)
							clipBox2.w -= 4;
					}
#else
					DEC_ALPHA(options.chapterPic[1]);
#endif

					options.chapterPic[0]->draw = options.chapterPic[1]->draw = 1;
					
					INC_ALPHA(options.chapterText[0],255);
					DEC_ALPHA(options.chapterText[1]);
					INC_ALPHA(options.chapterNameText[0][0],255);
					INC_ALPHA(options.chapterNameText[0][1],255);
					DEC_ALPHA(options.chapterNameText[1][0]);
					DEC_ALPHA(options.chapterNameText[1][1]);
				}
				if(options.pageDir)
				{
					if(options.chapterText[1]->a == 0)
						options.pageDir = 0;
				}
			}
			break;

		case OP_ARCADE:
			options.selectText->text = GAMESTRING(STR_SELECT_LEVEL);
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			DEC_ALPHA(options.title);
			options.numPText->draw = options.numPText2->draw = 0;
			DoArcadeMenu();
			break;

		case OP_SP_CHARSEL:
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			if((gameSelected) && (fadingOut == 0))
			{
				ArcadeStart();
				return;
			}
			DEC_ALPHA(options.title);
			INC_ALPHA(options.arcadeText,255);
			INC_ALPHA(options.selectText,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.worldBak,128);
			options.numPText->draw = options.numPText2->draw = 0;
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
			for(i = 0;i < FROG_NUMFROGS;i++)
			{
				if(frogPool[i].active)
					INC_ALPHA(options.levelText[i],255);
				if(i == options.spChar)
				{
					options.multiFace[options.spChar]->r = options.levelText[options.spChar]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
					options.multiFace[options.spChar]->g = options.levelText[options.spChar]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
					options.multiFace[options.spChar]->b = options.levelText[options.spChar]->b = 10;
				}
				else
				{
					options.multiFace[i]->r = 255;
					options.multiFace[i]->g = 255;
					options.multiFace[i]->b = 255;
					options.levelText[i]->r = 255;
					options.levelText[i]->g = 255;
					options.levelText[i]->b = 255;
				}
			}
			break;

		case OP_FMV:
			if((playingFMV) && (fadingOut == 0))
			{
#ifdef PSX_VERSION
				SsSetMute( 1 );
#else
				PauseAudio();
#endif
				StartVideoPlayback(options.fmvNum + 3);
#ifdef PSX_VERSION
				SsSetMute( 0 );
#else
				UnPauseAudio( );
#endif
				playingFMV = NO;
				PrepareSong(WORLDID_FRONTEND,YES);
				ScreenFade(0,255,30);
				keepFade = 0;
			}
			strcpy(options.subTitleStr,GAMESTRING(STR_TRIANGLE_BACK));
			INC_ALPHA(options.title,255);
			INC_ALPHA(options.titleBak,128);
			INC_ALPHA(options.subTitle,255);
			INC_ALPHA(options.statusBak,128);
//			options.title->draw = options.titleBak->draw = 1;
			options.numPText->draw = options.numPText2->draw = 0;
			strcpy(options.titleStr,GAMESTRING(STR_EXTRAS_2));
			for(i = 0;i < numFMVOpen;i++)
			{
				if(i == options.fmvNum)
				{
					options.fmvText[i]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
					options.fmvText[i]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
					options.fmvText[i]->b = 10;
				}
				else
				{
					options.fmvText[i]->r = 200;
					options.fmvText[i]->g = 200;
					options.fmvText[i]->b = 200;
				}
			}
			break;
	}
	
	SlurpCamPosition();
	if(goingToArtViewer)
	{
		if(fadingOut == 0)
		{
			gameState.mode = ARTVIEWER_MODE;

#ifdef PC_VERSION
			pFrameModifier = 0;
#else
			SpuSetKey(SPU_OFF,0xffffff);
			StopSong();
#endif

			GTInit(&artTimer,10);
			currentArt = 0;
			goingToArtViewer = 0;
		}
	}
	else if(!playingFMV)
		OptionsProcessController();
}



int CheckCamStill()
{
	long d1;

	d1 = DistanceBetweenPointsFF(&camSource,&currCamSource);
	if ((d1<4096*4) && (d1>-4096*4))
	{
		d1 = DistanceBetweenPointsFF(&camTarget,&currCamTarget);
		if ((d1<4096*3) && (d1>-4096*3))
			return TRUE;
	}
	return FALSE;
}


void UpdateSoundOptions(void)
{
	static oldsOpt = 4;

	options.soundBar[0]->width = globalMusicVol*SOUND_SCALE;
	options.soundBar[1]->width = globalSoundVol*SOUND_SCALE;
	
	if(soundCount > 0)
		soundCount -= gameSpeed;
}


#ifdef PC_VERSION
void SetObjectGouraudValues(MDX_OBJECT *obj,float r,float g,float b)
{
	int j;
	MDX_MESH *mesh = obj->mesh;

	for(j = 0;j < mesh->numFaces*3;j++)
		mesh->d3dVtx[j].color = D3DRGBA(r,g,b,1);

	if(obj->next)
		SetObjectGouraudValues(obj->next,r,g,b);
	if(obj->children)
		SetObjectGouraudValues(obj->children,r,g,b);
}
#endif

#ifdef PC_VERSION
void SetActorGouraudValues(ACTOR2 *actor, int r,int g,int b)
{
	SetObjectGouraudValues(((MDX_ACTOR *)actor->actor->actualActor)->objectController->object,(float)r/255.0,(float)g/255.0,(float)b/255.0);
}
#endif
#ifdef PSX_VERSION
void SetActorGouraudValues(FMA_MESH_HEADER *mesh, int r,int g,int b)
{
	int i;

	register char *opcd asm("$18");

#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		op->r0 = op->r1 = op->r2 = op->r3 = r;
		op->g0 = op->g1 = op->g2 = op->g3 = g;
		op->b0 = op->b1 = op->b2 = op->b3 = b;
	}
#undef op
#undef si
}


void SetActorGouraudValuesMinus(FMA_MESH_HEADER *mesh, int r,int g,int b)
{
	int i;

	register char *opcd asm("$18");

#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		if ( op->r0 > r )
			op->r0 -= r; else op->r0 = 0;
		if ( op->g0 > g )
			op->g0 -= g; else op->g0 = 0;
		if ( op->b0 > b )
			op->b0 -= b; else op->b0 = 0;

		if ( op->r1 > r )
			op->r1 -= r; else op->r1 = 0;
		if ( op->g1 > g )
			op->g1 -= g; else op->g1 = 0;
		if ( op->b1 > b )
			op->b1 -= b; else op->b1 = 0;

		if ( op->r2 > r )
			op->r2 -= r; else op->r2 = 0;
		if ( op->g2 > g )
			op->g2 -= g; else op->g2 = 0;
		if ( op->b2 > b )
			op->b2 -= b; else op->b2 = 0;

		if ( op->r3 > r )
			op->r3 -= r; else op->r3 = 0;
		if ( op->g3 > g )
			op->g3 -= g; else op->g3 = 0;
		if ( op->b3 > b )
			op->b3 -= b; else op->b3 = 0;

	}
#undef op
}


void SetActorGouraudValuesPlus(FMA_MESH_HEADER *mesh, int r,int g,int b, FMA_GT4 *gouraudList )
{
	int i;
	
	FMA_GT4 *gList;

	register char *opcd asm("$18");

#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;

	gList = gouraudList;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		if ( op->r0 < gList->r0 + r )
			op->r0 += r;

		if ( ( op->g0 < gList->g0 + g ) )
			op->g0 += g;

		if ( ( op->b0 < gList->b0 + b ) )
			op->b0 += b;

		if ( ( op->r1 < gList->r1 + r ) )
			op->r1 += r;
		if ( ( op->g1 < gList->g1 + g ) )
			op->g1 += g;
		if ( ( op->b1 < gList->b1 + b ) )
			op->b1 += b;

		if ( ( op->r2 < gList->r2 + r ) )
			op->r2 += r;
		if ( ( op->g2 < gList->g2 + g ) )
			op->g2 += g;
		if ( ( op->b2 < gList->b2 + b ) )
			op->b2 += b;

		if ( ( op->r3 < gList->r3 + r ) )
			op->r3 += r;
		if ( ( op->g3 < gList->g3 + g ) )
			op->g3 += g;
 		if ( ( op->b3 < gList->b3 + b ) )
			op->b3 += b;

		gList++;
	}
#undef op
}

#endif

void BookLeft(void)
{
	if(options.pageNum > 0)
	{
		if(options.pageDir == 0)
		{
			PlaySample( FindSample(utilStr2CRC("page2")), NULL, 0, SAMPLE_VOLUME, -1 );
			options.pageDir = pageDir = -1;					
			options.pageNum--;
			if(options.page)
			{
				grabToTexture = 3;
				actorAnimate(options.page->actor, 2, NO, NO, 99, NO );
				options.page->draw = 1;
			}
			else
			{
				options.chapterPic[1]->draw = 1;
				options.chapterPic[1]->a = options.chapterPic[0]->a;
#ifdef PSX_VERSION
				options.chapterPic[0]->a = 255;
				clipBox1.x = 0;
				clipBox2.w = 0;
				options.chapterPic[0]->flags = SPRITE_CLIP2;
				options.chapterPic[1]->flags = SPRITE_CLIP1;
#else
				options.chapterPic[0]->a = 0;
#endif
				options.chapterPic[1]->tex = options.chapterPic[0]->tex;
				options.chapterPic[0]->tex = FindTexture(chapterPic[options.pageNum]);

				sprintf(chapterStr[0],GAMESTRING(STR_CHAPTER),options.pageNum + 1);
				options.chapterText[1]->draw = 1;
				options.chapterText[1]->a = options.chapterText[0]->a;
				options.chapterText[0]->a = 0;
				sprintf(chapterStr[1],GAMESTRING(STR_CHAPTER),options.pageNum + 2);

				options.chapterNameText[0][0]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2);
				options.chapterNameText[0][1]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2 + 1);
				options.chapterNameText[1][0]->draw = 1;
				options.chapterNameText[1][1]->draw = 1;
				options.chapterNameText[1][0]->a = options.chapterNameText[0][0]->a;
				options.chapterNameText[1][1]->a = options.chapterNameText[0][0]->a;
				options.chapterNameText[0][0]->a = options.chapterNameText[0][1]->a = 0;
				options.chapterNameText[1][0]->text = GAMESTRING(STR_CHAPTER_1a + (options.pageNum + 1)*2);
				options.chapterNameText[1][1]->text = GAMESTRING(STR_CHAPTER_1a + (options.pageNum + 1)*2 + 1);
			}
		}
	}
}

void BookRight(void)
{
	if(options.pageNum < options.maxPageAllowed)
	{
		if (options.pageDir == 0)
		{
			PlaySample( FindSample(utilStr2CRC("page3")), NULL, 0, SAMPLE_VOLUME, -1 );
			options.pageDir = pageDir = 1;
			options.pageNum++;
			if(options.page)
			{
				grabToTexture = 1;
				actorAnimate(options.page->actor, 1, NO, NO, 99, NO );
				options.page->draw = 1;
			}
			else
			{
				options.chapterPic[1]->draw = 1;
				options.chapterPic[1]->a = options.chapterPic[0]->a;
#ifdef PSX_VERSION
				options.chapterPic[0]->a = 255;
				options.chapterPic[0]->flags = SPRITE_CLIP1;
				options.chapterPic[1]->flags = SPRITE_CLIP2;
				clipBox1.x = 64;
				clipBox2.w = 64;
#else
				options.chapterPic[0]->a = 0;
#endif
				options.chapterPic[1]->tex = options.chapterPic[0]->tex;
				options.chapterPic[0]->tex = FindTexture(chapterPic[options.pageNum]);

				sprintf(chapterStr[0],GAMESTRING(STR_CHAPTER),options.pageNum + 1);
				options.chapterText[1]->draw = 1;
				options.chapterText[1]->a = options.chapterText[0]->a;
				options.chapterText[0]->a = 0;
				sprintf(chapterStr[1],GAMESTRING(STR_CHAPTER),options.pageNum);

				options.chapterNameText[0][0]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2);
				options.chapterNameText[0][1]->text = GAMESTRING(STR_CHAPTER_1a + options.pageNum*2 + 1);
				options.chapterNameText[1][0]->draw = 1;
				options.chapterNameText[1][1]->draw = 1;
				options.chapterNameText[1][0]->a = options.chapterNameText[0][0]->a;
				options.chapterNameText[1][1]->a = options.chapterNameText[0][0]->a;
				options.chapterNameText[0][0]->a = options.chapterNameText[0][1]->a = 0;
				options.chapterNameText[1][0]->text = GAMESTRING(STR_CHAPTER_1a + (options.pageNum - 1)*2);
				options.chapterNameText[1][1]->text = GAMESTRING(STR_CHAPTER_1a + (options.pageNum - 1)*2 + 1);
			}
		}
	}
}



long pageToStoryLevel[BOOK_NUM_PAGES] = 
{
	0,2,4,6,8,10,12,14,15,16
};


extern short numLives[3];
void BookSelect(void)
{
	ScreenFade(255,0,30);
	keepFade = 1;
	gameSelected = 1;
	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	player[0].lives = numLives[gameState.difficulty];
}

void BookStart()
{
	NUM_FROGS = 1;
	gameState.multi = SINGLEPLAYER;
	gameState.single = STORY_MODE;

	gameState.storySequenceLevel = pageToStoryLevel[options.pageNum];
	if((options.pageNum == options.maxPageAllowed) && (storySequence[gameState.storySequenceLevel].another))
	{
		if(worldVisualData[storySequence[gameState.storySequenceLevel].worldNum].levelVisualData[storySequence[gameState.storySequenceLevel].levelNum].levelCompleted)
			gameState.storySequenceLevel++;
	}

	player[0].worldNum = (unsigned char)storySequence[gameState.storySequenceLevel].worldNum;
	player[0].levelNum = (unsigned char)storySequence[gameState.storySequenceLevel].levelNum;
		
	lastActFrameCount = actFrameCount;

	gameState.mode = INGAME_MODE;
	InitLevel(player[0].worldNum,player[0].levelNum);


//	actorAnimate(options.book,1,NO,NO,50,NO);

}

void BookDown(void)
{
	player[0].inputPause = (unsigned char)INPUT_POLLPAUSE;
	
	player[0].canJump = 0;
//	player[0].frogState |= FROGSTATUS_ISWANTINGD;
	player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
	frogFacing[0] = (camFacing[0] + MOVE_DOWN) & 3;
	player[0].isSuperHopping = 1;
	player[0].hasJumped = 1;
	fixedDir = 0;
	fixedPos = 0;

	// update player idleTime
	player[0].idleTime = MAX_IDLE_TIME;

	player[0].extendedHopDir = MOVE_DOWN;
	DoneOptionsMenu();
}

void ArcadeUp(void)
{
	int oldLev = options.levelNum;

	if(cWorld == WORLDID_GARDEN)
	{
		options.levelNum = 1 - options.levelNum;
//		if(options.levelNum == 0)
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].texCRC), 1 );
//		else
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].texCRC), 1 );
	}
	else
	{
		do
		{
			options.levelNum = (options.levelNum + worldVisualData[cWorld].numSinglePlayerLevels - 1) MOD worldVisualData[cWorld].numSinglePlayerLevels;
		}while (worldVisualData[cWorld].levelVisualData[options.levelNum].levelOpen == LEVEL_CLOSED);
//		CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[cWorld].levelVisualData[options.levelNum].texCRC), 1 );
	}

	if(options.levelNum != oldLev)
	{
		staticFlash = 5;
		PlaySample( FindSample(utilStr2CRC("espark1")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void ArcadeDown(void)
{
	int oldLev = options.levelNum;

	if(cWorld == WORLDID_GARDEN)
	{
		options.levelNum = 1 - options.levelNum;
//		if(options.levelNum == 0)
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].texCRC), 1 );
//		else
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].texCRC), 1 );
	}
	else
	{
		do
		{
			options.levelNum = (options.levelNum + 1) MOD worldVisualData[cWorld].numSinglePlayerLevels;
		}
		while (worldVisualData[cWorld].levelVisualData[options.levelNum].levelOpen == LEVEL_CLOSED);
//		CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[cWorld].levelVisualData[options.levelNum].texCRC), 1 );
	}
	if(options.levelNum != oldLev)
	{
		staticFlash = 5;
		PlaySample( FindSample(utilStr2CRC("espark1")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void ArcadeLeft(void)
{
	int oldWorld = cWorld;

	do
	{
		cWorld = (cWorld - 1 + NUM_ARCADE_WORLDS) MOD NUM_ARCADE_WORLDS;
	}
	while((worldVisualData[cWorld].worldOpen == WORLD_CLOSED) || (worldVisualData[cWorld].numSinglePlayerLevels == 0));

	if(cWorld == WORLDID_GARDEN)
	{
		if(options.levelNum > 1)
			options.levelNum = 1;
//		if(options.levelNum == 0)
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].texCRC), 1 );
//		else
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].texCRC), 1 );
	}
	else 
	{
		if(options.levelNum > worldVisualData[cWorld].numSinglePlayerLevels-1)
			options.levelNum = worldVisualData[cWorld].numSinglePlayerLevels-1;
		while(worldVisualData[cWorld].levelVisualData[options.levelNum].levelOpen == LEVEL_CLOSED)
			options.levelNum = (options.levelNum + 1) MOD worldVisualData[cWorld].numSinglePlayerLevels;
//		CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[cWorld].levelVisualData[options.levelNum].texCRC), 1 );
	}

	if(cWorld != oldWorld)
	{
		staticFlash = 5;
		PlaySample( FindSample(utilStr2CRC("espark1")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void ArcadeRight(void)
{
	int oldWorld = cWorld;

	do
	{
	 	cWorld = (cWorld + 1) MOD NUM_ARCADE_WORLDS;
	}
	while((worldVisualData[cWorld].worldOpen == WORLD_CLOSED) || (worldVisualData[cWorld].numSinglePlayerLevels == 0));

	if(cWorld == WORLDID_GARDEN)
	{
		if(options.levelNum > 1)
			options.levelNum = 1;
//		if(options.levelNum == 0)
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_FRONTEND].levelVisualData[LEVELID_FRONTEND4].texCRC), 1 );
//		else
//			CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[WORLDID_GARDEN].levelVisualData[LEVELID_GARDEN1].texCRC), 1 );
	}
	else 
	{
		if (options.levelNum > worldVisualData[cWorld].numSinglePlayerLevels-1)
			options.levelNum = worldVisualData[cWorld].numSinglePlayerLevels-1;
		while(worldVisualData[cWorld].levelVisualData[options.levelNum].levelOpen == LEVEL_CLOSED)
			options.levelNum = (options.levelNum + 1) MOD worldVisualData[cWorld].numSinglePlayerLevels;
//		CopyTexture ( arcadeScreenTex, FindTexture(worldVisualData[cWorld].levelVisualData[options.levelNum].texCRC), 1 );
	}

	if(cWorld != oldWorld)
	{
		staticFlash = 5;
		PlaySample( FindSample(utilStr2CRC("espark1")), NULL, 0, SAMPLE_VOLUME, -1 );
	}
}


void SPCharSelect(void)
{
	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	ScreenFade(128,0,30);
	keepFade = 1;
	gameSelected = 1;
	lastArcade = TILENUM_ARCADE;
	CameraSetOffset();
	SetVectorFF(&storeCamSource,&camSource);
	SetVectorFF(&storeCurrCamSource,&currCamSource);
	SetVectorFF(&storeCamTarget,&camTarget);
	SetVectorFF(&storeCurrCamTarget,&currCamTarget);
	SetVectorFF(&storeCamDist,&camDist);
	SetVectorFF(&storeCurrCamDist,&currCamDist);
	SetVectorFF(&storeCamOffset,&camOffset);
	SetVectorFF(&storeCamOffset,&currCamOffset);
	SetVectorFF(&storeCamVect,&camVect);
}

void ArcadeSelect()
{
	int i;

	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	options.mode = OP_SP_CHARSEL;
	options.parText[0]->draw = options.parText[1]->draw = options.parText[2]->draw = 0;
	options.worldText->draw = 0;

	options.worldBak->xPos = options.worldBak->xPosTo = 900;
	options.worldBak->yPos = options.worldBak->yPosTo += 250;
   	options.worldBak->width = 4096 - 2*options.worldBak->xPos;
	options.selectText->text = GAMESTRING(STR_X_SELECT_CHAR);

	for(i = 0;i < MAX_LEVELSTRING;i++)
	{
		options.beatenIcon[i]->draw = 0;
		options.levelCoinMedal[i]->draw = 0;
		options.levelCoinText[i]->draw = 0;
		options.levelParText[i]->draw = 0;
		options.levelSetByText[i]->draw = 0;
		options.levelText[i]->draw = 0;
	}
	for(i = 0;i < FROG_NUMFROGS;i++)
	{
		if(frogPool[i].active)
		{
			sprintf(levelStr[i],GAMESTRING(STR_CHAR_NAME_1 + i));
			options.levelText[i]->draw = 1;
			options.worldBak->height = i*300 + 600;
			options.worldBak->yPos = options.worldBak->yPosTo = 850;
			options.levelText[i]->yPos = options.levelText[i]->yPosTo = i*300 + 1000 + 50;
			options.levelText[i]->xPos = options.levelText[i]->xPosTo = 1600;
			options.multiFace[i]->xPos = options.multiFace[i]->xPosTo = 1000 + (i MOD 2) * 1500;
			options.multiFace[i]->yPos = options.multiFace[i]->yPosTo = i*300 + 1000-128;
			options.multiFace[i]->num = 1;
			options.multiFace[i]->draw = 1;
			if(i == options.spChar)
			{
				options.levelText[options.spChar]->r = 127+((rsin(actFrameCount*4000)+4096)*64)/4096;
				options.levelText[options.spChar]->g = 127+((rcos(actFrameCount*4000)+4096)*64)/4096;
				options.levelText[options.spChar]->b = 10;
			}
			else
			{
				options.levelText[i]->r = 255;
				options.levelText[i]->g = 255;
				options.levelText[i]->b = 255;
			}
		}
	}
}

void SPCharUp(void)
{
	do
	{
		options.spChar = (options.spChar + FROG_NUMFROGS - 1) MOD FROG_NUMFROGS;
	}while(frogPool[options.spChar].active == 0);
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}

void SPCharDown(void)
{
	do
	{
		options.spChar = (options.spChar + 1) MOD FROG_NUMFROGS;
	}while(frogPool[options.spChar].active == 0);
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}

void SPCharBack(void)
{
	int i;

	for(i = 0;i < FROG_NUMFROGS;i++)
		options.multiFace[i]->draw = 0;


	options.mode = OP_ARCADE;
	options.parText[0]->draw = options.parText[1]->draw = options.parText[2]->draw = 1;
	options.worldText->draw = 1;
	options.worldBak->xPos = options.worldBak->xPosTo = 150;
	options.worldBak->yPos = options.worldBak->yPosTo = 938;
	options.worldBak->width = 4096 - 2*options.worldBak->xPos;
	options.selectText->text = GAMESTRING(STR_SELECT_LEVEL);

	for(i = 0;i < MAX_LEVELSTRING;i++)
	{
		levelStr[i][0] = 0;
		options.levelCoinText[i]->draw = 1;
		options.levelParText[i]->draw = 1;
		options.levelSetByText[i]->draw = 1;
		options.levelText[i]->draw = 1;
		options.levelText[i]->xPos = options.levelText[i]->xPosTo = 306;
		options.levelText[i]->yPos = options.levelText[i]->yPosTo = (1445+i*170);
	}
	DoArcadeMenu();
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}

void ArcadeStart(void)
{
	NUM_FROGS = 1;
	gameState.multi = SINGLEPLAYER;
	gameState.single = ARCADE_MODE;

					
	lastActFrameCount = actFrameCount;
	gameState.mode = INGAME_MODE;

	// NB: THis is a heowge hack to make race mode work for internet test levels
	if( cWorld == WORLDID_TEST )
	{
		NUM_FROGS = 2;

		player[0].worldNum = (unsigned char)WORLDID_TEST;
		player[0].levelNum = (unsigned char)options.levelNum;

		player[0].character = FROG_FROGGER;
		player[1].character = FROG_LILLIE;

		gameState.mode = INGAME_MODE;
		gameState.single = INVALID_MODE;
		gameState.multi = MULTILOCAL;

		multiplayerMode = MULTIMODE_RACE;

		InitLevel(player[0].worldNum, player[0].levelNum);
		fog.max = 25000;

		return;
	}
	else if(cWorld == WORLDID_GARDEN)
	{
		if(options.levelNum == 0)
		{
			player[0].worldNum = WORLDID_FRONTEND;
			player[0].levelNum = LEVELID_FRONTEND4;
		}
		else
		{
			player[0].worldNum = WORLDID_GARDEN;
			player[0].levelNum = LEVELID_GARDEN1;
		}
	}
	else
	{
		player[0].worldNum = (unsigned char)cWorld;
		player[0].levelNum = (unsigned char)options.levelNum;			
	}

	player[0].character = options.spChar;
	InitLevel(player[0].worldNum, player[0].levelNum);
}

void ArcadeBack(void)
{
	player[0].inputPause = (unsigned char)INPUT_POLLPAUSE;
	
	player[0].canJump = 0;
//	player[0].frogState |= FROGSTATUS_ISWANTINGD;
	player[0].frogState |= FROGSTATUS_ISWANTINGSUPERHOPD;
//	frogFacing[0] = (camFacing[0] + MOVE_DOWN) & 3;
	player[0].isSuperHopping = 1;
	player[0].hasJumped = 1;
	fixedDir = 0;
	fixedPos = 0;

	// update player idleTime
	player[0].idleTime = MAX_IDLE_TIME;

	player[0].extendedHopDir = MOVE_DOWN;
	ScreenFade(210,255,30);
	fadeText = NO;
	keepFade = NO;


	DoneOptionsMenu();
}

void StartCredits()
{
	int j,spacing = 0;
	psFont *credFont = font;

	creditsY = 0;
	creditsRunning = YES;
	endingCredits = NO;

	for(j = 0;;j++)
	{
		if( ((char*)GAMESTRING(j+STR_CREDITS_1)) [0] == '*' )
			break;

		if((credFont == font) && ( ((char*)GAMESTRING(j+STR_CREDITS_1)) [0] == '_' ))
			credFont = fontSmall;

		spacing += creditData[j].spacing;
#ifdef PSX_VERSION
		creditsText[j] = CreateAndAddTextOverlay(256,min(400,240 + PALMODE*16 + (j + spacing)*CREDIT_SPACING),GAMESTRING(STR_CREDITS_1 + j),YES,255,credFont,TEXTOVERLAY_SHADOW + TEXTOVERLAY_PIXELS);
	 	creditsText[j]->draw = 0;
#else
		creditsText[j] = CreateAndAddTextOverlay(2048,min(10000,4096 + (j + spacing)*CREDIT_SPACING),GAMESTRING(STR_CREDITS_1 + j),YES,255,credFont,TEXTOVERLAY_SHADOW + TEXTOVERLAY_PIXELS);
#endif	 
	 	creditsText[j]->r = creditData[j].r;
		creditsText[j]->g = creditData[j].g;
		creditsText[j]->b = creditData[j].b;
//		creditsText[j]->yPosTo = -1024;
		creditsText[j]->speed = 4096*6;
	}
}

void EndCredits()
{
	int j;

	creditsRunning = NO;

	for(j = 0;;j++)
	{
		if( ((char*)GAMESTRING(j+STR_CREDITS_1)) [0] == '*' )
			break;

		SubTextOverlay(creditsText[j]);
	}
}

void DoCredits()
{
	int j;
	long spacing = 0;

#ifdef PSX_VERSION
	creditsY -= 2;
#else
	creditsY -= (8*gameSpeed)/4096;
#endif

	if(endingCredits)
	{
		for(j = 0;;j++)
		{
			if( ((char*)GAMESTRING(j+STR_CREDITS_1)) [0] == '*' )
				break;
			DEC_ALPHA(creditsText[j]);
		}
		if(creditsText[0]->draw == 0)
			EndCredits();
	}
	else
	{
		DEC_ALPHA(options.title);
		DEC_ALPHA(options.titleBak);
		for(j = 0;j < numExtrasAvailable;j++)
		{
			DEC_ALPHA(options.extras[j]);
		}
	}
	for(j = 0;;j++)
	{
		spacing += creditData[j].spacing;

#ifdef PSX_VERSION
		creditsText[j]->yPos = creditsText[j]->yPosTo = max(-100,min(400,240 + PALMODE*16 + (j + spacing)*CREDIT_SPACING + creditsY));
		if((creditsText[j]->yPos < -20) || (creditsText[j]->yPos > 256))
			creditsText[j]->draw = 0;
		else
			creditsText[j]->draw = 1;
#else
		creditsText[j]->yPos = creditsText[j]->yPosTo = max(-10000,min(10000,4096 + (j + spacing)*CREDIT_SPACING + creditsY));
#endif


		if( ((char*)GAMESTRING(j+STR_CREDITS_1+1)) [0] == '*' )
		{
			if(creditsText[j]->yPos < -CREDIT_SPACING)
				EndCredits();

			break;
		}
	}
}

void FMVUp(void)
{
	if(numFMVOpen > 1)
	{
		options.fmvNum = (options.fmvNum + numFMVOpen - 1) MOD numFMVOpen;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void FMVDown(void)
{
	if(numFMVOpen > 1)
	{
		options.fmvNum = (options.fmvNum + 1) MOD numFMVOpen;
		PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
	}
}

void FMVSelect(void)
{
	ScreenFade(255,0,30);
	playingFMV = YES;
	keepFade = 0;
//	PlaySample(genSfx[GEN_SUPER_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}

void FMVBack(void)
{
	int i;

	options.mode = OP_EXTRA;
	for(i = 0;i < numFMVOpen;i++)
		SubTextOverlay(options.fmvText[i]);
	for(i = 0;i < numExtrasAvailable;i++)
		options.extras[i]->draw = 1;
	PlaySample(genSfx[GEN_FROG_HOP], NULL, 0, SAMPLE_VOLUME, -1 );
}


void SetMusicVolume()
{
#ifdef PC_VERSION
	SetCDVolume((globalMusicVol * 65535)/MAX_SOUND_VOL);
#elif PSX_VERSION
	SpuSetCommonCDVolume((0x7fff*globalMusicVol)/MAX_SOUND_VOL, (0x7fff*globalMusicVol)/MAX_SOUND_VOL);
	xaFileData[0]->vol = xaFileData[1]->vol = globalMusicVol;
#endif
}


#define MAX_ARTWORK 21

#ifdef PSX_VERSION
// on the PSX everything is read from CD .. we just want to do this specially
// on the PC so we don't eat hard drive. Considerate, like. - ds
#define InitCDBackdrop(name)	InitBackdrop(name)
#endif


void RunArtViewer()
{
	char name[32];

	if(padData.debounce[0] & (PAD_START | PAD_TRIANGLE))
		currentArt = MAX_ARTWORK;

	GTUpdate(&artTimer,-1);
	
	if((!fadingOut) && (keepFade))
	{
		FreeBackdrop();
		if(currentArt == MAX_ARTWORK)
		{
			gameState.mode = FRONTEND_MODE;
			InitLevel(WORLDID_FRONTEND,LEVELID_FRONTEND1);

			frameCount = 0;
		}
		else
		{
			sprintf(name,"ARTWORK%02d",currentArt);
			InitBackdrop(name);
			currentArt++;
			ScreenFade(0,255,30);
			GTInit(&artTimer,10);
			keepFade = NO;
		}
	}
	if((artTimer.time == 0) || ((!fadingOut) && (padData.debounce[0])))
	{
		ScreenFade(255,0,30);
		keepFade = YES;
	}
}


#define MAX_TEASERS 6

#ifdef PC_DEMO
void RunTeaserScreens( )
{
	char name[32];

	GTUpdate(&artTimer,-1);
	
	if( !artTimer.time || ((!fadingOut) && (padData.debounce[0])) )
	{
		ScreenFade(255,0,30);
		keepFade = YES;
	}

	if( (!fadingOut) && (keepFade) )
	{
		FreeBackdrop();
		if(currentArt == MAX_TEASERS)
		{
			PostQuitMessage(0);
		}
		else if( currentArt == MAX_TEASERS-1 )
		{
			strcpy( name, "LOADING" );
			if( CheckUS() )
				strcat( name, "US" );
			else
				strcat( name, "EU" );

			InitBackdrop(name);
			currentArt++;
			ScreenFade(0,255,30);
			GTInit(&artTimer,6);
			keepFade = NO;
		}
		else
		{
			sprintf(name,"TEASER%d",currentArt);
			InitBackdrop(name);
			currentArt++;
			ScreenFade(0,255,30);
			GTInit(&artTimer,6);
			keepFade = NO;
		}
	}
}
#endif

#ifdef DREAMCAST_VERSION
void ToggleStereo( )
{
	options.stereoSelectText[options.stereo]->r = options.stereoSelectText[options.stereo]->g = options.stereoSelectText[options.stereo]->b = 255;

	options.stereo = 1-options.stereo;

	options.stereoSelectText[options.stereo]->r = 255;
	options.stereoSelectText[options.stereo]->g = options.stereoSelectText[options.stereo]->b = 0;

	if( options.stereo )
	{
		// JIM: Martin, do your funky sound stuff here, m'kay?
	}
}
#endif
