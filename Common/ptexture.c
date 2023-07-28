/*
	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: ptexture.c
	Purpose		: Procedurally generated textures
	Programmer	: Jim Hubbard
	Date		: 2/12/99 10:30

----------------------------------------------------------------------------------------------- */

#include <islmem.h>
#include "Main.h"
#include "maths.h"

#ifdef PC_VERSION
#include <windows.h>
#include <gelf.h>
#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>
#include <pcmisc.h>
#else
#include "map_draw.h"
#include "world_eff.h"
#endif

#include "layout.h"
#include "ptexture.h"

PROCTEXTURE *dissolveTex=NULL;

PROCTEXTURE *prcTexList = NULL;

#ifdef PC_VERSION

/*	--------------------------------------------------------------------------------
	Function		: ProcessPTFire
	Purpose			: Procedural fire
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTFire( PROCTEXTURE *pt )
{
	unsigned long i,j,t;
	unsigned char *tmp;
	short p;

	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	for( i=992; i<1024; i++ )
		pt->buf1[i] = 0;

	for( i=0; i<9; i++ )
	{
		p = Random(30)-15;
		pt->buf1[1008+p] = 0xff;
		pt->buf1[1008+p+1] = 0xff;
		pt->buf1[1008+p-1] = 0xff;
		pt->buf1[1008+p-32] = 0xff;
		pt->buf1[1008+p-31] = 0xff;
		pt->buf1[1008+p-33] = 0xff;
	}


//	pt->buf1[0] = 0xff;

//	pt->buf1[512] = 0xff;

//	pt->buf1[1] = 0xff;
//	pt->buf1[2] = 0xff;

	// Smooth, move up and fade
	// NOTE FROM JIM - DON'T PUT SPACES IN MY CODE YOU BASTARDS!!!!
	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5) + j;
			t = pt->buf1[p+31] + pt->buf1[p+33] + pt->buf1[p+64];
			pt->buf2[p] = ((t+pt->buf1[p-64]) + (t+pt->buf1[p-32]))>>3;
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTCandle
	Purpose			: Little candle flame
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTCandle( PROCTEXTURE *pt )
{
	unsigned long i,j,t;
	unsigned char *tmp;
	short p;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	pt->buf1[528] = 200+Random(56);
	pt->buf1[528+1] = 200+Random(56);
	pt->buf1[528-1] = 200+Random(56);
	pt->buf1[528+32] = 200+Random(56);
	pt->buf1[528-32] = 200+Random(56);
	pt->buf1[528+31] = 200+Random(56);
	pt->buf1[528+33] = 200+Random(56);
	pt->buf1[528-31] = 200+Random(56);
	pt->buf1[528-33] = 200+Random(56);

	// Smooth, move up and fade
	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			t = ( pt->buf1[p+31]+ pt->buf1[p+33] + pt->buf1[p+64] );
			pt->buf2[p]=((t+pt->buf1[p-64])+(t+pt->buf1[p-32]))>>3;
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTForcefield
	Purpose			: Procedural forcefield effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTForcefield( PROCTEXTURE *pt )
{
	unsigned long i;//,j;
	unsigned char *tmp;
//	unsigned long t;
	unsigned short res;
	short p;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	p = ((Random(30)+1)*32) + Random(30)+1;
	pt->buf1[p]-=30;
	pt->buf1[p+1]-=15;
	pt->buf1[p-1]-=15;
	pt->buf1[p-32]-=15;
	pt->buf1[p+32]-=15;

	for (i=0; i<1024; i++)
	{
		res = (((unsigned long)((pt->buf1[(i)&1023] + pt->buf1[(i-1)&1023] + pt->buf1[(i-31)&1023] + pt->buf1[(i+32)&1023]-10)))) >>2;
		pt->buf2[i] = (unsigned char)res;
	}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTWater
	Purpose			: Procedural water drops effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTWaterDrops( PROCTEXTURE *pt )
{
	unsigned long i,j;
	unsigned char *tmp;
	short p, res;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	if( Random(2) )
		pt->buf1[((Random(30)+1)*32) + Random(30)+1] += 200;

	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			res = (pt->buf1[p+32] + pt->buf1[p-32] + pt->buf1[p+1] + pt->buf1[p-1] + pt->buf1[p+33] + pt->buf1[p+31] + pt->buf1[p-33] + pt->buf1[p-31] )>>2;
			res -= pt->buf2[p] + (pt->buf2[p]>>2);
			pt->buf2[p] = max(res,0);
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTWater
	Purpose			: Procedural water random rings effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTWaterBubbler( PROCTEXTURE *pt )
{
	unsigned long i,j;
	unsigned char *tmp;
	short p, res;
	static unsigned long x=16, y=16;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	x += Random(3)-1;
	y += Random(3)-1;

	if( x < 10 ) x = 10;
	else if( x > 20 ) x = 20;
	if( y < 10 ) y = 10;
	else if( y > 20 ) y = 20;

	pt->buf1[(y*32) + x] += 200;

	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			res = (pt->buf1[p+32] + pt->buf1[p-32] + pt->buf1[p+1] + pt->buf1[p-1] + pt->buf1[p+33] + pt->buf1[p+31] + pt->buf1[p-33] + pt->buf1[p-31] )>>2;
			res -= pt->buf2[p] + (pt->buf2[p]>>2);
			pt->buf2[p] = max(res,0);
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTWater
	Purpose			: Procedural water random rings effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTWaterTrail( PROCTEXTURE *pt )
{
	unsigned long i,j;
	unsigned char *tmp;
	short p, res;
	static short x=16, y=16, xv=2, yv=2;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	if( Random(10) > 8 )
	{
		xv += Random(3)-1;
		yv += Random(3)-1;

		if( xv > 2 ) xv = 2;
		else if( xv < -2 ) xv = -2;
		if( yv > 2 ) yv = 2;
		else if( yv < -2 ) yv = -2;
	}

	x += xv;
	y += yv;

	if( x < 2 || x > 30 )
	{
		xv *= -1;
		x += xv;
	}
	if( y < 2 || y > 30 )
	{
		yv *= -1;
		y += yv;
	}

	pt->buf1[(y*32) + x] += 200;

	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			res = (pt->buf1[p+32] + pt->buf1[p-32] + pt->buf1[p+1] + pt->buf1[p-1] + pt->buf1[p+33] + pt->buf1[p+31] + pt->buf1[p-33] + pt->buf1[p-31] )>>2;
			res -= pt->buf2[p] + (pt->buf2[p]>>2);
			pt->buf2[p] = max(res,0);
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTWaterRipples
	Purpose			: Procedural water ripple effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTWaterRipples( PROCTEXTURE *pt )
{
	unsigned long i,j;
	unsigned char *tmp;
	short p, res;

	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	pt->buf1[(Random(30)+1)+960] = 255;

	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			res = (pt->buf1[p+32] + pt->buf1[p-32] + pt->buf1[p+1] + pt->buf1[p-1] + pt->buf1[p+33] + pt->buf1[p+31] + pt->buf1[p-33] + pt->buf1[p-31] )>>2;
			res -= pt->buf2[p] + (pt->buf2[p]>>3);
			pt->buf2[p] = max(res,0);
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTSteam
	Purpose			: Procedural steam effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTWaterWaves( PROCTEXTURE *pt )
{
	unsigned long i,j;
	unsigned char *tmp;
	short p, res;

	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	if( Random(3)>1 )
		for( i=928; i<992; i++ )
			if( Random(3)>1 )
				pt->buf1[i] = 200 + Random(56);

	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			res = (pt->buf1[p+32] + pt->buf1[p-32] + pt->buf1[p+1] + pt->buf1[p-1] + pt->buf1[p+33] + pt->buf1[p+31] + pt->buf1[p-33] + pt->buf1[p-31] )>>2;
			res -= pt->buf2[p] + (pt->buf2[p]>>3);
			pt->buf2[p] = max(res,0);
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTSteam
	Purpose			: Procedural steam effect
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTSteam( PROCTEXTURE *pt )
{
	unsigned long i,j,t;
	unsigned char *tmp;
	short p;//, res;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	for( i=935; i<953; i++ )
		if( Random(3)>1 )
			pt->buf1[i] = 200 + Random(56);

	for( i=968; i<984; i++ )
		if( Random(3)>1 )
			pt->buf1[i] = 200 + Random(56);

	// Smooth, move up and fade
	for( i=30; i; i-- )
		for( j=30; j; j-- )
		{
			p = (i<<5)+j;
			t = ( pt->buf1[p+31]+ pt->buf1[p+33] + pt->buf1[p+64]);
			pt->buf2[p]=((t+pt->buf1[p-64])+(t+pt->buf1[p-32]))>>3;
		}

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessPTDissolve
	Purpose			: Dissolve texture
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessPTDissolve( PROCTEXTURE *pt )
{
	unsigned long i=1024;
	unsigned char *tmp;

	// Copy resultant buffer into texture
	PTSurfaceBlit( pt->tex, pt->buf1, pt->palette );

	while( i-- )
		pt->buf2[i] = (Random((10*gameSpeed)>>12)) ? pt->buf1[i] : (pt->active==1);

	// Swap buffers
	tmp = pt->buf1;
	pt->buf1 = pt->buf2;
	pt->buf2 = tmp;
}


/*	--------------------------------------------------------------------------------
	Function		: ProcessProcTextures
	Purpose			: Call update function pointers for all procedural textures
	Parameters		: 
	Returns			: 
	Info			: 
*/
void ProcessProcTextures( )
{
	PROCTEXTURE *pt;

	for( pt=prcTexList; pt; pt=pt->next )
		if( pt->Update && (actFrameCount > pt->timer) && pt->active )
		{
			pt->timer = actFrameCount+1; // Max 60fps - slow machines will just have to slow down
			pt->Update( pt );
		}
}


/*	--------------------------------------------------------------------------------
	Function		: FreeProcTextures
	Purpose			: Free procedural texture memory
	Parameters		: 
	Returns			: 
	Info			: 
*/
void FreeProcTextures( )
{
	PROCTEXTURE *pt;

	if( !prcTexList ) return;

	dissolveTex = NULL;

	for( pt=prcTexList; pt; pt=prcTexList )
	{
		// Remove from list
		prcTexList = pt->next;

		// Free buffers
		if( pt->palette ) FREE( pt->palette );
		if( pt->buf1 ) FREE( pt->buf1 );
		if( pt->buf2 ) FREE( pt->buf2 );

		// And free the object
		FREE( pt );
	}

#ifdef PC_VERSION
	if( pSurface ) pSurface->lpVtbl->Release(pSurface);
	pSurface = NULL;
#endif
}

#endif