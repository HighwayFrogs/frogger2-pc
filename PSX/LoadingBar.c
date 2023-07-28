#include "sonylibs.h"
#include "Shell.h"

#include "layout.h"
#include "main.h"
#include "lang.h"
#include "textover.h"
#include "frogger.h"
#include "loadingbar.h"
#include "textures.h"
#include "frontend.h"
#include "fadeout.h"
#include "menus.h"
#include "options.h"
#include "game.h"
#include "multi.h"
#include "story.h"
#include "define.h"

int NUM_WATER_TILESX = 8;
int NUM_WATER_TILESY = 8;

int barProgress = 0;

int barLength = 400;

TEXTOVERLAY *worldName;
TEXTOVERLAY *levelName;
TEXTOVERLAY *loadingText;
TEXTOVERLAY *parTimeText;
TEXTOVERLAY *parNameText;
TEXTOVERLAY *coinsText;
TEXTOVERLAY *playerText[4] = {NULL,NULL,NULL,NULL};
SPRITEOVERLAY *playerFace[4] = {NULL,NULL,NULL,NULL};

SPRITEOVERLAY *backgrounds[4] = {NULL,NULL,NULL,NULL};

static VERT waterPoints[8][8];
static long waterXYs[8][8];
static long waterZs[8][8];
static char waterShade[8][8];

char recordStr[256];
char coinStr[128];
char chaptStr[128];
char playerStr[4][32];
short loadingDisplay = 0;
TextureType *waterTex = NULL;

void loadingTransformPoints()
{
	long loop1, loop2;

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
	
	for(loop1 = 0; loop1 < NUM_WATER_TILESX; loop1 ++)
	{
		for(loop2 = 0; loop2 < NUM_WATER_TILESY; loop2 += 3)
		{
			gte_ldv3(&waterPoints[loop1][loop2], &waterPoints[loop1][loop2 + 1], &waterPoints[loop1][loop2 + 2]);
			gte_rtpt();
			gte_stsxy3(&waterXYs[loop1][loop2], &waterXYs[loop1][loop2 + 1], &waterXYs[loop1][loop2 + 2]);
			gte_stsz3(&waterZs[loop1][loop2], &waterZs[loop1][loop2 + 1], &waterZs[loop1][loop2 + 2]);
		}
	}
}

static GsRVIEW2	loadingCam;

static void setCamera(int vpx,int vpy,int vpz, int vrx,int vry,int vrz)
{
	GsSetProjection(900);
	loadingCam.vpx = vpx;
	loadingCam.vpy = vpy;
	loadingCam.vpz = vpz;
	loadingCam.vrx = vrx;
	loadingCam.vry = vry;
	loadingCam.vrz = vrz;
	GsSetRefView2L(&loadingCam);
  	SetGeomOffset(0, 0);
}

int loadwaterx = 1000;
int loadwatery = 1000;
void loadingInitPolys()
{
	int i,c;

	for ( i = 0; i < NUM_WATER_TILESX; i++ )
	{
		for ( c = 0; c < NUM_WATER_TILESY; c++ )
		{
			waterPoints[i][c].vx = ((i-NUM_WATER_TILESX/2)*(1200*8))/NUM_WATER_TILESX + (loadwaterx*NUM_WATER_TILESX)/8;
			waterPoints[i][c].vy = ((c-NUM_WATER_TILESY/2)*(1000*8))/NUM_WATER_TILESY + (loadwatery*NUM_WATER_TILESY)/8;
			waterPoints[i][c].vz = 0;
		}
	}
}

int fontsLoaded;

void loadingInitText(int worldID, int levelID)
{
	int i,c,y = 400;

	worldName = CreateAndAddTextOverlay( 2048 + 4096, y + 52, chaptStr, YES, 255, fontSmall, TEXTOVERLAY_LOADING); 

	if(gameState.mode == FRONTEND_MODE)
		chaptStr[0] = 0;
	else if((gameState.single == STORY_MODE) && (gameState.multi == SINGLEPLAYER))
	{
		sprintf(chaptStr,"%d - %s %s",storySequenceLevelToChapter[gameState.storySequenceLevel] + 1,GAMESTRING(STR_CHAPTER_1a + storySequenceLevelToChapter[gameState.storySequenceLevel]*2),GAMESTRING(STR_CHAPTER_1b + storySequenceLevelToChapter[gameState.storySequenceLevel]*2));
		backgrounds[0] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,400,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
		backgrounds[0]->r = backgrounds[0]->g = backgrounds[0]->b = 128;
		y += 700;
	}
	else
		chaptStr[0] = 0;

	worldName->r = 0;
	worldName->b = 0;
	worldName->xPosTo = 2048;
	worldName->speed = 4096*40*3;

	if(gameState.mode == FRONTEND_MODE)
	{
		y = 1900;
		levelName = CreateAndAddTextOverlay( 2048 - 4096*2, y, GAMESTRING(STR_LOADING), YES, 255, font, TEXTOVERLAY_LOADING); 
		loadingText = NULL;
	}
	else
	{
		loadingText = CreateAndAddTextOverlay( 2048 + 4096*2, y, GAMESTRING(STR_LOADING), YES, 255, fontSmall, TEXTOVERLAY_LOADING); 
		y += 300;
		levelName = CreateAndAddTextOverlay( 2048 - 4096*2, y, GAMESTRING(worldVisualData[worldID].levelVisualData[levelID].description_str), YES, 255, font, TEXTOVERLAY_LOADING); 
		loadingText->xPosTo = 2048;
		loadingText->speed = 4096*40*3;
	}


	backgrounds[1] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,400,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
	backgrounds[1]->r = backgrounds[1]->g = backgrounds[1]->b = 128;

	if(gameState.multi == SINGLEPLAYER)
		y += 1100;
	else
		y += 600;

	levelName->r = 0;
	levelName->b = 0;
	levelName->xPosTo = 2048;
	levelName->speed = 4096*40*3;

	parTimeText = CreateAndAddTextOverlay( 2048 + 4096*3, y, recordStr, YES, 255, font, TEXTOVERLAY_LOADING);
	if(gameState.mode == FRONTEND_MODE)
		recordStr[0] = 0;
	else if(gameState.multi == SINGLEPLAYER) 
	{
		if(gameState.mode == DEMO_MODE)
		{
			backgrounds[2] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,400,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
			backgrounds[2]->r = backgrounds[2]->g = backgrounds[2]->b = 128;
			sprintf(recordStr,GAMESTRING(STR_DEMO_MODE));
		}
		else if((gameState.single == ARCADE_MODE) && (worldID != WORLDID_FRONTEND))
		{
			sprintf(recordStr,GAMESTRING(STR_RECORD),worldVisualData[worldID].levelVisualData[levelID].parName,((int)worldVisualData[worldID].levelVisualData[levelID].parTime/600)%600,((int)worldVisualData[worldID].levelVisualData[levelID].parTime/10)%60,((int)worldVisualData[worldID].levelVisualData[levelID].parTime)%10);
			if(worldID == WORLDID_SUPERRETRO)
				backgrounds[2] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,400,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
			else
				backgrounds[2] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,800,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
			backgrounds[2]->r = backgrounds[2]->g = backgrounds[2]->b = 128;
			y += 400;
		}
		else
			recordStr[0] = 0;
	}
	else
	{
//		recordStr[0] = 0;
		sprintf(recordStr,GAMESTRING(STR_MULTI_DESC_1 + multiGameTypes[player[0].worldNum] - 1),worldVisualData[worldID].levelVisualData[levelID].parName,((int)worldVisualData[worldID].levelVisualData[levelID].parTime/60)%60,((int)worldVisualData[worldID].levelVisualData[levelID].parTime)%60);
		backgrounds[2] = CreateAndAddSpriteOverlay(0,y - 30,NULL,4096,400,0,SPRITE_SUBTRACTIVE | SPRITE_LOADING);
		backgrounds[2]->r = backgrounds[2]->g = backgrounds[2]->b = 128;

		y += 500;
		for(i = 0;i < NUM_FROGS;i++)
		{
			sprintf(playerStr[i],"%s %d",GAMESTRING(STR_PLAYER),i + 1);
			c = (fontExtentWScaled(font,playerStr[i],4096)+96)*4;
			playerText[i] = CreateAndAddTextOverlay(2048 - c - 4096*(4+i) - 128 + 256*(i MOD 2),y + i*550,playerStr[i],NO,255,font,TEXTOVERLAY_LOADING);
			playerText[i]->xPosTo = 2048 - c - 128 + 256*(i MOD 2);
			playerText[i]->speed = 4096*40*3;
			playerText[i]->draw = 0;
			playerFace[i] = CreateAndAddSpriteOverlay(2048 + c - 64*8 + 4096*(4 +i) - 128 + 256*(i MOD 2),y + i*550 - 128,NULL,4096,1,255,SPRITE_LOADING);
			playerFace[i]->xPosTo = 2048 + c - 64*8 - 128 + 256*(i MOD 2);
			playerFace[i]->speed = 4096*40*3;
			playerFace[i]->draw = 0;
		}
	}
	parTimeText->xPosTo = 2048;
	parTimeText->speed = 4096*40*3;


	if(gameState.mode == FRONTEND_MODE)
		coinStr[0] = 0;
	else if(gameState.multi == SINGLEPLAYER)
	{
		if((gameState.mode != DEMO_MODE) && (gameState.single == ARCADE_MODE) && (worldID != WORLDID_FRONTEND) && (worldID != WORLDID_SUPERRETRO))
			sprintf ( coinStr, "%s : %d",GAMESTRING(STR_COINS), worldVisualData[worldID].levelVisualData[levelID].maxCoins );
		else
			coinStr[0] = 0;
	}
	else
		coinStr[0] = 0;
	coinsText = CreateAndAddTextOverlay( 2048 - 4096*4, y, coinStr, YES, 255, font, TEXTOVERLAY_LOADING);
	coinsText->xPosTo = 2048;
	coinsText->speed = 4096*40*3;
}

void loadingInit ( int worldID, int levelID )
{
	int i, c;
	RECT rect;
	int y = 500;

	fontsLoaded = NO;
	if(NUM_FROGS == 4)
		NUM_WATER_TILESX = NUM_WATER_TILESY = 6;
	else
		NUM_WATER_TILESX = NUM_WATER_TILESY = 8;

	waterTex = NULL;
	LoadTextureBank ( LOADING_TEX_BANK );
	loadingDisplay = 1;

	loadFrameCount = 0;
	ScreenFade(0,255,30);
	keepFade = NO;

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

//	if ( worldID == 8 )
//		return;

	setCamera(0,0,-12000, 0,0,0);

/*
	rect.x = 1024-64;
	rect.y = 260;
	rect.w = 32;
	rect.h = 64;

	DrawSync(0);
	LoadImage (&rect, (unsigned long*)waterimage );

	rect.x = 1024-64;
	rect.y = 258;
	rect.w = 16;
	rect.h = 1;

	DrawSync(0);
	LoadImage (&rect, (unsigned long*)waterpal );
*/
	loadingInitPolys();

}

void loadingFree()
{
	int i;

	for(i = 0;i < 4;i++)
	{
		SubSpriteOverlay(backgrounds[i]);
		backgrounds[i] = NULL;
		SubSpriteOverlay(playerFace[i]);
		playerFace[i] = NULL;
		SubTextOverlay(playerText[i]);
		playerText[i] = NULL;
	}

	loadingDisplay = 0;
	frameCount = 1;
	gameSpeed = 4096;
	SubTextOverlay(worldName);
	SubTextOverlay(levelName);
	if(loadingText)
		SubTextOverlay(loadingText);
	SubTextOverlay(parTimeText);
	SubTextOverlay(coinsText);
	GsSetProjection(350);
	waterTex = NULL;
}

int loadwatershadefac = 20;
int loadwatershadeval = 128;
int loadFrameCount = 0;
int wateru0,wateru1,wateru2,waterv0,waterv1,waterv2;
int loadcamrotspeed = 500;
void loadingWaterFrame ( void )
{
	int i, c;
	register PACKET*		packet;
	RECT rect;
	int temp;
	

	if(fontsLoaded == NO)
	{
		if((fontSmall) && (font))
		{
			fontsLoaded = YES;
//			for(i = 0;i < NUM_FROGS;i++)
//				playerText[i]->font = font;
			loadingInitText(player[0].worldNum,player[0].levelNum);
//			worldName->font = fontSmall;
//			levelName->font = font;
//			parTimeText->font = font;
//			coinsText->font = font;
//			worldName->draw = 1;
//			levelName->draw = 1;
//			parTimeText->draw = 1;
//			coinsText->draw = 1;
		}
	}
	else
	{
		for(i = 0;i < 4;i++)
		{
			if(backgrounds[i])
				INC_ALPHA(backgrounds[i],254);
		}
		if(textureBanks[1])
		{
			for(i = 0;i < NUM_FROGS;i++)
			{
				if((playerFace[i]) && (playerFace[i]->draw == 0))
				{
					playerFace[i]->tex = FindTexture(frogPool[player[i].character].icon);
					if(playerFace[i]->tex)
						playerFace[i]->draw = playerText[i]->draw = 1;
				}
			}
		}
	}


	++loadFrameCount;
	DrawScreenTransition();
	PrintTextOverlays();
	PrintSpriteOverlays(0);

	if(waterTex == NULL)
	{
		waterTex = FindTexture("LOADWAT");
		if(!waterTex)
			return;
		else
		{
			wateru0 = waterTex->u0;
			wateru1 = (waterTex->u0 + waterTex->u1)/2;
			wateru2 = waterTex->u3;
			waterv0 = waterTex->v0;
			waterv1 = (waterTex->v0 + waterTex->v2)/2;
			waterv2 = waterTex->v3;
		}
	}

	loadingTransformPoints();

	
	for ( i = 0; i <NUM_WATER_TILESX - 1; i++ )
	{
		for ( c = 0; c <NUM_WATER_TILESY - 1; c++ )
		{
			waterPoints[i][c].vz = (rcos((loadFrameCount<<5) + ((waterPoints[i][c].vx) | (waterPoints[i][c].vy)))>>2);
		}
	}
	for ( i = 0; i <NUM_WATER_TILESX - 1; i++ )
	{
		for ( c = 0; c <NUM_WATER_TILESY - 1; c++ )
		{
			temp = (waterPoints[i][c].vz)/loadwatershadefac + loadwatershadeval;
			if (temp > 180) temp = 180;
			if (temp < 0) temp = 0;
			waterShade[i][c] = 255-temp;
		}
	}

	for ( i = 0; i <NUM_WATER_TILESX - 1; i++ )
	{
		for ( c = 0; c <NUM_WATER_TILESY - 1; c++ )
		{
#define si ((POLY_GT4*)packet)
			BEGINPRIM(si, POLY_GT4);

			setPolyGT4(si);

	
			si->r0 = waterShade[i][c];
			si->g0 = waterShade[i][c];
			si->b0 = 180;//waterShade[i][c];
			si->r1 = waterShade[i][c+1];
			si->g1 = waterShade[i][c+1];
			si->b1 = 180;//waterShade[i][c+1];
			si->r2 = waterShade[i+1][c];
			si->g2 = waterShade[i+1][c];
			si->b2 = 180;//waterShade[i+1][c];
			si->r3 = waterShade[i+1][c+1];
			si->g3 = waterShade[i+1][c+1];
			si->b3 = 180;//waterShade[i+1][c+1];

			*(long *)&si->x0 = *(long *)&waterXYs[i][c];
			*(long *)&si->x1 = *(long *)&waterXYs[i][c + 1];
			*(long *)&si->x2 = *(long *)&waterXYs[i + 1][c];
			*(long *)&si->x3 = *(long *)&waterXYs[i + 1][c + 1];


			if((c % 2) == 0)
			{
				si->u0 = si->u2 = wateru0;
				si->u1 = si->u3 = wateru1;
//				si->u2 = waterTex->u2;
//				si->u3 = waterTex->u3;
			}
			else
			{
				si->u0 = si->u2 = wateru1;
				si->u1 = si->u3 = wateru2;
			}

			if((i % 2) == 0)
			{
				si->v0 = si->v1 = waterv0;
				si->v2 = si->v3 = waterv1;
			}
			else
			{
				si->v0 = si->v1 = waterv1;
				si->v2 = si->v3 = waterv2;
			}


			si->code = GPU_COM_TG4;

/*
			rect.x = 1024-64;
			rect.y = 260;
			rect.w = 32;
			rect.h = 64;
			si->tpage = getTPage ( 0,0, rect.x, rect.y );

			rect.x = 1024-64;
			rect.y = 258;
			rect.w = 16;
			rect.h = 1;

			si->clut	= getClut(rect.x,rect.y);
*/
			si->clut = waterTex->clut;
			si->tpage = waterTex->tpage;


			ENDPRIM(si, 100, POLY_GT4);
#undef si
		}
	}
}

void loadingDisplayFrame ( void )
{
//	if ( player[0].worldNum == 8 )
//		return;


	gameSpeed = 4096;
	loadingWaterFrame();
}

void StepProgressBar ( int percentAmount )
{
	// JH: Add the required amount onto the bar progress....
	barProgress += percentAmount;

	// JH: Just for safety, let's make sure we can't go above 100%....
	if ( barProgress > 100 )
		barProgress = 100;
	// ENDIF
}

void SetProgressBar ( int percentage )
{
	// JH: Equal the required amount into the bar progress....
	barProgress = percentage;

	// JH: Just for safety, let's make sure some fool hasn't enter a wrong value....
	if ( barProgress > 100 )
		barProgress = 100;

	if ( barProgress < 0 )
		barProgress = 0;
}

void DrawLoadingBar ( int execute )
{
	register PACKET*		packet;

#define si ((POLY_G4*)packet)

	if ( execute )
	{
		currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[0]):(&displayPage[1]);
		ClearOTagR(currentDisplayPage->ot, 1024);
		currentDisplayPage->primPtr = currentDisplayPage->primBuffer;

//		DrawSync(0);
// 		VSync(2);
 		/*PutDispEnv(&currentDisplayPage->dispenv);
 		PutDrawEnv(&currentDisplayPage->drawenv);
 		DrawOTag(currentDisplayPage->ot+(1024-1));*/
	}
	// ENDIF

	BEGINPRIM(si, POLY_G4);

	setPolyG4(si);

	si->r0 = 255;
	si->g0 = 0;
	si->b0 = 0;

	si->r1 = 0;
	si->g1 = 0;
	si->b1 = 255;

	si->r2 = 255;
	si->g2 = 0;
	si->b2 = 0;

	si->r3 = 0;
	si->g3 = 0;
	si->b3 = 255;


	si->x0 = -200;
	si->y0 = 100;

	si->x2 = -200;
	si->y2 = 105;

	si->x1 = -200 + ((barLength * barProgress) / 100 );
	si->y1 = 100;

	si->x3 = -200 + ((barLength * barProgress) / 100 );
	si->y3 = 105;

	si->code = GPU_COM_G4;

	ENDPRIM(si, 1, POLY_G4);


	BEGINPRIM(si, POLY_G4);

	setPolyG4(si);

	si->r0 = 0;
	si->g0 = 0;
	si->b0 = 0;

	si->r1 = 0;
	si->g1 = 255;
	si->b1 = 0;

	si->r2 = 0;
	si->g2 = 0;
	si->b2 = 0;

	si->r3 = 0;
	si->g3 = 0;
	si->b3 = 0;


	si->x0 = -205;
	si->y0 = 97;

	si->x2 = -205;
	si->y2 = 108;

	si->x1 = -205 + barLength + 12;
	si->y1 = 97;

	si->x3 = -205 + barLength + 12;
	si->y3 = 108;

	si->code = GPU_COM_G4;

	ENDPRIM(si, 10, POLY_G4);


	if ( execute )
	{
/* 			DrawSync(0);
 			currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
			ClearOTagR(currentDisplayPage->ot, 1024);
 			currentDisplayPage->primPtr = currentDisplayPage->primBuffer;
 			DrawSync(0);

 			DrawSync(0);
 			VSync(2);
 			PutDispEnv(&currentDisplayPage->dispenv);
 			PutDrawEnv(&currentDisplayPage->drawenv);
 			DrawOTag(currentDisplayPage->ot+(1024-1));

 			DrawSync(0);*/
	}
	// ENDIF
#undef si
}

