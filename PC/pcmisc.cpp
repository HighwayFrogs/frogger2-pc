/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: pcmisc.cpp
	Programmer	: Andy Eder
	Date		: 05/05/99 17:42:44

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <gelf.h>
#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>
#include <islmem.h>
#include <isltex.h>
#include <islutil.h>

#include "Main.h"
#include "pcmisc.h"
#include "ptexture.h"
#include "islPad.h"
#include "controll.h"
#include "general.h"

#include <stdio.h>

int drawOverlays = 1;

float imtx[4][4]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};


LPDIRECTDRAWSURFACE7 pSurface = NULL;

LPDIRECTDRAWSURFACE7 LoadEditorTexture(const char* filename)
{
	int xDim, yDim;
	short *textureData;
	LPDIRECTDRAWSURFACE7 temp;

	textureData = (short *)gelfLoad_BMP((char*)filename, NULL, (void**)-1, 
		&xDim,&yDim,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);

	if (!textureData)
		return NULL;

	// Create surface and blit
	temp = D3DCreateTexSurface(xDim, yDim, 0xf81f, 0, 1);
	DDrawCopyToSurface(temp,(unsigned short *)textureData, 0, xDim, yDim,0);

	//res = D3DCreateTexSurface(xDim, yDim, 0xf81f, 0, 1);
	//res->BltFast(0,0,temp,NULL,DDBLTFAST_WAIT);

	return temp;
}


/*	--------------------------------------------------------------------------------
	Function 	: PTSurfaceBlit
	Purpose 	: Copy data into texture surface from procedural texture
	Parameters 	: Target texture, source data
	Returns 	: 
	Info 		:
*/
void PTSurfaceBlit( TextureType *tex, unsigned char *buf, unsigned short *pal )
{
	long i;

	if( !tex || !buf || !pal )
		return;

	if( rHardware )
	{
		DDSURFACEDESC2 ddsd;
		HRESULT res;
		LPDIRECTDRAWSURFACE7 to = ((MDX_TEXENTRY *)tex)->surf;
		DDINIT(ddsd);

		// Create static _AI_ surface you dolt
		if( !pSurface ) pSurface = D3DCreateTexSurface( 32,32,0xf81f, 1,1);
		
		if( (res = pSurface->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY,0)) != DD_OK )
			ddShowError(res);

		i=1024;

		while( i-- ) ((unsigned short *)ddsd.lpSurface)[i] = (unsigned short)pal[(unsigned char)buf[i]];

		pSurface->Unlock(NULL);

		if ((res = to->BltFast(0,0,pSurface,NULL,DDBLTFAST_NOCOLORKEY))!=DD_OK)
			ddShowError(res);
	}
	else
	{
		MDX_TEXENTRY *mdxdst = (MDX_TEXENTRY *)tex;

		if( mdxdst->softData )
		{
			unsigned short c;
			unsigned char r, g, b, a;
			i=1024;

			while( i-- )
			{
				c = (unsigned short)pal[(unsigned char)buf[i]];

				// This converts from 16bit 4444 to 24 bit 888 - it would be 32 bit 8888 but the alpha channel does nowt but make it red.
				// Presumably software just uses the top 16 bits for red and doesn't bother masking it.
				mdxdst->softData[i] =	(((((((c & 0x0f00)>>8)+1)<<4)-1)<<16) |
										((((((c & 0x00f0)>>4)+1)<<4)-1)<<8) |
										(((((c & 0x000f)>>0)+1)<<4)-1)) & 0x00ffffff;
			}
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: PTTextureLoad
	Purpose 	: Scan the texture list and convert any textures named "prc*" into procedurals
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void PTTextureLoad( )
{
	MDX_TEXENTRY *t;

	// List search
	for( t = texList; t; t = t->next )
	{
		if( t->name[0]=='p' && t->name[1]=='r' && t->name[2]=='c' )
			CreateProceduralTexture( (TextureType *)t, t->name );		
	}
/*
	t = GetTexEntryFromCRC(UpdateCRC("00wate04.bmp"));

	if (t)
		testS = t->surf;
*/
	pSurface = D3DCreateTexSurface( 32,32,0xf81f, 1,1);
}


/*	--------------------------------------------------------------------------------
	Function		: CreateAndAddProceduralTexture
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void CreateProceduralTexture( TextureType *tex, char *name )
{
	unsigned long i;
	unsigned long rVand,gVand,bVand,rVshr,gVshr,bVshr;
	unsigned short newCol,nR,nG,nB,nA;

	MDX_TEXENTRY *tx = (MDX_TEXENTRY *)tex;
	PROCTEXTURE *pt = (PROCTEXTURE *)MALLOC0( sizeof(PROCTEXTURE) );

	// Add to procedural text list
	pt->next = prcTexList;
	prcTexList = pt;

	// Set proc texture members
	pt->tex = tex;
	pt->buf1 = (unsigned char *)MALLOC0( 1024 );
	pt->buf2 = (unsigned char *)MALLOC0( 1024 );
	pt->palette = (unsigned short *)MALLOC0( 512 );

	pt->active = 1;

	// Convert palette to 4444 format

	rVand = 0x1f;
	gVand = 0x1f;
	bVand = 0x1f;
	rVshr = 10;
	gVshr = 5;
	bVshr = 0;

	for( i=0; i<256; i++ )
	{
		nR = ((unsigned short *)tx->data)[i] >> rVshr;
		nG = ((unsigned short *)tx->data)[i] >> gVshr;
		nB = ((unsigned short *)tx->data)[i] >> bVshr;
		nA = ((unsigned short *)tx->data)[i+256] >> bVshr;
		
		nR &= rVand;
		nG &= gVand;
		nB &= bVand;
		nA &= bVand;
		
		nR = (nR * 0x0f )/rVand;
		nG = (nG * 0x0f )/gVand;
		nB = (nB * 0x0f )/bVand;
		nA = (nA * 0x0f )/bVand;

		newCol = ((nA << 12) | (nR<<8) | (nG<<4) | (nB));
		
		((unsigned short *)pt->palette)[i] = newCol;
	}

	// Set update function and type depending on filename
	if( name[4]=='f' && name[5]=='i' && name[6]=='r' && name[7]=='e' )
		pt->Update = ProcessPTFire;
	else if( name[4]=='f' && name[5]=='f' && name[6]=='l' && name[7]=='d' )
		pt->Update = ProcessPTForcefield;
	else if( name[4]=='w' && name[5]=='a' && name[6]=='t' && name[7]=='r' )
	{
		if( name[8]=='r' )
			pt->Update = ProcessPTWaterRipples;
		else if( name[8]=='d' )
			pt->Update = ProcessPTWaterDrops;
		else if( name[8]=='b' )
			pt->Update = ProcessPTWaterBubbler;
		else if( name[8]=='t' )
			pt->Update = ProcessPTWaterTrail;
		else if( name[8]=='w' )
			pt->Update = ProcessPTWaterWaves;
	}
	else if( name[4]=='s' && name[5]=='t' && name[6]=='e' && name[7]=='a' )
	{
		pt->Update = ProcessPTSteam;
		pt->active = 0;
	}
	else if( name[4]=='c' && name[5]=='n' && name[6]=='d' && name[7]=='l' )
		pt->Update = ProcessPTCandle;
	else if( name[4]=='d' && name[5]=='s' && name[6]=='l' && name[7]=='v' )
	{
		pt->Update = ProcessPTDissolve;
		pt->active = 0;
		dissolveTex = pt;
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: FindTexture
	Purpose 	: Add .bmp extension to filename and find in all banks
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
TextureType *FindTexture( char *name )
{
	char bmp[32];

	strcpy( bmp, name );
	strlwr( bmp );
	strcat( bmp, ".bmp" );

	return (TextureType *)textureFindCRCInAllBanks(utilStr2CRC(bmp));
}

/*	--------------------------------------------------------------------------------
	Function 	: FindTexture
	Purpose 	: Add .bmp extension to filename and find in all banks
	Parameters 	: 
	Returns 	: 
	Info 		:
*/

// It's a very own graphics/code loop all on it's own!

short *screen = NULL;
void LoadScreenFromNumber(unsigned long num)
{
	int pptr = -1;
	char file[MAX_PATH];

	sprintf(file,"%s%s\\des%03lu.bmp",baseDirectory,"Design\\",num);
	screen = (short *)gelfLoad_BMP(file,NULL,(void**)&pptr,NULL,NULL,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);	
}

void FreeScreen(void)
{
	screen = 0;
}

void ShowScreen(void)
{
	unsigned long mPitch;
	DDSURFACEDESC2		ddsd;

	DDINIT(ddsd);
	
	while (surface[RENDER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR,0)!=DD_OK);
	
	mPitch = ddsd.lPitch/2;

	for (int y=0; y<480; y++)
		memcpy(&((short *)ddsd.lpSurface)[y*mPitch],&screen[y*640],640*sizeof(short));
	
	surface[RENDER_SRF]->Unlock(NULL);	
}

unsigned long quitDesignLoop = 0;
unsigned long designNum = 0;
#define MAX_DESIGNS 43

void DesignProcessController(long pl)
{
	unsigned long changedLevel = 0;
	u16 button[4];
	static u16 lastbutton[4];

	button[pl] = padData.digital[pl];
	
	if((button[pl] & PAD_UP) && !(lastbutton[pl] & PAD_UP))
	{
	}	    
	else if((button[pl] & PAD_RIGHT) && !(lastbutton[pl] & PAD_RIGHT))
	{
		if (designNum<MAX_DESIGNS)
		{
			designNum++;
			FreeScreen();
			LoadScreenFromNumber(designNum);
		}
	}
    else if((button[pl] & PAD_DOWN) && !(lastbutton[pl] & PAD_DOWN))
	{
	}
    else if((button[pl] & PAD_LEFT) && !(lastbutton[pl] & PAD_LEFT))
	{
		if (designNum>0)
		{
			designNum--;
			FreeScreen();
			LoadScreenFromNumber(designNum);
		}
	}


	if((button[pl] & PAD_CROSS) && !(lastbutton[pl] & PAD_CROSS))
    {
		FreeScreen();
		quitDesignLoop=1;
	}
	
	if((button[pl] & PAD_START) && !(lastbutton[pl] & PAD_START))
	{
	}

	lastbutton[pl] = button[pl];
}



extern "C"
{
void RunDesignWorkViewer(void)
{
	quitDesignLoop = 0;
	designNum = 0;
	while(!quitDesignLoop)
	{
		ProcessUserInput();
		DesignProcessController(0);

		if (screen)
			ShowScreen();
		else
			LoadScreenFromNumber(designNum);

		BeginDraw();
		EndDraw();
		DDrawFlip();
		D3DClearView();
	}
}
}

void PcNameEntryInit(void)
{
	int j;

	for(j = 0;j < NAME_LENGTH;j++)
		if(textString[j] == 0)
			textString[j] = '-';

	textString[NAME_LENGTH] = 0;
}

void mtxSetIdent( float *m )
{
	memcpy (m,imtx,sizeof(float)*16);
}


// *ASL* 26/06/2000 
/* -----------------------------------------------------------------------
   Function : CopyTexture
   Purpose : copt texture from source to destination
   Parameters : destination texture pointer, source texture pointer, copy palette flag
   Returns : 
   Info : only copies software textures if dimensions og texture buffers are equal
*/

void CopyTexture(TextureType *dest, TextureType *src, int copyPalette)
{
	if (!dest || !src) 
		return;

	if (rHardware)
	{
		HRESULT					res;
		LPDIRECTDRAWSURFACE7	to, from;

		to = ((MDX_TEXENTRY *)dest)->surf;
		from = ((MDX_TEXENTRY *)src)->surf;

		if ((res = to->BltFast(0, 0, from, NULL, DDBLTFAST_NOCOLORKEY)) != DD_OK)
			ddShowError(res);
	}
	else
	{
		MDX_TEXENTRY			*mdxsrc, *mdxdst;

		// re-cast to mdx textures
		mdxsrc = (MDX_TEXENTRY *)src;
		mdxdst = (MDX_TEXENTRY *)dest;
		// check if source and destinations are equal..
		if (mdxsrc->softData != NULL && mdxdst->softData != NULL && (mdxsrc->xSize * mdxsrc->ySize == mdxdst->xSize * mdxdst->ySize))
		{
			// blit texture from source to destination
			memcpy(mdxdst->softData, mdxsrc->softData, mdxsrc->xSize*mdxdst->ySize*sizeof(unsigned long));
		}
	}
}



enum { TOP=0x1, BOTTOM=0x2, LEFT=0x4, RIGHT=0x8, INWARD=0x10, OUTWARD=0x20 };

int OutcodeCheck( SVECTOR *p1, SVECTOR *p2 )
{
	MDX_VECTOR t1, t2;
	unsigned long oc1=0, oc2=0;
	float oozd;

	SetVectorRS( &t2, p1 );
	ScaleVector( &t2, 0.1 );
	// do the work of XfmPoint, using the matrix concatenated above
	guMtxXFMF(vMatrix.matrix,
		t2.vx,t2.vy,t2.vz,
		&(t1.vx),&(t1.vy),&(t1.vz));

	// clippage

	if( t1.vz < nearClip )
		oc1 |= INWARD;
	else if( t1.vz > farClip )
		oc1 |= OUTWARD;
	else
	{
		// Perform XfmPoint's really fucked up perspective calculation
		// that's "so much more efficient".
		oozd = -FOV * *(oneOver+fftol((((long *)&t1)+2))+DIST);
		t1.vx = halfWidth+(t1.vx * oozd);
		t1.vy = halfHeight+(t1.vy * oozd);

		if( t1.vx < 0 )
			oc1 |= LEFT;
		else if( t1.vx > rXRes )
			oc1 |= RIGHT;

		if( t1.vy < 0 )
			oc1 |= TOP;
		else if( t1.vy > rYRes )
			oc1 |= BOTTOM;
	}

	// Point1 is onscreen
	if( !oc1 )
		return 0;

	SetVectorRS( &t1, p2 );
	ScaleVector( &t1, 0.1 );

	guMtxXFMF(vMatrix.matrix,
		t1.vx,t1.vy,t1.vz,
		&(t2.vx),&(t2.vy),&(t2.vz));

	if( t2.vz < nearClip )
		oc2 |= INWARD;
	else if( t2.vz > farClip )
		oc2 |= OUTWARD;
	else
	{
		// Perform XfmPoint's really fucked up perspective calculation
		// that's "so much more efficient".
		oozd = -FOV * *(oneOver+fftol((((long *)&t2)+2))+DIST);
		t2.vx = halfWidth+(t2.vx * oozd);
		t2.vy = halfHeight+(t2.vy * oozd);

		if( t2.vx < 0 )
			oc2 |= LEFT;
		else if( t2.vx > rXRes )
			oc2 |= RIGHT;

		if( t2.vy < 0 )
			oc2 |= TOP;
		else if( t2.vy > rYRes )
			oc2 |= BOTTOM;
	}

	// Point2 is onscreen
	if( !oc2 )
		return 0;

	// If both offscreen on the same side, clip.
	if( oc1 & oc2 )
		return 1;

	// Off different sides - crossing screen
	return 0;
}
