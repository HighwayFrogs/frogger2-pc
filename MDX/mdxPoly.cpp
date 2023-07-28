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
#include "mdxSprite.h"
#include "gelf.h"
#include "softstation.h"

extern "C"
{

// Instance of FRAME_INFO, for storing... Surprisingly.. The frame information./
// Would be ideal to choose what to alloc when render hardware/software is chosen!

FRAME_INFO frameInfo[MA_MAX_FRAMES];
FRAME_INFO *cFInfo = &frameInfo[0], *oFInfo = &frameInfo[1];

unsigned long numSeperates;
unsigned long numSoftPolys,numSoftVertex;
SOFTPOLY softPolyBuffer[MA_MAX_FACES];
SOFTPOLY *softDepthBuffer[MA_SOFTWARE_DEPTH];
D3DTLVERTEX softV[MA_MAX_VERTICES];
short softScreen[640*480];
short Indices[6] = {0,1,2,0,2,3};

unsigned long numHaloPoints;
long limTex = 0;
unsigned long fogging = 0;

//long softFlags = POLY_TEXTURE | POLY_MAGENTAMASK;

unsigned short haloZVals[MA_MAX_HALOS];
MDX_VECTOR haloPoints[MA_MAX_HALOS];
float flareScales[MA_MAX_HALOS];
float flareScales2[MA_MAX_HALOS];
unsigned long haloColor[MA_MAX_HALOS];
unsigned long haloSize[MA_MAX_HALOS];

MDX_TEXENTRY *haloHandle = 0;
MDX_TEXENTRY *haloHandle2 = 0;
MDX_TEXENTRY *haloHandle3 = 0;

unsigned long totalFacesDrawn = 0;

MDX_TEXENTRY *cTexture = NULL;
unsigned long sortMode = 0;
short sortFaces[32000];
long numSortFaces;

long curMode = 0;

unsigned long lightingMapRS[] = 
{
	D3DRENDERSTATE_ZFUNC,D3DCMP_EQUAL,
	D3DRENDERSTATE_ZWRITEENABLE,FALSE,	
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long normalAlphaCmpRS[] = 
{
	D3DRENDERSTATE_ALPHAREF,			0,
	D3DRENDERSTATE_ALPHAFUNC,			D3DCMP_NOTEQUAL,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long tightAlphaCmpRS[] = 
{
	D3DRENDERSTATE_ALPHAREF,			0x64,
	D3DRENDERSTATE_ALPHAFUNC,			D3DCMP_GREATER,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long normalZRS[] = 
{
	D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL,
	D3DRENDERSTATE_ZWRITEENABLE,TRUE,	
	D3DRENDERSTATE_ZENABLE,TRUE,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluZRS[] = 
{
	D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL,
	D3DRENDERSTATE_ZWRITEENABLE,FALSE,	
	D3DRENDERSTATE_ZENABLE,TRUE,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long noZRS[] = 
{
	D3DRENDERSTATE_ZWRITEENABLE,FALSE,
	D3DRENDERSTATE_ZENABLE,FALSE,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluSubRS[] = 
{
	D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO,
	D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCCOLOR,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluInvisibleRS[] =
{
	D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO,
	D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE,
	//	
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluAddRS[] = 
{
	D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA,
	D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE,	
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluSemiRS[] = 
{
	D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA,
	D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluNoneRS[] = 
{
	D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE,
	D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO,
	D3DRENDERSTATE_ALPHABLENDENABLE,0,

	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long xluEnableRS[] = 
{
	D3DRENDERSTATE_ALPHABLENDENABLE,1,
	//D3DRENDERSTATE_STIPPLEDALPHA,1,

	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long cullNoneRS[] = 
{
	D3DRENDERSTATE_CULLMODE,D3DCULL_NONE,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long cullCWRS[] = 
{
	D3DRENDERSTATE_CULLMODE,D3DCULL_CW,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};

unsigned long cullCCWRS[] = 
{
	D3DRENDERSTATE_CULLMODE,D3DCULL_CCW,
	//
	D3DRENDERSTATE_FORCE_DWORD,			NULL
};


long dPoly = 1;
	

/*  --------------------------------------------------------------------------------
    Function      : CopySoftScreenToSurface
	Purpose       :	copies the 320x240 software render to the screen at 640x480
	Parameters    : -
	Returns       : -
	Info          : -
*/
void CopySoftScreenToSurface(LPDIRECTDRAWSURFACE7 srf)
{
	DDSURFACEDESC2		ddsd;
//	unsigned long r,g,b,d;
	static unsigned long mPitch,cPitch,srcAddr,dstAddr;

	if (rHardware || !dPoly)
		return;
		
	DDINIT(ddsd);
	
	if (srf->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,0)!=DD_OK)
		return;
	
	mPitch = ddsd.lPitch * 2;
	cPitch = ddsd.lPitch;

	srcAddr = (unsigned long)softScreen;
	dstAddr = (unsigned long)ddsd.lpSurface;
	__asm
	{
		
		push ebp

		mov esi,[srcAddr]
		mov edi,[dstAddr]
		
		mov ebp,edi
		add ebp,[cPitch]
		
		mov edx,0
		YLOOP:
		
		mov ecx,0
		XLOOP:

		mov ax,[esi+ecx*2]
		mov bx,ax
		shl eax,16
		mov ax,bx

		mov dword ptr[edi+ecx*4],eax
		mov dword ptr[ebp+ecx*4],eax
		
		inc ecx
		cmp ecx,320
		jne XLOOP

		add esi,640*2
		add edi,[mPitch]
		add ebp,[mPitch]

		inc edx
		cmp edx,239
		jne YLOOP
		
		pop ebp
	}

/*	for (int y=0; y<480; y++)
		for (int x=0; x<640; x++)
		{
			d = softScreen[x+y*640];
			r = (d>>16) & 0xff;
			g = (d>>8) & 0xff;
			b = (d) & 0xff;

			r >>= 3;
			g >>= 2;
			b >>= 3;


			((short *)ddsd.lpSurface)[x+y*mPitch] = (r<<11 | g<<5 | b);
			((short*)ddsd.lpSurface)[x+y*ddsd.lPitch/2] = d;
		}
*/
	//for (int y=0; y<240; y++)
	//	memcpy(&((short *)ddsd.lpSurface)[y*mPitch],&softScreen[y*640],640*sizeof(short));
	
	srf->Unlock(NULL);	
}

/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/

void ClearSoftwareSortBuffer(void)
{
	numSoftPolys = 0;
	numSoftVertex = 0;
	for (int i=0; i<MA_SOFTWARE_DEPTH; i++)
		softDepthBuffer[i] = 0;
}

/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/

void InitFrames(void)
{
	for (int i=0; i<MA_MAX_FRAMES; i++)
	{
		SwapFrame(i);
		BlankFrame;
	}
	SwapFrame(0);
	numHaloPoints = 0;
	ClearSoftwareSortBuffer();
}

/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/
long softDepthOff = 0;

void PushPolys_Software( D3DTLVERTEX *v, int vC, short *fce, long fC, MDX_TEXENTRY *tEntry)
{
	long i,zVal;
	SOFTPOLY *m;

	for (i=0; i<fC; i+=3)
	{
		m = &softPolyBuffer[numSoftPolys++];
		
		m->flags = 0;

		if (cFInfo == &frameInfo[MA_FRAME_ADDITIVE])
			m->flags |= MDXSP_ADDITIVE;

		if( (currentDrawActor->flags & ACTOR_SLIDY) || (currentDrawActor->flags & ACTOR_WRAPTC) )
			m->flags |= MDXSP_WRAPUVS;
		
		m->f[0] = (unsigned short)(fce[i]+numSoftVertex);
		m->f[1] = (unsigned short)(fce[i+1]+numSoftVertex);
		m->f[2] = (unsigned short)(fce[i+2]+numSoftVertex);
		m->t = tEntry->surf;
		m->tEntry = tEntry;

		zVal = (long)(v[fce[i]].sz * MA_SOFTWARE_DEPTH);
		zVal += (long)(v[fce[i+1]].sz * MA_SOFTWARE_DEPTH);
		zVal += (long)(v[fce[i+2]].sz * MA_SOFTWARE_DEPTH);
		// Shift depth a la psx version
		zVal += softDepthOff;
		zVal >>= 2;

		if( zVal >= MA_SOFTWARE_DEPTH )
			zVal = MA_SOFTWARE_DEPTH-1;

		if (tEntry->type == TEXTURE_AI)
		{
			if ((RGBA_GETRED(v[fce[i]].color) + RGBA_GETGREEN(v[fce[i]].color) + RGBA_GETBLUE(v[fce[i]].color))<0.1)
				m->flags |= MDXSP_SUBTRACTIVE;
			else
				m->flags |= MDXSP_ADDITIVE;
		}
		
		if (softDepthBuffer[zVal])
			m->next = softDepthBuffer[zVal];
		else
			m->next = NULL;
		
		softDepthBuffer[zVal] = m;
	}
}



/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/

/*
void PushPolys( D3DTLVERTEX *v, int vC, short *fce, long fC, MDX_TEXENTRY *tEntry )
{
	long cnt;
	short *mfce = fce;
	LPDIRECTDRAWSURFACE7 tH = (tEntry ? tEntry->surf : 0);

	// Push for software/sorting...
	
	if (!rHardware)
	{
		if (tEntry)
		{
			PushPolys_Software(v,vC,fce,fC,tEntry);
			memcpy(&softV[numSoftVertex],v,vC*sizeof(D3DTLVERTEX));
			numSoftVertex+=vC;
		}

		return;
	}

	if (sortMode)
	{
		if (tEntry)
		{
			if (tEntry->type == TEXTURE_NORMAL)
			{
				PushPolys_Software(v,vC,fce,fC,tEntry);
				memcpy(&softV[numSoftVertex],v,vC*sizeof(D3DTLVERTEX));
				numSoftVertex+=vC;
			}
		}
		return;
	}

	// discard excess polys
	if ((cFInfo->nV + vC) > MA_MAX_VERTICES || (cFInfo->nF + fC) > MA_MAX_FACES)
		return;

	for (cnt=fC;cnt; cnt--)
	{
		*cFInfo->cF = (unsigned short)((*mfce) + cFInfo->nV);
		*cFInfo->cT = tH;
		cFInfo->cF++;
		cFInfo->cT++;
		mfce++;
	}

	memcpy(cFInfo->cV,v,vC*sizeof(D3DTLVERTEX));
	
	cFInfo->cV+=vC;
	cFInfo->nV+=vC;
	cFInfo->nF+=fC;
}
*/
//#endif





// *ASL* 27/06/2000
/* -----------------------------------------------------------------------
   Function: DrawSoftwarePolys
   Purpose : draw all software polygons from the software depth buffer
   Parameters : 
   Returns : 
   Info :
*/

void DrawSoftwarePolys (void)
{
	SOFTPOLY		*polyPtr;
	int				l, alphaVal;
	TSSVertex		ssVerts[3];
	int				v0, v1, v2;


	// default to use gouraud shading
	ssSetRenderState(SSRENDERSTATE_SHADEMODE,SSSHADEMODE_GOURAUD);

	// draw all polygons from depth buffer
	for (l=MA_SOFTWARE_DEPTH-1; l>0; l--)
	{
		polyPtr = (SOFTPOLY *)softDepthBuffer[l];

		// ** According to this loop, only textured polygons are drawn from the software
		// ** buffer. Can this be right?

		while (polyPtr)
		{
			v0 = (int)polyPtr->f[0];
			v1 = (int)polyPtr->f[1];
			v2 = (int)polyPtr->f[2];

			// load all software vertices
			ssVerts[0].x = (int)softV[v0].sx;
			ssVerts[0].y = (int)softV[v0].sy;
			ssVerts[0].r = RGBA_GETRED(softV[v0].color);
			ssVerts[0].g = RGBA_GETGREEN(softV[v0].color);
			ssVerts[0].b = RGBA_GETBLUE(softV[v0].color);

			ssVerts[1].x = (int)softV[v1].sx;
			ssVerts[1].y = (int)softV[v1].sy;
			ssVerts[1].r = RGBA_GETRED(softV[v1].color);
			ssVerts[1].g = RGBA_GETGREEN(softV[v1].color);
			ssVerts[1].b = RGBA_GETBLUE(softV[v1].color);

			ssVerts[2].x = (int)softV[v2].sx;
			ssVerts[2].y = (int)softV[v2].sy;
			ssVerts[2].r = RGBA_GETRED(softV[v2].color);
			ssVerts[2].g = RGBA_GETGREEN(softV[v2].color);
			ssVerts[2].b = RGBA_GETBLUE(softV[v2].color);

			// alpha blend all vertex rgbs
			alphaVal = RGBA_GETALPHA(softV[v0].color);
			if( alphaVal != 0xff )
			{
				ssVerts[0].r = (ssVerts[0].r * alphaVal) >>8;
				ssVerts[0].g = (ssVerts[0].g * alphaVal) >>8;
				ssVerts[0].b = (ssVerts[0].b * alphaVal) >>8;

				alphaVal = RGBA_GETALPHA(softV[v1].color);
				ssVerts[1].r = (ssVerts[1].r * alphaVal) >>8;
				ssVerts[1].g = (ssVerts[1].g * alphaVal) >>8;
				ssVerts[1].b = (ssVerts[1].b * alphaVal) >>8;

				alphaVal = RGBA_GETALPHA(softV[v2].color);
				ssVerts[2].r = (ssVerts[2].r * alphaVal) >>8;
				ssVerts[2].g = (ssVerts[2].g * alphaVal) >>8;
				ssVerts[2].b = (ssVerts[2].b * alphaVal) >>8;
			}

			// is this polygon textured?
			if (polyPtr->t)
			{
				unsigned long uLimit, vLimit, flags=0;

				if( !polyPtr->tEntry->keyed )
					flags |= SSTEXHINT_NOTRANS;
				if( polyPtr->flags & MDXSP_WRAPUVS )
					flags |= SSTEXHINT_WRAPUVS;

				uLimit = polyPtr->tEntry->xSize;
				vLimit = polyPtr->tEntry->ySize;

				// set texture
				if (polyPtr->tEntry->softData)
					ssSetTexture(polyPtr->tEntry->softData, uLimit, vLimit, flags);
				else
					ssSetTexture(NULL, 0, 0);

				// range all vertex uvs into texture size range
				ssVerts[0].u = softV[v0].tu * uLimit;
				ssVerts[0].v = softV[v0].tv * vLimit;

				ssVerts[1].u = softV[v1].tu * uLimit;
				ssVerts[1].v = softV[v1].tv * vLimit;
	
				ssVerts[2].u = softV[v2].tu * uLimit;
				ssVerts[2].v = softV[v2].tv * vLimit;

				// Double limit for wrapping textures
				if( polyPtr->flags & MDXSP_WRAPUVS )
				{
					uLimit <<= 1;
					vLimit <<= 1;
				}

				uLimit -= 1;
				vLimit -= 1;

				// validate all vertex uvs
				ssVerts[0].u = min(ssVerts[0].u, uLimit);
				ssVerts[0].u = max(ssVerts[0].u, 0);
				ssVerts[0].v = min(ssVerts[0].v, vLimit);
				ssVerts[0].v = max(ssVerts[0].v, 0);

				ssVerts[1].u = min(ssVerts[1].u, uLimit);
				ssVerts[1].u = max(ssVerts[1].u, 0);
				ssVerts[1].v = min(ssVerts[1].v, vLimit);
				ssVerts[1].v = max(ssVerts[1].v, 0);

				ssVerts[2].u = min(ssVerts[2].u, uLimit);
				ssVerts[2].u = max(ssVerts[2].u, 0);
				ssVerts[2].v = min(ssVerts[2].v, vLimit);
				ssVerts[2].v = max(ssVerts[2].v, 0);

				// convert all vertex uvs into softstation format
				ssVerts[0].u = SSMAKEUV(ssVerts[0].u);
				ssVerts[0].v = SSMAKEUV(ssVerts[0].v);

				ssVerts[1].u = SSMAKEUV(ssVerts[1].u);
				ssVerts[1].v = SSMAKEUV(ssVerts[1].v);

				ssVerts[2].u = SSMAKEUV(ssVerts[2].u);
				ssVerts[2].v = SSMAKEUV(ssVerts[2].v);
			}
			else
				ssSetTexture(NULL, 0, 0);

			// ** Additive alpha? Subtactive alpha?, Semi alpha?
			// ** This cannot be correct?

			// use additive alpha?
			if (polyPtr->flags & MDXSP_ADDITIVE)
				ssSetRenderState(SSRENDERSTATE_ALPHAMODE, SSALPHAMODE_ADD);
			// use subtractive alpha?
			else if (polyPtr->flags & MDXSP_SUBTRACTIVE)
			{
				ssSetRenderState(SSRENDERSTATE_ALPHAMODE, SSALPHAMODE_SUB);
				// fix all rgbs to psx max
				ssVerts[0].r = 127;
				ssVerts[0].g = 127;
				ssVerts[0].b = 127;
				ssVerts[1].r = 127;
				ssVerts[1].g = 127;
				ssVerts[1].b = 127;
				ssVerts[2].r = 127;
				ssVerts[2].g = 127;
				ssVerts[2].b = 127;
			}

			// draw softstation primitive
			ssDrawPrimitive(ssVerts, 3);

			// what is this!!! reset alpha mode (!!)
			if (polyPtr->flags & (MDXSP_ADDITIVE | MDXSP_SUBTRACTIVE))
				ssSetRenderState(SSRENDERSTATE_ALPHAMODE, SSALPHAMODE_NONE);

			polyPtr = polyPtr->next;
		}
	}
}





/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/


LPDIRECTDRAWSURFACE7 last;

void DrawSortedPolys (void)
{
 	SOFTPOLY *cur,*next;

	last = NULL;
	numSortFaces=0;

	if (sortMode == MA_SORTBACKFRONT)
	{
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);

		for (int i=MA_SOFTWARE_DEPTH-1; i>0; i--)
		{
			cur = (SOFTPOLY *)softDepthBuffer[i];
			while (cur)
			{
				if (cur->t!=last)
				{
					pDirect3DDevice->SetTexture(0,0);				
					pDirect3DDevice->SetTexture(0,cur->t);
					last=cur->t;
				}

			
				pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,softV,numSoftVertex,cur->f,3,0);
				
				cur = cur->next;
			}
		}

		pDirect3DDevice->SetTexture(0,0);
				
	}
	else
	{
		numSeperates = 0;

		if (cTexture)
			pDirect3DDevice->SetTexture(0,cTexture->surf);
		
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);

		for (int i=0; i<MA_SOFTWARE_DEPTH;)
		{
			cur = (SOFTPOLY *)softDepthBuffer[i++];
			while (cur)
			{		
				next = cur->next;

				numSortFaces = 0;

				last=cur->t;
					
				while (cur && cur->t==last)
				{
					
					next = cur->next;						
					sortFaces[numSortFaces+0] = cur->f[0];
					sortFaces[numSortFaces+1] = cur->f[1];
					sortFaces[numSortFaces+2] = cur->f[2];
					numSortFaces += 3;
					cur = next;

					while (!cur && i<MA_SOFTWARE_DEPTH)
						cur = (SOFTPOLY *)softDepthBuffer[i++];
				}

				pDirect3DDevice->SetTexture(0,0);				
				pDirect3DDevice->SetTexture(0,last);
		
				numSeperates++;
				
				pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,softV,numSoftVertex,(unsigned short *)sortFaces,numSortFaces,0);
			}
		
		}				
		
		pDirect3DDevice->SetTexture(0,0);
	}
}

/*  --------------------------------------------------------------------------------
    Function      : DrawBatchedPolys
	Purpose       :	-
	Parameters    : -
	Returns       : -
	Info          : -
*/



void DrawBatchedPolys (void)
{
	unsigned long i;
	unsigned long nFace,done;
	LPDIRECTDRAWSURFACE7 lSurface,*cT;
	
	cFInfo->cF = cFInfo->f;
	cFInfo->cT = cFInfo->t;

	nFace = 3;
	i=0;

	numSeperates = 0;
	done=0;
	cT = cFInfo->cT;

	
	while ((i<cFInfo->nF))
	{				
		lSurface = *cT;
		nFace = 0;
		while ((((*(cT)) == lSurface) || (limTex)) && (i<cFInfo->nF)) 
		{			
			cT+=3;
			nFace+=3;			
			i+=3;
		}

		pDirect3DDevice->SetTexture(0,lSurface);
		
		numSeperates++;
				
		pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,cFInfo->v,cFInfo->nV,cFInfo->cF,nFace,0);

		pDirect3DDevice->SetTexture(0,0);
		
		cFInfo->cF+=nFace;
		totalFacesDrawn+=nFace/3;	// Idle Time
		
		
	}
}

void DrawBatchedOpaquePolys (void)
{
	unsigned long i,oi;
	unsigned long nFace,done;
	char key,*cK;

	LPDIRECTDRAWSURFACE7 lSurface,*cT;
	
	cFInfo->cF = cFInfo->f;
	cFInfo->cT = cFInfo->t;
	cFInfo->cK = cFInfo->key;

	nFace = 3;
	i=0;

	numSeperates = 0;
	done=0;
	cT = cFInfo->cT;
	cK= cFInfo->cK;
	
	while ((i<cFInfo->nF))
	{				
		lSurface = *cT;
		
		key = *cK;
		
		oi = i;
		
		while ((((*(cT)) == lSurface)) && (i<cFInfo->nF)) 
		{
			cT+=3;
			i+=3;
		}

		nFace=(i-oi);
		cK+=nFace;
		
		pDirect3DDevice->SetTexture(0,lSurface);
		
		if (!key)
		{
			HRESULT res;

			res = pDirect3DDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX,
				cFInfo->v, cFInfo->nV, cFInfo->cF, nFace, 0);

			// Fucking monkey
/*			while((!=D3D_OK)
			{
				StartTimer(4,"DIP");
				Sleep(10);
				EndTimer(4);
			}
*/
		}
		
		pDirect3DDevice->SetTexture(0,0);
		
		cFInfo->cF+=nFace;

		totalFacesDrawn+=nFace/3;	// Idle Time
		
		
	}
}

void DrawBatchedKeyedPolys (void)
{
	unsigned long i;
	unsigned long nFace,done;
	char key,*cK;

	LPDIRECTDRAWSURFACE7 lSurface,*cT;
	
	cFInfo->cF = cFInfo->f;
	cFInfo->cT = cFInfo->t;
	cFInfo->cK = cFInfo->key;

	nFace = 3;
	i=0;

	numSeperates = 0;
	done=0;
	cT = cFInfo->cT;
	cK= cFInfo->cK;
	
	while ((i<cFInfo->nF))
	{				
		lSurface = *cT;
		nFace = 0;
		key = *cK;
		while ((((*(cT)) == lSurface) || (limTex)) && (i<cFInfo->nF)) 
		{
			cT+=3;
			cK+=3;
			nFace+=3;			
			i+=3;
		}

		pDirect3DDevice->SetTexture(0,lSurface);
		
		numSeperates++;
				
		if (key)
		{
			HRESULT res;
			
			res = pDirect3DDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX,
				cFInfo->v, cFInfo->nV, cFInfo->cF, nFace, 0);

			
			if (res != D3D_OK)
			{
				ddShowError(res);
				break;
			}

/*			Beware the monkey

			while(!=D3D_OK)
			{
				StartTimer(4,"DIP");
				Sleep(10);
				EndTimer(4);
			}
*/
		}

		pDirect3DDevice->SetTexture(0,0);
		
		cFInfo->cF+=nFace;

	}
}

void SetSoftwareState(unsigned long *me)
{
	while (*me!=D3DRENDERSTATE_FORCE_DWORD)
	{
		switch(*me)
		{
			case D3DRENDERSTATE_DESTBLEND:
				switch (*(me+1))
				{
//					case D3DBLEND_ZERO:
//						ssSetRenderState(SSRENDERSTATE_ALPHAMODE,SSALPHAMODE_NONE);
//						break;
					case D3DBLEND_ONE:
						ssSetRenderState(SSRENDERSTATE_ALPHAMODE,SSALPHAMODE_ADD);
						break;
					case D3DBLEND_INVSRCALPHA:
						ssSetRenderState(SSRENDERSTATE_ALPHAMODE,SSALPHAMODE_NONE);
						break;
					case D3DBLEND_INVSRCCOLOR:
						ssSetRenderState(SSRENDERSTATE_ALPHAMODE,SSALPHAMODE_SUB);
						break;
				}
				break;
		}
		me+=2;
	}
}

void SetTexture(MDX_TEXENTRY *me)
{
	if (rHardware) 
	{
		if (me)	
			pDirect3DDevice->SetTexture(0,me->surf); 
		else 
			pDirect3DDevice->SetTexture(0,0);
	}
	cTexture = me;	
}

HRESULT DrawPoly(D3DPRIMITIVETYPE d3dptPrimitiveType,DWORD  dwVertexTypeDesc, LPVOID lpvVertices, DWORD  dwVertexCount, LPWORD lpwIndices, DWORD  dwIndexCount, DWORD  dwFlags)
{
	HRESULT res;
//	MPR_BITMAP16 thisTex;
//	MPR_VERT v[3];
	D3DTLVERTEX *verts;
	unsigned long f1,f2,f3;
	int			alphaVal;

	totalFacesDrawn+=dwIndexCount / 3;
	if (rHardware)
	{

//		pDirect3DDevice->SetTexture(0,tex);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,FALSE);

		res = pDirect3DDevice->DrawIndexedPrimitive(d3dptPrimitiveType,dwVertexTypeDesc,lpvVertices,dwVertexCount,lpwIndices,dwIndexCount,dwFlags);

		if (fogging)
			pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,TRUE);


		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
		pDirect3DDevice->SetTexture(0,NULL);



//		res = pDirect3DDevice->DrawIndexedPrimitive(d3dptPrimitiveType,dwVertexTypeDesc,lpvVertices,dwVertexCount,lpwIndices,dwIndexCount,dwFlags);
	}
	else
	{
		ssSetRenderState(SSRENDERSTATE_SHADEMODE,SSSHADEMODE_GOURAUD);
		verts = (D3DTLVERTEX *)lpvVertices;
		for (int i=0; i<dwIndexCount; i+=3)
		{
//			unsigned long f1,f2,f3;
			TSSVertex	v[3];		
		
			f1 = lpwIndices[i+0];
			f2 = lpwIndices[i+1];
			f3 = lpwIndices[i+2];

			//.*ASL* 14/06/2000
			// ** Changed softV refs to verts

			v[0].x = (long)verts[f1].sx;
			v[0].y = (long)verts[f1].sy;
			v[0].r = RGBA_GETRED(verts[f1].color);
			v[0].g = RGBA_GETGREEN(verts[f1].color);
			v[0].b = RGBA_GETBLUE(verts[f1].color);

			v[1].x = (long)verts[f2].sx;
			v[1].y = (long)verts[f2].sy;
			v[1].r = RGBA_GETRED(verts[f2].color);
			v[1].g = RGBA_GETGREEN(verts[f2].color);
			v[1].b = RGBA_GETBLUE(verts[f2].color);

			v[2].x = (long)verts[f3].sx;
			v[2].y = (long)verts[f3].sy;
			v[2].r = RGBA_GETRED(verts[f3].color);
			v[2].g = RGBA_GETGREEN(verts[f3].color);
			v[2].b = RGBA_GETBLUE(verts[f3].color);

			// *ASL* 27/06/2000
			// alpha blend all vertex rgbs
			alphaVal = RGBA_GETALPHA(softV[f1].color);
			if( alphaVal != 0xff )
			{
				v[0].r = (v[0].r * alphaVal) >>8;
				v[0].g = (v[0].g * alphaVal) >>8;
				v[0].b = (v[0].b * alphaVal) >>8;

				alphaVal = RGBA_GETALPHA(softV[f2].color);
				v[1].r = (v[1].r * alphaVal) >>8;
				v[1].g = (v[1].g * alphaVal) >>8;
				v[1].b = (v[1].b * alphaVal) >>8;

				alphaVal = RGBA_GETALPHA(softV[f3].color);
				v[2].r = (v[2].r * alphaVal) >>8;
				v[2].g = (v[2].g * alphaVal) >>8;
				v[2].b = (v[2].b * alphaVal) >>8;
			}

			if (cTexture)
			{

				if (cTexture->softData)
				{
					// *ASL* 12/06/2000 - added colour key hint
					ssSetTexture(cTexture->softData,cTexture->xSize,cTexture->ySize, cTexture->keyed ? 0 : SSTEXHINT_NOTRANS);
				}
				else
					ssSetTexture(NULL, 0,0);

				v[0].u = verts[f1].tu * cTexture->xSize;
				v[0].v = verts[f1].tv * cTexture->ySize;

				v[1].u = verts[f2].tu * cTexture->xSize;
				v[1].v = verts[f2].tv * cTexture->ySize;
	
				v[2].u = verts[f3].tu * cTexture->xSize;
				v[2].v = verts[f3].tv * cTexture->ySize;

				if (v[0].u>cTexture->xSize-1)
					 v[0].u = cTexture->xSize-1;

				if (v[0].v>cTexture->ySize-1)
					 v[0].v = cTexture->ySize-1;

				if (v[1].u>cTexture->xSize-1)
					 v[1].u = cTexture->xSize-1;

				if (v[1].v>cTexture->ySize-1)
					 v[1].v = cTexture->ySize-1;

				if (v[2].u>cTexture->xSize-1)
					 v[2].u = cTexture->xSize-1;

				if (v[2].v>cTexture->ySize-1)
					 v[2].v = cTexture->ySize-1;

				if (v[0].u<0)
					 v[0].u = 0;

				if (v[0].v<0)
					 v[0].v = 0;

				if (v[1].u<0)
					 v[1].u = 0;

				if (v[1].v<0)
					 v[1].v = 0;

				if (v[2].u<0)
					 v[2].u = 0;

				if (v[2].v<0)
					 v[2].v = 0;

				v[0].u = SSMAKEUV(((long)v[0].u));
				v[1].u = SSMAKEUV(((long)v[1].u));
				v[2].u = SSMAKEUV(((long)v[2].u));
		
				v[0].v = SSMAKEUV(((long)v[0].v));
				v[1].v = SSMAKEUV(((long)v[1].v));
				v[2].v = SSMAKEUV(((long)v[2].v));
			}
			else
				ssSetTexture(NULL, 0,0);

			ssDrawPrimitive(v, 3);
			// *ASL* 13/06/2000
			res = D3D_OK;
		}
	}
		/*
		verts = (D3DTLVERTEX *)lpvVertices;
		for (int i=0; i<dwIndexCount; i+=3)
		{
			f1 = lpwIndices[i+0];
			f2 = lpwIndices[i+1];
			f3 = lpwIndices[i+2];

			v[0].x = (long)verts[f1].sx >> 1;
			v[0].y = (long)verts[f1].sy >> 1;
			v[0].argb = verts[f1].color;
			
			v[1].x = (long)verts[f2].sx >> 1;
			v[1].y = (long)verts[f2].sy >> 1;
			v[1].argb = verts[f2].color;

			v[2].x = (long)verts[f3].sx >> 1;
			v[2].y = (long)verts[f3].sy >> 1;
			v[2].argb = verts[f3].color;
			
			if (cTexture)
			{
				thisTex.width = cTexture->xSize;
				thisTex.height = cTexture->ySize;
				thisTex.image = (unsigned short *)cTexture->data;
				
				v[0].u = verts[f1].tu * thisTex.width;
				v[0].v = verts[f1].tv * thisTex.height;

				v[1].u = verts[f2].tu * thisTex.width;
				v[1].v = verts[f2].tv * thisTex.height;
	
				v[2].u = verts[f3].tu * thisTex.width;
				v[2].v = verts[f3].tv * thisTex.height;

				if (v[0].u>thisTex.width-1)
					 v[0].u = thisTex.width-1;

				if (v[0].v>thisTex.height-1)
					 v[0].v = thisTex.height-1;

				if (v[1].u>thisTex.width-1)
					 v[1].u = thisTex.width-1;

				if (v[1].v>thisTex.height-1)
					 v[1].v = thisTex.height-1;

				if (v[2].u>thisTex.width-1)
					 v[2].u = thisTex.width-1;

				if (v[2].v>thisTex.height-1)
					 v[2].v = thisTex.height-1;

				if (v[0].u<0)
					 v[0].u = 0;

				if (v[0].v<0)
					 v[0].v = 0;

				if (v[1].u<0)
					 v[1].u = 0;

				if (v[1].v<0)
					 v[1].v = 0;

				if (v[2].u<0)
					 v[2].u = 0;

				if (v[2].v<0)
					 v[2].v = 0;

				if (dPoly)
					f1 = MPR_DrawPoly((unsigned short *)softScreen,v,3,softFlags, &thisTex);
				res = D3D_OK;
			}			
		}
	}
*/	
	return res;
}

/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

void DrawFlatRect(RECT r, D3DCOLOR colour)
{
	D3DTLVERTEX v[4] = {
		{
				r.left,r.top,0,0,
			colour,D3DRGBA(0,0,0,1),
			0,0
		},
		{
			r.left,r.bottom,0,0,
			colour,D3DRGBA(0,0,0,1),
			0,0
		},
		{
			r.right,r.bottom,0,0,
			colour,D3DRGBA(0,0,0,1),
			0,0
		},
		{
			r.right,r.top,0,0,
			colour,D3DRGBA(0,0,0,1),
			0,0
	}};

	if (rHardware)
	{
		pDirect3DDevice->SetTexture(0,NULL);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		pDirect3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2);

		if (DrawPoly(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,v,4,(unsigned short *)Indices, 6,D3DDP_WAIT)!=D3D_OK)
			dp("Could not draw flat rectangle\n");

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,1);
		pDirect3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,1);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);

		return;
	}

	int sr,sg,sb,sa, su0,sv0, su1,sv1;
	sr = RGBA_GETRED(colour);
	sg = RGBA_GETGREEN(colour);
	sb = RGBA_GETBLUE(colour);
	sa = RGBA_GETALPHA(colour);

	// alpha rgbs
	sr = (sr * sa) >>8;
	sg = (sg * sa) >>8;
	sb = (sb * sa) >>8;

	su0 = sv0 = su1 = sv1 = 0;

	TSSVertex verts[4] = 
	{
		{r.left,  r.top,    sr,sg,sb, su0,sv0},
		{r.left,  r.bottom, sr,sg,sb, su0,sv1},
		{r.right, r.bottom, sr,sg,sb, su1,sv1},
		{r.right, r.top,    sr,sg,sb, su1,sv0},
	};

	ssSetRenderState(SSRENDERSTATE_CULLING, SSCULLING_NONE);
	ssSetRenderState(SSRENDERSTATE_CLIPPING, SSCLIPPING_OFF);

	ssSetRenderState(SSRENDERSTATE_SHADEMODE, SSSHADEMODE_FLAT);
//	ssSetRenderState(SSRENDERSTATE_ALPHAMODE, SSALPHAMODE_SEMI);

	ssSetTexture(NULL, 0, 0, 0);

	ssDrawPrimitive(verts, 4);
}

/*
void softDrawTexturedRect(RECT r, D3DCOLOR colour, float u0, float v0, float u1, float v1)
{
	if ((r.left>clx1) || (r.top>cly1) || (r.right<clx0) || (r.bottom<cly0))
		return;
		
	if (r.left<clx0)
	{
		u0 += ((clx0 - r.left)/((float)r.right-r.left)) * (u1-u0);
		r.left = clx0;
		
	}

	if (r.top<cly0)
	{
		v0 += ((cly0 - r.top)/((float)r.bottom-r.top)) * (v1-v0);
		r.top = cly0;
		
	}

	if (r.right>clx1)
	{
		u1 += ((clx1 - r.right)/((float)r.right-r.left)) * (u1-u0);
		r.right = clx1;		
	}

	if (r.bottom>cly1)
	{
		v1 += ((cly1 - r.bottom)/((float)r.bottom-r.top))*(v1-v0);
		r.bottom = cly1;		
	}


	int rd = RGBA_GETRED(colour);
	int g = RGBA_GETGREEN(colour);
	int b = RGBA_GETBLUE(colour);

	TSSVertex v[4] = {
		{ r.left, r.top, rd, g, b, u0, v0 },
		{ r.left, r.bottom, rd, g, b, u0, v1 },
		{ r.right, r.bottom, rd, g, b, u1, v1 },
		{ r.right, r.top, rd, g, b, u1, v0 }
	};

	ssDrawPrimitive(v, 4);		
}
*/


/* -----------------------------------------------------------------------
   Function : DrawTexturedRect
   Purpose : draw a texture rectangle using hardware only
   Parameters : rectangle, colour, mdx texture pointer, u0,v0 pair, u1,v1 pair
   Returns : 
   Info : 
*/

void DrawTexturedRect(RECT r, D3DCOLOR colour, LPDIRECTDRAWSURFACE7 tex, float u0, float v0, float u1, float v1)
{
	if ((r.left>clx1) || (r.top>cly1) || (r.right<clx0) || (r.bottom<cly0))
		return;
		
	if (r.left<clx0)
	{
		u0 += ((clx0 - r.left)/((float)r.right-r.left)) * (u1-u0);
		r.left = clx0;
		
	}

	if (r.top<cly0)
	{
		v0 += ((cly0 - r.top)/((float)r.bottom-r.top)) * (v1-v0);
		r.top = cly0;
		
	}

	if (r.right>clx1)
	{
		u1 += ((clx1 - r.right)/((float)r.right-r.left)) * (u1-u0);
		r.right = clx1;		
	}

	if (r.bottom>cly1)
	{
		v1 += ((cly1 - r.bottom)/((float)r.bottom-r.top))*(v1-v0);
		r.bottom = cly1;		
	}

	if (rHardware)
	{
		D3DTLVERTEX v[4] = {
			{
				r.left,r.top,0,1,
				colour,D3DRGBA(0,0,0,1),
				u0,v0
			},
			{
				r.left,r.bottom,0,1,
				colour,D3DRGBA(0,0,0,1),
				u0,v1
				},
			{
				r.right,r.bottom,0,1,
				colour,D3DRGBA(0,0,0,1),
				u1,v1
			},
			{
				r.right,r.top,0,1,
				colour,D3DRGBA(0,0,0,1),
				u1,v0
			}
		};

		pDirect3DDevice->SetTexture(0,tex);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,FALSE);

//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_POINT);  
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_POINT);

		pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,v,4,0);

		if (fogging)
			pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,TRUE);


//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_LINEAR);  
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_LINEAR);
	
	//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
		pDirect3DDevice->SetTexture(0,NULL);
	}
}


// *ASL* 13/06/2000
/* -----------------------------------------------------------------------
   Function : mdxPolyDrawTextureRect
   Purpose : draw a MDX texture rectangle
   Parameters : rectangle, colour, mdx texture pointer, u0,v0 pair, u1,v1 pair
   Returns : 
   Info : this routine will eventually replace the DrawTexturedRect() call above
*/

void mdxPolyDrawTextureRect(RECT rc, D3DCOLOR colour, MDX_TEXENTRY *mdxTexture, float u0, float v0, float u1, float v1)
{

//	colour = D3DRGB(1, 1, 1);

	// check if rectangle out of clip window
	if ((rc.left>clx1) || (rc.top>cly1) || (rc.right<clx0) || (rc.bottom<cly0))
		return;

	// clip left
	if (rc.left<clx0)
	{
		u0 += ((clx0 - rc.left)/((float)rc.right-rc.left)) * (u1-u0);
		rc.left = clx0;
		
	}

	// clip top
	if (rc.top<cly0)
	{
		v0 += ((cly0 - rc.top)/((float)rc.bottom-rc.top)) * (v1-v0);
		rc.top = cly0;
	}

	// clip right
	if (rc.right>clx1)
	{
		u1 += ((clx1 - rc.right)/((float)rc.right-rc.left)) * (u1-u0);
		rc.right = clx1;		
	}

	// clip bottom
	if (rc.bottom>cly1)
	{
		v1 += ((cly1 - rc.bottom)/((float)rc.bottom-rc.top))*(v1-v0);
		rc.bottom = cly1;		
	}

	// hardware?
	if (rHardware)
	{
		D3DTLVERTEX v[4] = {
			{
				rc.left,rc.top,0,1,
				colour,D3DRGBA(0,0,0,1),
				u0,v0
			},
			{
				rc.left,rc.bottom,0,1,
				colour,D3DRGBA(0,0,0,1),
				u0,v1
				},
			{
				rc.right,rc.bottom,0,1,
				colour,D3DRGBA(0,0,0,1),
				u1,v1
			},
			{
				rc.right,rc.top,0,1,
				colour,D3DRGBA(0,0,0,1),
				u1,v0
			}
		};

		pDirect3DDevice->SetTexture(0, mdxTexture->surf);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 0);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);

		pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,v,4,D3DDP_WAIT);

		if (fogging)
			pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,TRUE);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
		pDirect3DDevice->SetTexture(0,NULL);
		return;
	}

	// software
	int sr,sg,sb,sa, su0,sv0, su1,sv1;
	sr = RGBA_GETRED(colour);
	sg = RGBA_GETGREEN(colour);
	sb = RGBA_GETBLUE(colour);
	sa = RGBA_GETALPHA(colour);

	// alpha rgbs
	sr = (sr * sa) >>8;
	sg = (sg * sa) >>8;
	sb = (sb * sa) >>8;

	if (mdxTexture != NULL)
	{
		su0 = (int)(u0*mdxTexture->xSize);
		sv0 = (int)(v0*mdxTexture->ySize);
		su1 = (int)(u1*mdxTexture->xSize);
		sv1 = (int)(v1*mdxTexture->ySize);

		su0 = min(su0, mdxTexture->xSize);
		su0 = max(su0, 0);
		sv0 = min(sv0, mdxTexture->ySize);
		sv0 = max(sv0, 0);
		su1 = min(su1, mdxTexture->xSize);
		su1 = max(su1, 0);
		sv1 = min(sv1, mdxTexture->ySize);
		sv1 = max(sv1, 0);

		su0 = SSMAKEUV(su0);
		sv0 = SSMAKEUV(sv0);
		su1 = SSMAKEUV(su1);
		sv1 = SSMAKEUV(sv1);
	}
	else
		su0 = sv0 = su1 = sv1 = 0;

	TSSVertex verts[4] = 
	{
		{rc.left,  rc.top,    sr,sg,sb, su0,sv0},
		{rc.left,  rc.bottom, sr,sg,sb, su0,sv1},
		{rc.right, rc.bottom, sr,sg,sb, su1,sv1},
		{rc.right, rc.top,    sr,sg,sb, su1,sv0},
	};

	ssSetRenderState(SSRENDERSTATE_CULLING, SSCULLING_NONE);
	ssSetRenderState(SSRENDERSTATE_CLIPPING, SSCLIPPING_OFF);

	ssSetRenderState(SSRENDERSTATE_SHADEMODE, SSSHADEMODE_FLAT);
	ssSetRenderState(SSRENDERSTATE_ALPHAMODE, SSALPHAMODE_NONE);

	if (mdxTexture != NULL)
		ssSetTexture(mdxTexture->softData, mdxTexture->xSize, mdxTexture->ySize, mdxTexture->keyed ? 0 : SSTEXHINT_NOTRANS);
	else
		ssSetTexture(NULL, 0, 0, 0);

	ssDrawPrimitive(verts, 4);
}


/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

	
void DrawTexturedRect2(RECT r, D3DCOLOR colour, float u0, float v0, float u1, float v1)
{
	
	if ((r.left>clx1) || (r.top>cly1) || (r.right<clx0) || (r.bottom<cly0))
		return;
		
	if (r.left<clx0)
	{
		u0 += ((clx0 - r.left)/((float)r.right-r.left)) * (u1-u0);
		r.left = clx0;
		
	}

	if (r.top<cly0)
	{
		v0 += ((cly0 - r.top)/((float)r.bottom-r.top)) * (v1-v0);
		r.top = cly0;
		
	}

	if (r.right>clx1)
	{
		u1 += ((clx1 - r.right)/((float)r.right-r.left)) * (u1-u0);
		r.right = clx1;		
	}

	if (r.bottom>cly1)
	{
		v1 += ((cly1 - r.bottom)/((float)r.bottom-r.top))*(v1-v0);
		r.bottom = cly1;		
	}

	D3DTLVERTEX v[4] = {
		{
			r.left,r.top,0,1,
			colour,D3DRGBA(0,0,0,0),
			u0,v0
		},
		{
			r.left,r.bottom,0,1,
			colour,D3DRGBA(0,0,0,0),
			u0,v1
			},
		{
			r.right,r.bottom,0,1,
			colour,D3DRGBA(0,0,0,0),
			u1,v1
		},
		{
			r.right,r.top,0,1,
			colour,D3DRGBA(0,0,0,0),
			u1,v0
	}};

	if (rHardware)
	{
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,FALSE);
	
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_POINT);  
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_POINT);

		pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,v,4,D3DDP_WAIT);

		if (fogging)
			pDirect3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,TRUE);
			
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_LINEAR);  
//		pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_LINEAR);
	
	//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);		
	}
	else
	{
		DrawPoly(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,v, 4,(unsigned short *)Indices, 6, D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS );
	}
}

/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

void DrawTexturedRectRotated(float x, float y, float width, float height, D3DCOLOR colour, LPDIRECTDRAWSURFACE7 tex, float u0, float v0, float u1, float v1, float ang)
{
	float widthover2 = width/2, heightover2 = height /2;
	float x1,x2,x3,x4,y1,y2,y3,y4;
	D3DTLVERTEX v[4] = {
		{
			0,0,0,0,
			colour,D3DRGBA(0,0,0,0),
			u0,v0
		},
		{
			0,0,0,0,
			colour,D3DRGBA(0,0,0,0),
			u0,v1
			},
		{
			0,0,0,0,
			colour,D3DRGBA(0,0,0,0),
			u1,v1
		},
		{
			0,0,0,0,
			colour,D3DRGBA(0,0,0,0),
			u1,v0
	}};

	// Can be optimised lot's... I know... I know...
    x1 = -widthover2;
	y1 = -heightover2;

	x2 = +widthover2;
	y2 = -heightover2;

	x3 = +widthover2;
	y3 = +heightover2;

	x4 = -widthover2;
	y4 = +heightover2;

	v[0].sx =x+ cosf(ang) * x1 - sinf(ang) * y1;
	v[0].sy =y+ sinf(ang) * x1 + cosf(ang) * y1;
	
	v[1].sx =x+ cosf(ang) * x2 - sinf(ang) * y2;
	v[1].sy =y+ sinf(ang) * x2 + cosf(ang) * y2;
	
	v[2].sx =x+ cosf(ang) * x3 - sinf(ang) * y3;
	v[2].sy =y+ sinf(ang) * x3 + cosf(ang) * y3;
	
	v[3].sx =x+ cosf(ang) * x4 - sinf(ang) * y4;
	v[3].sy =y+ sinf(ang) * x4 + cosf(ang) * y4;
	
	pDirect3DDevice->SetTexture(0,tex);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,0);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,0);

	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
	
//	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_POINT);  
//	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_POINT);
	pDirect3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,v,4,D3DDP_WAIT);
			
//	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_LINEAR);  
//	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_LINEAR);
	
		pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);
//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	pDirect3DDevice->SetTexture(0,NULL);
}


/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRec
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

void BlankAllFrames(void)
{	
	for (int i=MA_FRAME_NORMAL; i<MA_MAX_FRAMES; i++)
	{
		SwapFrame(i);
		BlankFrame;
	}	
	numHaloPoints = 0;
	
//	ClearSoftwareSortBuffer();
}

/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

void LockRender(void)
{
	DDSURFACEDESC2		ddsd;
	DDINIT(ddsd);
	if (surface[RENDER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,0)!=DD_OK) return;
	surface[RENDER_SRF]->Unlock(NULL);
}

void StoreHaloPoints(void)
{
	DDSURFACEDESC2		ddsd;
	MDX_VECTOR v,t;

	if (numHaloPoints)
	{
		DDINIT(ddsd);
		if (surface[ZBUFFER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT,0)!=DD_OK) return;
		
		ddsd.lPitch /= sizeof(short);

		for (int i=0; i<numHaloPoints; i++)
		{
			SetVector(&t,&haloPoints[i]);
			XfmPoint(&v,&haloPoints[i],NULL);

			if (((v.vx>0) && (v.vx<rXRes)) && ((v.vy>0) && (v.vy<rYRes)))
			{
				haloPoints[i].vx = (unsigned long)v.vx;
				haloPoints[i].vy = (unsigned long)v.vy;
				haloPoints[i].vz = v.vz;
				
				if (haloPoints[i].vz)
				{
					haloZVals[i] = ((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+(haloPoints[i].vy*ddsd.lPitch))];
					if (haloZVals[i]>0)
						haloZVals[i]--;
					else
						haloZVals[i] = 1;
				}
			}	
			else
				haloPoints[i].vz = 0;
		}

		surface[ZBUFFER_SRF]->Unlock(NULL);
	}
}

void GetHaloPoints(void)
{
	DDSURFACEDESC2		ddsd;
	MDX_VECTOR v,t;

	if (numHaloPoints)
	{
		DDINIT(ddsd);
		if (surface[RENDER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT,0)!=DD_OK) return;
		
		ddsd.lPitch /= sizeof(short);

		for (int i=0; i<numHaloPoints; i++)
		{
			SetVector(&t,&haloPoints[i]);
			XfmPoint(&v,&haloPoints[i],NULL);

			if (((v.vx>0) && (v.vx<640)) && ((v.vy>0) && (v.vy<480)))
			{
				haloPoints[i].vx = (unsigned long)v.vx;
				haloPoints[i].vy = (unsigned long)v.vy;
				haloPoints[i].vz = v.vz;
				
				if (haloPoints[i].vz)
					haloZVals[i] = ((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+(haloPoints[i].vy*ddsd.lPitch))];
			}	
			else
				haloPoints[i].vz = 0;
		}

		surface[RENDER_SRF]->Unlock(NULL);
	}
}


void WriteHaloPoints(void)
{
//	DDSURFACEDESC2		ddsd;
//	MDX_VECTOR v,t;
	D3DTLVERTEX verts[4] = 
	{
		{64,64,0,1,D3DRGB(0,0,1),0, 0,0},
		{64+320,64,0,1,D3DRGB(0,0,1),0, 0,0},
		{64+320,64+320,0,1,D3DRGB(0,0,1),0, 0,0},
	};
	
	short faces[6] = {0,1,2};

	if (numHaloPoints)
	{
		for (int i=0; i<numHaloPoints; i++)
		{
			if (haloPoints[i].vz)
			{					
				haloPoints[i].vz -= 10;
				
				verts[0].sx = haloPoints[i].vx;	
				verts[0].sy = haloPoints[i].vy-5;
				verts[0].sz = haloPoints[i].vz * 0.00025;
				verts[1].sx = haloPoints[i].vx-5;
				verts[1].sy = haloPoints[i].vy+5;
				verts[1].sz = haloPoints[i].vz * 0.00025;
				verts[2].sx = haloPoints[i].vx+5;
				verts[2].sy = haloPoints[i].vy+5;
				verts[2].sz = haloPoints[i].vz * 0.00025;
				pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,verts,3,(unsigned short *)faces,3,0);
			}			
		}
	}
}

void ReadHaloPoints(void)
{
	DDSURFACEDESC2		ddsd;
//	MDX_VECTOR v;
	unsigned long i;

	if (numHaloPoints)
	{
		DDINIT(ddsd);
		if (surface[RENDER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_READONLY,0)!=DD_OK) return;
		
		ddsd.lPitch /= sizeof(short);

		for (i=0; i<numHaloPoints; i++)
			if (haloPoints[i].vz)
			{
				short val;				
				//((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+((haloPoints[i].vy)*ddsd.lPitch))-1] = 0xffff;
				//((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+((haloPoints[i].vy)*ddsd.lPitch))+1] = 0xffff;
				val = ((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+((haloPoints[i].vy)*ddsd.lPitch))];
				if (val!=0x001f)
					haloPoints[i].vz = 0;
				dp("%lx\n",val);
			}
		surface[RENDER_SRF]->Unlock(NULL);
	}
	
}


void AddHalo(MDX_VECTOR *point, float flareScaleA,float flareScaleB, unsigned long color, unsigned long size)
{
	flareScales[numHaloPoints] = flareScaleA;
	flareScales2[numHaloPoints] = flareScaleB;
	haloColor[numHaloPoints] = color;
	haloSize[numHaloPoints] = size;
	SetVector(&haloPoints[numHaloPoints++], point);	
}

/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

void CheckHaloPoints(void)
{
	DDSURFACEDESC2		ddsd;
//	MDX_VECTOR v;
	unsigned long i;

	if (numHaloPoints)
	{
		DDINIT(ddsd);
		if (surface[ZBUFFER_SRF]->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_WAIT,0)!=DD_OK) return;
		
		ddsd.lPitch /= sizeof(short);

		for (i=0; i<numHaloPoints; i++)
			if (haloPoints[i].vz)
				if (haloZVals[i] == ((short *)ddsd.lpSurface)[(unsigned long)(haloPoints[i].vx+(haloPoints[i].vy*ddsd.lPitch))])
					haloPoints[i].vz = 0;
		
		surface[ZBUFFER_SRF]->Unlock(NULL);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

LPDIRECTDRAWSURFACE7 haloS;
LPDIRECTDRAWSURFACE7 flareS;
LPDIRECTDRAWSURFACE7 flareS2;

void DrawHalos(void)
{
//	RECT rec;
	D3DSetupRenderstates(xluAddRS);
	float r = 1, g = 80/256.0, b = 10/256.0;
	float r1 = 1, g1 = 130/256.0, b1 = 110/256.0;
	
	if (!haloHandle)
	{
		haloHandle = GetTexEntryFromCRC(UpdateCRC("glowtex.bmp"));
		haloHandle2 = GetTexEntryFromCRC(UpdateCRC("halotex2.bmp"));
		haloHandle3 = GetTexEntryFromCRC(UpdateCRC("halotex3.bmp"));
		//FindTexture((TEXTURE **)&haloHandle2,UpdateCRC("halotex2.bmp"),1);
		//FindTexture((TEXTURE **)&haloHandle3,UpdateCRC("halotex3.bmp"),1);
	}

	for (int i=0; i<numHaloPoints; i++)
	{
		if (haloPoints[i].vz)
		{
			unsigned long c,size,size2;
			size = haloSize[i];
			size2 = size/2;
			
			DrawAlphaSprite(haloPoints[i].vx - size2,haloPoints[i].vy - size2, 0,size,size, 0,0,1,1,haloHandle,haloColor[i]);
			
			c = haloColor[i];
			c &= 0x00ffffff;
			
			size = fabs(haloSize[i] * 2 * flareScales[i]);
			size2 = 40+fabs(20 * flareScales[i]);

			DrawAlphaSpriteRotating(&haloPoints[i],0,haloPoints[i].vx - size2/2,haloPoints[i].vy - size/2, 0,size2,size, 0,0,1,1,haloHandle2,c | D3DRGBA(0,0,0,1-fabs(flareScales[i])));
			size = fabs(haloSize[i] * 1.5 * flareScales2[i]);
			DrawAlphaSpriteRotating(&haloPoints[i],0.5,haloPoints[i].vx - 20,haloPoints[i].vy - size/2, 0,40,size, 0,0,1,1,haloHandle2,c | D3DRGBA(0,0,0,1-fabs(flareScales2[i])));
			size = fabs(haloSize[i] * flareScales[i] * flareScales2[i]);
			
	//		DrawAlphaSpriteRotating(&(haloPoints[i].vx),-0.7,haloPoints[i].vx - 20,haloPoints[i].vy - size/2, 0,40,size, 0,0,1,1,haloHandle2,c | D3DRGBA(0,0,0,1-fabs(flareScales2[i])));

			/*rec.left = haloPoints[i].vx - 60;
			rec.right = haloPoints[i].vx + 60;

			rec.top = haloPoints[i].vy - 60;
			rec.bottom = haloPoints[i].vy + 60;

			DrawTexturedRect(rec,D3DRGBA(r1,g1,b1,1),haloS,FULL_TEXTURE);

			DrawTexturedRectRotated(haloPoints[i].vx,haloPoints[i].vy,fabs(200 * flareScales[i]),60 * (fabs(flareScales[i])),D3DRGBA(r,g,b,0.8-fabs(flareScales[i])*0.5),flareS,FULL_TEXTURE,1.57);
			DrawTexturedRectRotated(haloPoints[i].vx,haloPoints[i].vy,fabs(150 * flareScales2[i]),20,D3DRGBA(r,g,b,1-fabs(flareScales2[i])*0.5),flareS,FULL_TEXTURE,1);
			DrawTexturedRectRotated(haloPoints[i].vx,haloPoints[i].vy,30+fabs(80 * flareScales2[i]),30+fabs(80 * flareScales2[i]),D3DRGBA(r,g,b,1-fabs(flareScales2[i])),flareS2,FULL_TEXTURE,3*flareScales2[i]);
			DrawTexturedRectRotated(haloPoints[i].vx,haloPoints[i].vy,30+fabs(80 * flareScales[i]),30+fabs(80 * flareScales[i]),D3DRGBA(r,g,b,1-fabs(flareScales[i])),flareS2,FULL_TEXTURE,0);			*/
		}
	}

}


/*	--------------------------------------------------------------------------------
	Function		: DrawFlatRect
	Purpose			: draw a flat rectangle
	Parameters		: 
	Returns			: 
	Info			: 
*/

unsigned long drawLighting = 1;
unsigned long drawPhong = 1;

void DrawAllFrames(void)
{
//	int i;
//	D3DTLVERTEX *cV;
//	unsigned long aValue,cValue;
/*	pDirect3DDevice->BeginScene();
	DrawSoftwarePolys();
	pDirect3DDevice->EndScene();
*/
//	GetHaloPoints();
	//BeginDraw();
	
//	if (numHaloPoints)
//		WriteHaloPoints();	

	// Draw Normal Polys
	D3DSetupRenderstates(xluSemiRS);
	D3DSetupRenderstates(normalAlphaCmpRS);
	SwapFrame(MA_FRAME_NORMAL);

	if (!rHardware)
	{
//		DrawSoftwarePolys();
		//EndDraw();
		return;
	}
	else
		if (!sortMode)
		{
			// ds- is there any benefit to doing this? 
			// I'll just set the tight alpha compare state and do DrawBatchedPolys() instead..
			/*
			DrawBatchedOpaquePolys();

			D3DSetupRenderstates(tightAlphaCmpRS);
			DrawBatchedKeyedPolys();
			
			D3DSetupRenderstates(normalAlphaCmpRS);
			*/

			D3DSetupRenderstates(tightAlphaCmpRS);
			DrawBatchedPolys();

			pDirect3DDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			//pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_POINT);  
			//pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_POINT);

			SwapFrame(MA_FRAME_WRAP);
			DrawBatchedPolys();	

			//pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_LINEAR);  
			//pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_LINEAR);
			pDirect3DDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
		}
		else
			DrawSortedPolys();
	
	D3DSetupRenderstates(normalAlphaCmpRS);
	D3DSetupRenderstates(xluEnableRS);
	if (numHaloPoints)
	{
//		EndDraw();
///		
//		GetHaloPoints();

//		BeginDraw();
//		WriteHaloPoints();
//		EndDraw();

//		LockRender();
		
//		BeginDraw();
//		WriteHaloPoints();
//		EndDraw();
		

		// Need to test the ZBuffer on these polys...

/*		BeginDraw();

		SwapFrame(MA_FRAME_GLOW);
		D3DSetupRenderstates(xluInvisibleRS);
		DrawBatchedPolys();

		EndDraw();*/
		
//		ReadHaloPoints();
	
//		BeginDraw();
	}


	D3DSetupRenderstates(tightAlphaCmpRS);

	if (drawLighting || drawPhong)
	{
		// Draw Lightingmap polys
		D3DSetupRenderstates(lightingMapRS);
	
		if (drawLighting)
		{
			// First normal lighting
			D3DSetupRenderstates(xluSubRS);
			SwapFrame(MA_FRAME_LIGHTMAP);
			DrawBatchedPolys();
		}

		if (drawPhong)
		{
			// Then Phong
			D3DSetupRenderstates(xluAddRS);
			SwapFrame(MA_FRAME_PHONG);
			DrawBatchedPolys();
		}

		// Switch Back to normal
		
	}
	D3DSetupRenderstates(normalAlphaCmpRS);	
	D3DSetupRenderstates(xluZRS);
	D3DSetupRenderstates(xluSemiRS);

	pDirect3DDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);

	SwapFrame(MA_FRAME_XLU);
	DrawBatchedPolys();

 	D3DSetupRenderstates(xluAddRS);	
	SwapFrame(MA_FRAME_ADDITIVE);

/*	// because this uses the specular component and apparently lots of other code doesn't,
	// this effectively buggers up lots of polygons seemingly at random. Whatever next, eh? - ds
	// Sort out additive for fog.
	i = 0;
	cV = cFInfo->v;
	while ((i<cFInfo->nV))
	{
		cValue = cV->color;
		aValue =  ((cValue>>24) * (cV->specular>>24))>>8;		
		cV->color = (cValue&0x00ffffff) | (aValue << 24);
		cV++;
		i++;
	}
*/
	DrawBatchedPolys();
	pDirect3DDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
	SwapFrame(MA_FRAME_OVERLAY);

	//DrawHalos();
	D3DSetupRenderstates(noZRS);
	D3DSetupRenderstates(cullNoneRS);
	DrawBatchedPolys();	
	D3DSetupRenderstates(cullCWRS);
	
	D3DSetupRenderstates(normalZRS);
	D3DSetupRenderstates(normalAlphaCmpRS);		
	
	D3DSetupRenderstates(xluSemiRS);

	//pDirect3DDevice->EndScene();
}




/* -----------------------------------------------------------------------
   Function : mdxDrawRectangle
   Purpose : draw a rectange into the surface
   Parameters : rectangle, r, g, b
   Returns : 1 blit error else 0 okay
   Info : 
*/

int mdxDrawRectangle(RECT rc, int r, int g, int b)
{
	DDBLTFX		dxBlit;
	HRESULT		dxError;

	DDINIT(dxBlit);
	dxBlit.dwFillColor = (r565) ? ((r>>3)<<11) | ((g>>2)<<5) | (b>>3) : ((r>>3)<<10) | ((g>>3)<<5) | (b>>3);
	dxError = surface[RENDER_SRF]->Blt(&rc, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &dxBlit);
	return FAILED(dxError) ? 1 : 0;
}



}