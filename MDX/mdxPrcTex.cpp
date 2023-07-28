/*





	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: mdxReport.cpp
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>

#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxTexture.h"
#include "mdxCRC.h"
#include "mdxTiming.h"
#include "mgeReport.h"
#include "gelf.h"

#ifdef __cplusplus

extern "C"
{
#endif

LPDIRECTDRAWSURFACE7 testS = NULL;

short *waterData1;
short *waterData2;
#define WATER_EFFECT_RATIO 64
#define WATER_TEXTURE_SIZE 128
#define WATER_TEXTURE_SIZE2 (128*128)
#define WATER_TEXTURE_SHIFT 7

void InitWater(char *file)
{
	int pptr = -1;
	int xDim,yDim;
	char file1[MAX_PATH];
	char file2[MAX_PATH];

	sprintf(file1,"%swater.bmp",file);
	sprintf(file2,"%sreflect.bmp",file);

	// Load the files,
	waterData1 = (short *)gelfLoad_BMP(file1,NULL,(void**)&pptr,&xDim,&yDim,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);
	waterData2 = (short *)gelfLoad_BMP(file2,NULL,(void**)&pptr,&xDim,&yDim,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);
	
	// Update bump to map to 64
	for (unsigned int y=0; y<WATER_TEXTURE_SIZE; y++)
		for (unsigned int x=0; x<WATER_TEXTURE_SIZE; x++)
		{
			// 128 64 32 16 8 4 2 1
			waterData2[x+y*WATER_TEXTURE_SIZE] = ((waterData2[x+y*WATER_TEXTURE_SIZE] & 0x1f) * WATER_EFFECT_RATIO) / 0x1f;
		}
}

void UpdateWater(void)
{
	long wIndex1,wIndex2;
	long wOffset;//,lOffset;
	long x,y,y1,p;
	short *tPtr;
	
	DDSURFACEDESC2		ddsd;
	DDINIT(ddsd);

	if (!testS)
		return;

	wOffset = timeInfo.frameCount;//2;
	
	if (testS->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,0)!=DD_OK) return;
	
	tPtr = (short *)ddsd.lpSurface;
	p = ddsd.lPitch>>1;
	
	for (y=0,y1=0; y<WATER_TEXTURE_SIZE2; y+= WATER_TEXTURE_SIZE,y1+=p)
		for (x=0; x<WATER_TEXTURE_SIZE; x++)
		{
			wIndex1 = ((x + wOffset) & (WATER_TEXTURE_SIZE-1)) + y;
			wIndex2 = ((x + *(waterData2+wIndex1)) & (WATER_TEXTURE_SIZE-1)) + y;

			*(tPtr+x+y1) = *(waterData1+wIndex2) | 0x8000;
		}

	testS->Unlock(NULL);
}

#ifdef __cplusplus
}
#endif
