/******************************************************************************************
	AM2 PS   (c) 2000-1 ISL		-=Modified for Chicken Run=-

	language.c:		Language selection
******************************************************************************************/

//#include "global.h"
#include "cr_lang.h"
#include "sonylibs.h"
#include "shell.h"
#include <islpad.h>
#include "main.h"
#include <islmem.h>
#include "fadeout.h"
#include "textures.h"
#include "layout.h"
#include "audio.h"
//#include "menu.h"

#define FLAGPOINTS 36
#define FLAGPOLYS FLAGPOINTS-1

LanguageDataType languageData;

int DoneLangSel = 0;

static char *buttonName[] = 
{
	"BUT_LEFT",
	"BUT_RIGH",
	"CONTROL2",
};

static char *flagName[] =
{
	"UK",
	"FRENCH",
	"GERMANY",
	"ITALY",
	"DUTCH",
};


static char *langName[] = {
	"English",
	"Français",
	"Deutsch",
	"Italiano",
	"Nederlands",
};

static VERT flagPoints[FLAGPOINTS][FLAGPOINTS];
static long screenXYs[FLAGPOINTS][FLAGPOINTS];
static long screenZs[FLAGPOINTS][FLAGPOINTS];

static long wind[FLAGPOINTS*2];
static long	crosswind[FLAGPOINTS];
static long accel[FLAGPOINTS][FLAGPOINTS];
unsigned char flagShade[FLAGPOINTS][FLAGPOINTS];

unsigned char textureU[FLAGPOLYS][FLAGPOLYS][4];
unsigned char textureV[FLAGPOLYS][FLAGPOLYS][4];
unsigned char tileTextureNum;

static char enable;
static short fadeDirection,finishing;
static GsRVIEW2	langCam;

static void setCamera(int vpx,int vpy,int vpz, int vrx,int vry,int vrz)
{
	GsSetProjection(900);
	langCam.vpx = vpx;
	langCam.vpy = vpy;
	langCam.vpz = vpz;
	langCam.vrx = vrx;
	langCam.vry = vry;
	langCam.vrz = vrz;
	GsSetRefView2L(&langCam);
  	SetGeomOffset(0, 0);
}


static void languageTransformPoints()
{
	long loop1, loop2;

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
	
	for(loop1 = 0; loop1 < FLAGPOINTS; loop1 ++)
	{
		for(loop2 = 0; loop2 < FLAGPOINTS; loop2 += 3)
		{
			gte_ldv3(&flagPoints[loop1][loop2], &flagPoints[loop1][loop2 + 1], &flagPoints[loop1][loop2 + 2]);
			gte_rtpt();
			gte_stsxy3(&screenXYs[loop1][loop2], &screenXYs[loop1][loop2 + 1], &screenXYs[loop1][loop2 + 2]);
			gte_stsz3(&screenZs[loop1][loop2], &screenZs[loop1][loop2 + 1], &screenZs[loop1][loop2 + 2]);
		}
	}
}

static void languageDrawPolys()
{
	long		loop1, loop2;
//	uchar		rgb;
	unsigned char		rgb;
	POLY_GT4	*gt4;
	POLY_G4		*g4;

	BEGINPRIM(g4, POLY_G4);
	setPolyG4(g4);
	setXYWH(g4, -256,-128, 512,256);
	rgb = rsin(languageData.frame*24)/70;
	setRGB0(g4, (150+rgb),(60+rgb),0);
	rgb = rsin(languageData.frame*24+1000)/70;
	setRGB1(g4, (150+rgb),(60+rgb),0);
	rgb = rsin(languageData.frame*24+2000)/70;
	setRGB2(g4, (150+rgb),(60+rgb),0);
	rgb = rsin(languageData.frame*24+3000)/70;
	setRGB3(g4, (150+rgb),(60+rgb),0);
	ENDPRIM(g4, 200, POLY_G4);

	for(loop1 = 0; loop1 < FLAGPOLYS; loop1 ++)
	{
		for(loop2 = 0; loop2 < FLAGPOLYS; loop2 ++)
		{
			BEGINPRIM(gt4, POLY_GT4);
			setPolyGT4(gt4);
			gt4->tpage = languageData.flagTexture[tileTextureNum]->tpage;
			gt4->clut = languageData.flagTexture[tileTextureNum]->clut;

			*(long *)&gt4->x0 = *(long *)&screenXYs[loop1][loop2];
			*(long *)&gt4->x1 = *(long *)&screenXYs[loop1][loop2 + 1];
			*(long *)&gt4->x2 = *(long *)&screenXYs[loop1 + 1][loop2];
			*(long *)&gt4->x3 = *(long *)&screenXYs[loop1 + 1][loop2 + 1];

			gt4->r0 = flagShade[loop1][loop2];
			gt4->g0 = flagShade[loop1][loop2];
			gt4->b0 = flagShade[loop1][loop2];

			gt4->r1 = flagShade[loop1][loop2 + 1];
			gt4->g1 = flagShade[loop1][loop2 + 1];
			gt4->b1 = flagShade[loop1][loop2 + 1];

			gt4->r2 = flagShade[loop1 + 1][loop2];
			gt4->g2 = flagShade[loop1 + 1][loop2];
			gt4->b2 = flagShade[loop1 + 1][loop2];

			gt4->r3 = flagShade[loop1 + 1][loop2 + 1];
			gt4->g3 = flagShade[loop1 + 1][loop2 + 1];
			gt4->b3 = flagShade[loop1 + 1][loop2 + 1];

			gt4->u0 = textureU[loop1][loop2][0];
			gt4->v0 = textureV[loop1][loop2][0];

			gt4->u1 = textureU[loop1][loop2][1];
			gt4->v1 = textureV[loop1][loop2][1];

			gt4->u2 = textureU[loop1][loop2][2];
			gt4->v2 = textureV[loop1][loop2][2];

			gt4->u3 = textureU[loop1][loop2][3];
			gt4->v3 = textureV[loop1][loop2][3];

//			setSemiTrans(gt4, 1);
//			SETSEMIPRIM(gt4, SEMITRANS_SEMI);

			ENDPRIM(gt4,100, POLY_GT4);
		}
	}
}

void calculateTileTexture(int x, int y)
{
	if(tileTextureNum<LANG_NUMLANGS)
	{
		textureU[x][y][0] = textureU[x][y][1] = languageData.flagTexture[tileTextureNum]->u0
											  + ((languageData.flagTexture[tileTextureNum]->u3 + 1
											  - languageData.flagTexture[tileTextureNum]->u0)*x)/FLAGPOLYS + 1;
		textureU[x][y][2] = textureU[x][y][3] = languageData.flagTexture[tileTextureNum]->u0
											  + ((languageData.flagTexture[tileTextureNum]->u3 + 1
											  - languageData.flagTexture[tileTextureNum]->u0)*(x+1))/FLAGPOLYS + 1;

		textureV[x][y][0] = textureV[x][y][2] = languageData.flagTexture[tileTextureNum]->v0
											  + ((languageData.flagTexture[tileTextureNum]->v3 + 1 
											  - languageData.flagTexture[tileTextureNum]->v0)*y)/FLAGPOLYS + 1;
		textureV[x][y][1] = textureV[x][y][3] = languageData.flagTexture[tileTextureNum]->v0
											  + ((languageData.flagTexture[tileTextureNum]->v3 + 1
											  - languageData.flagTexture[tileTextureNum]->v0)*(y+1))/FLAGPOLYS + 1;
	}
	else
	{
		textureU[y][x][2] = textureU[y][x][0] = languageData.flagTexture[tileTextureNum]->u0
											  + ((languageData.flagTexture[tileTextureNum]->u3 
											  - languageData.flagTexture[tileTextureNum]->u0)*x)/FLAGPOLYS + 1;
		textureU[y][x][3] = textureU[y][x][1] = languageData.flagTexture[tileTextureNum]->u0
											  + ((languageData.flagTexture[tileTextureNum]->u3 
											  - languageData.flagTexture[tileTextureNum]->u0)*(x+1))/FLAGPOLYS + 1;

		textureV[y][x][1] = textureV[y][x][0] = languageData.flagTexture[tileTextureNum]->v0
											  + ((languageData.flagTexture[tileTextureNum]->v3 
											  - languageData.flagTexture[tileTextureNum]->v0)*y)/FLAGPOLYS + 1;
		textureV[y][x][3] = textureV[y][x][2] = languageData.flagTexture[tileTextureNum]->v0
											  + ((languageData.flagTexture[tileTextureNum]->v3 
											  - languageData.flagTexture[tileTextureNum]->v0)*(y+1))/FLAGPOLYS + 1;
	}
}

unsigned char* imagePtr;
RECT tex;
TextureType face;

void languageInitialise()
{
	long loop,loop1,loop2;

	utilPrintf("Just Before screen Fade.\n");
	ScreenFade(0,255,30);
	keepFade = 0;


//	utilPrintf("Just Before font Load.\n");
//	font = fontLoad("BIGFONT.FON");

//	utilPrintf("Loaded the Font file.\n");

	imagePtr=0;
	tex.x = 960;
	tex.y = 384;
	tex.w = 64;
	tex.h = 128;

	face.tpage = getTPage(2,0,960,256);
	face.clut = 0;
	face.u0 = 63; face.v0 = 128;
	face.u1 = 63; face.v1 = 255;
	face.u2 =  0; face.v2 = 128;
	face.u3 =  0; face.v3 = 255;

	languageData.frame = 0;
	languageData.lastFade = 0;

	enable = 0;

	// setup stuff goes here, malloc memory or structs whatever
//	languageData.flagTextures = textureLoadBank("FRONTEND\\LANGUAGE.SPT");
	languageData.flagTextures = textureLoadBank("TEXTURES\\LANGUAGE.SPT");

	//bb
//	textureDownloadBank(languageData.flagTextures);

	utilPrintf("loaded ok");
	for (loop = 0; loop < LANG_NUMLANGS; loop++)
		languageData.flagTexture[loop] = textureFindInBank(languageData.flagTextures,flagName[loop]);
	for (loop = LANG_NUMLANGS; loop < LANG_NUMLANGS+BONUS_FLAGS; loop++)
		languageData.flagTexture[loop] = &face;

	setCamera(0,0,-12000, 0,0,0);

	//Movement stuff
	for(loop = 0; loop < FLAGPOINTS; loop ++)
	{
		wind[loop] = 0;
		crosswind[loop] = 0;
	}

	//Position and movement stuff
	for(loop1 = 0; loop1 < FLAGPOINTS; loop1 ++)
	{
		for(loop2 = 0; loop2 < FLAGPOINTS; loop2 ++)
		{
			accel[loop1][loop2] = 0;
			flagPoints[loop1][loop2].vx = (loop1-FLAGPOINTS/2)*200;
			flagPoints[loop1][loop2].vy = (loop2-FLAGPOINTS/2)*100 - 500;
			if (loop1 > 0) 
				flagPoints[loop1][loop2].vz = loop1-loop2;
			else
				flagPoints[loop1][loop2].vz = 0;
		}
	}

	tileTextureNum = gameTextLang;
	// Texture position stuff
	for(loop1 = 0; loop1 < FLAGPOLYS; loop1 ++)
		for(loop2 = 0; loop2 < FLAGPOLYS; loop2 ++)
			calculateTileTexture(loop1,loop2);

//	gameTextLang=LANG_UK;//no language select on american version - tossers
//	gameInfo.destFadeLevel = 0;
	finishing = 0;

//	setProgress(PROGRESS_NONE);
//	loadingDisplay = 0;

}

void languageFrame()
{
	long loop1,loop2,temp;
		

//	if(languageData.frame < 20)
//		ScreenFade(0,255,30);

//	utilPrintf("Frame\n");
	wind[0] += utilRandomInt(800)-100+rsin(languageData.frame*200)/15;
	wind[0] = wind[0]/2;
	for(loop1 = FLAGPOINTS*2-1; loop1 > 0; loop1 --)
		wind[loop1] = wind[loop1-1];

	crosswind[0] += utilRandomInt(900)+rsin(languageData.frame*170)/15;
	crosswind[0] = wind[0]/2;
	for(loop1 = FLAGPOINTS-1; loop1 > 0; loop1 --)
		crosswind[loop1] = crosswind[loop1-1];

	setCamera(languageData.lastFade*500*fadeDirection,0,-12000+(long)finishing*128,	languageData.lastFade * 500 * fadeDirection,0,0);

	if (padData.debounce[0] & PAD_LEFT && !languageData.lastFade && !finishing)
	{
		gameTextLang = (gameTextLang+(LANG_NUMLANGS-1)-1)%(LANG_NUMLANGS-1);
		languageData.lastFade = 1;
		fadeDirection = -1;
		PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
	}
	if (padData.debounce[0] & PAD_RIGHT && !languageData.lastFade && !finishing)
	{
		gameTextLang = (gameTextLang+1)%(LANG_NUMLANGS-1);
		languageData.lastFade = 1;
		fadeDirection = 1;
		PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
	}

	if (languageData.lastFade) languageData.lastFade++;
	if (languageData.lastFade>16)
	{
		tileTextureNum = gameTextLang;
		languageData.lastFade=-16;
		for(loop1 = 0; loop1 < FLAGPOLYS; loop1 ++)
			for(loop2 = 0; loop2 < FLAGPOLYS; loop2 ++)
				calculateTileTexture(loop1,loop2);
	}

	for(loop1 = 0; loop1 < FLAGPOINTS; loop1 ++)
	{
		for(loop2 = 0; loop2 < FLAGPOINTS; loop2 ++)
		{
			if (loop1 == 0)
				accel[loop1][loop2] = (accel[loop1][loop2] + wind[loop2+loop1] + crosswind[loop1]) / 3;
			else
				accel[loop1][loop2] = (accel[loop1-1][loop2] + (wind[loop2+loop1]/2) + crosswind[loop1]) / 2;
			if (!(loop1 + languageData.frame)) accel[loop1][loop2] += utilRandomInt(128)-64;

			flagPoints[loop1][loop2].vz += accel[loop1][loop2];
			flagPoints[loop1][loop2].vz = (flagPoints[loop1][loop2].vz*9)/10;
			temp = (flagPoints[loop1][loop2].vz/*+2000*/)/20;
			if (temp > 180) temp = 180;
			if (temp < 0) temp = 0;
			flagShade[loop1][loop2] = 255-temp;
		}
	}
	languageTransformPoints();
	languageDrawPolys();

	if (gameTextLang < LANG_NUMLANGS)
	{
		fontPrintScaled(font,-fontExtentWScaled(font,langName[gameTextLang],4096)/2,70,langName[gameTextLang],128,128,128,4096);

		fontDispSprite(FindTexture(buttonName[0]),-200, 75,128,128,128,font->alpha);
		fontDispSprite(FindTexture(buttonName[1]), 180, 75,128,128,128,font->alpha);
		fontDispSprite(FindTexture(buttonName[2]), -16, 90,128,128,128,font->alpha);

//		mapScreenDispSquare(gameInfo.buttons[4],-80,80,128,128,128,0,0,4096,-1,0);
//		mapScreenDispSquare(gameInfo.buttons[5],80,80,128,128,128,0,0,4096,-1,0);
//		if ((temp = ((gameInfo.frame<<3) & 255))>127)
//			temp = 127-(temp-127);
//		temp += 64;
//		mapScreenDispSquare(gameInfo.buttons[0],0,100, temp,temp,temp,0,0,4096,-1,0);
	}

	languageData.frame++;

	if(finishing)
		finishing += 2;
	else if((fadingOut == 0) && (padData.debounce[0] & PAD_CROSS))
	{
		PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,SAMPLE_VOLUME,-1);
		finishing += 2;
		ScreenFade(255,0,30);
		keepFade = 1;
	}

	if((finishing > 0) && (fadingOut == 0))
	{
		if (gameTextLang > LANG_NUMLANGS)
		gameTextLang = LANG_UK;
		languageDestroy();
		DoneLangSel=1;
		textureDestroyBank(languageData.flagTextures);
	}
}

void languageDestroy()
{
	FREE(flagPoints);
	if(imagePtr)	FREE(imagePtr);
	// free memory and stuff
	textureUnloadBank(languageData.flagTextures);
	if(gameTextLang>=LANG_NUMLANGS) gameTextLang=LANG_UK;

	gameTextDestroy();
//	gameTextInit(gameTextLang);

}

