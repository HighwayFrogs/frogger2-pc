/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: mavis.cpp
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <math.h>
#include "mgeReport.h"
#include "mdxDDraw.h"
#include "mdxD3D.h"
#include "mdxInfo.h"
#include "mdxTiming.h"
#include "mdxCRC.h"
#include "mdxTexture.h"
#include "mdxMath.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxLandscape.h"
#include "mdxRender.h"
#include "mdxPoly.h"
#include "mdxDText.h"
#include "mdxProfile.h"
#include "mdxWindows.h"

extern "C"
{

unsigned short spIndices[] = {0,1,2,2,3,0};


/*	--------------------------------------------------------------------------------
    Function		: DrawAlphaSprite
	Parameters		: 
	Returns			: 
	Purpose			: Draw an alpha-ed, non-rotating sprite
*/
void DrawAlphaSprite (float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour )
{
	D3DTLVERTEX v[4];
	float x2 = (x+xs), y2 = (y+ys);
//	float fogAmt;
	totalFacesDrawn++;

	if (x < clx0)
	{
		if (x2 < clx0) return;
		u1 += (u2-u1) * (clx0-x)/xs;	// clip u
		xs += x-clx0; x = clx0;
	}
	if (x2 > clx1)
	{
		if (x > clx1) return;
		u2 += (u2-u1) * (clx1-x2)/xs;	// clip u
		xs -= (x-clx1);
		x2 = clx1;
	}

	if (y < cly0)
	{
		if (y2 < cly0) return;
		v1 += (v2-v1) * (cly0-y)/ys;	// clip v
		ys += y-cly0; y = cly0;
	}
	if (y2 > cly1)
	{
		if (y > cly1) return;
		v2 += (v2-v1) * (cly1-y2)/ys;	// clip v
		ys -= (y-cly1);
		y2 = cly1;
	}
	
/*	fogAmt = FOGADJ(z);
	if (fogAmt<0)
		fogAmt=0;
	if (fogAmt>1)
		fogAmt=1;
*/	

	v[0].sx = x; v[0].sy = y; v[0].sz = z; v[0].rhw = 1;
	v[0].color = colour; v[0].specular = D3DRGBA(0,0,0,1);
	v[0].tu = u1; v[0].tv = v1;

	v[1].sx = x2; v[1].sy = y; v[1].sz = z; v[1].rhw = 1;
	v[1].color = v[0].color; v[1].specular = v[0].specular;
	v[1].tu = u2; v[1].tv = v1;
	
	v[2].sx = x2; v[2].sy = y2; v[2].sz = z; v[2].rhw = 1;
	v[2].color = v[0].color; v[2].specular = v[0].specular;
	v[2].tu = u2; v[2].tv = v2;

	v[3].sx = x; v[3].sy = y2; v[3].sz = z; v[3].rhw = 1;
	v[3].color = v[0].color; v[3].specular = v[0].specular;
	v[3].tu = u1; v[3].tv = v2;

	PushPolys(v,4,(short *)spIndices,6,tex);
}

void DrawAlphaSpriteRotating(MDX_VECTOR *pos,float angle,float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour )
{
	D3DTLVERTEX v[5];
	float x2 = (x+xs), y2 = (y+ys), sine, cosine, newX, newY;
	int i;
	totalFacesDrawn++;

	if( !tex ) return;

	// populate our structure ready for transforming and clipping the sprite
	v[0].sx = x - pos->vx;
	v[1].sx = x2 - pos->vx;
	v[2].sx = x2 - pos->vx;
	v[3].sx = x - pos->vx;

	v[0].sy = y - pos->vy;
	v[1].sy = y - pos->vy;
	v[2].sy = y2 - pos->vy;
	v[3].sy = y2 - pos->vy;

	v[0].tu = u1;	v[0].tv = v1;
	v[1].tu = u2;	v[1].tv = v1;
	v[2].tu = u2;	v[2].tv = v2;
	v[3].tu = u1;	v[3].tv = v2;

	// get rotation angle
	cosine	= (float)cosf(angle);
	sine	= (float)sinf(angle);

	// populate remaining data members and rotate the vertices comprising the sprite
	i = 4;
	while(i--)
	{
		v[i].sz			= z;
		v[i].rhw		= 1;
		v[i].color		= colour;
		v[i].specular	= D3DRGBA(0,0,0,1);

		newX = (v[i].sx * cosine) + (v[i].sy * sine);
		newY = (v[i].sy * cosine) - (v[i].sx * sine);

		v[i].sx = newX + pos->vx;
		v[i].sy = newY + pos->vy;
	}

	memcpy( &v[4], &v[0], sizeof(D3DTLVERTEX) );

	Clip3DPolygon( v, tex );
	Clip3DPolygon( &v[2], tex );
}	


/*	--------------------------------------------------------------------------------
    Function		: DrawSpriteOverlayRotating
	Parameters		: 
	Returns			: 
	Purpose			: Draw an alpha-ed, rotating sprite overlay
*/
void DrawSpriteOverlayRotating( float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, float angle, MDX_TEXENTRY *tex, DWORD colour )
{
//	float ca = cosf(angle), sa = sinf(angle);

//	D3DTLVERTEX v[4] = {
//		{
//			0,0,0,0,
//			colour,D3DRGBA(0,0,0,0),
//			u1,v1
//		},
//		{
//			0,0,0,0,
//			colour,D3DRGBA(0,0,0,0),
//			u1,v2
//			},
//		{
//			0,0,0,0,
//			colour,D3DRGBA(0,0,0,0),
//			u2,v2
//		},
//		{
//			0,0,0,0,
//			colour,D3DRGBA(0,0,0,0),
//			u2,v1
//	}};

//	v[0].sx = x + (ca*-w2) - (sa*-h2);
//	v[0].sy = y + (ca*-w2) + (ca*-h2);
	
//	v[1].sx = x + (ca*w2) - (sa*-h2);
//	v[1].sy = y + (sa*w2) + (ca*-h2);
	
//	v[2].sx = x + (ca*w2) - (sa*h2);
//	v[2].sy = y + (sa*w2) + (ca*h2);
	
//	v[3].sx = x + (ca*-w2) - (sa*h2);
//	v[3].sy = y + (sa*-w2) + (ca*h2);

	D3DTLVERTEX v[4];
	float sine, cosine, w2, h2, newX, newY;
	int i;

	if( !tex ) return;

	w2 = xs*0.5;
	h2 = ys*0.5;
	// populate our structure ready for transforming and clipping the sprite
	v[0].sx = -w2;
	v[1].sx = w2;
	v[2].sx = w2;
	v[3].sx = -w2;

	v[0].sy = -h2;
	v[1].sy = -h2;
	v[2].sy = h2;
	v[3].sy = h2;

	v[0].tu = u1;	v[0].tv = v1;
	v[1].tu = u2;	v[1].tv = v1;
	v[2].tu = u2;	v[2].tv = v2;
	v[3].tu = u1;	v[3].tv = v2;

	// get rotation angle
	cosine = cosf(angle);
	sine = sinf(angle);

	// populate remaining data members and rotate the vertices comprising the sprite
	i = 4;
	while(i--)
	{
		v[i].sz			= z;
		v[i].rhw		= 1;
		v[i].color		= colour;
		v[i].specular	= D3DRGBA(0,0,0,1);

		newX = (v[i].sx * cosine) + (v[i].sy * sine);
		newY = (v[i].sy * cosine) - (v[i].sx * sine);

		v[i].sx = newX + x + w2;
		v[i].sy = newY + y + h2;
	}

	SetTexture(tex);

	DrawPoly( D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
		v, 4, (unsigned short *)spIndices, 6,
		D3DDP_WAIT);

	SetTexture(NULL);
}


}
