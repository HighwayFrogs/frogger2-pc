
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>

#include "fxBlur.h"
#include "mdx.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SCREENTEX_SIZE 64
#define BLUR_Y 384+64
#define BLUR_X 384

#define BLUR_XOFS (320-(384/2))

D3DTLVERTEX *vList;
LPDIRECTDRAWSURFACE7 *tList;

D3DTLVERTEX *InitScreenVertexList(void);
long numRequired;

void fxInitBlur(void)
{
	vList = InitScreenVertexList();
	tList = new LPDIRECTDRAWSURFACE7[numRequired];
	for (int i=0; i<numRequired; i++)
		tList[i] = 0;
}

void fxFreeBlur(void)
{
	delete [] vList;
	vList = NULL;

	for (int i=0; i<numRequired; i++)
		if( tList[i] )
			tList[i]->Release( );

	delete [] tList;
	tList = NULL;
}

void DrawScreenOverlays(void)
{
	unsigned short faces[] = {0,1,2,0,2,3};
	
	pDirect3DDevice->BeginScene();

	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);

	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_POINT);  
	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_POINT);

	for (int i=0; i<numRequired; i++)
	{
		pDirect3DDevice->SetTexture(0,tList[i]);
		pDirect3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,&vList[i*4],4,faces,6,D3DDP_WAIT);
		pDirect3DDevice->SetTexture(0,NULL);
	}

	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW);

	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFN_LINEAR);  
	pDirect3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFN_LINEAR);

	pDirect3DDevice->EndScene();

}

void GrabScreenTextures(LPDIRECTDRAWSURFACE7 from, LPDIRECTDRAWSURFACE7 *to)
{

	int i, j,y,sY,dY;
	LPDIRECTDRAWSURFACE7 surface, *dds, surface2, *dds2;
	DDSURFACEDESC ddsd,ddsd2; 
	POINT t = {0,0};
	
	unsigned long *hdl=0;
	HRESULT res;
	RECT rect,r2;

	dds = &to[0];	
	for (rect.top = 0, rect.bottom = SCREENTEX_SIZE; rect.top < BLUR_Y; rect.top += SCREENTEX_SIZE, rect.bottom += SCREENTEX_SIZE)
		for (rect.left = BLUR_XOFS, rect.right = SCREENTEX_SIZE+BLUR_XOFS; rect.left < BLUR_X+BLUR_XOFS; rect.left += SCREENTEX_SIZE, rect.right += SCREENTEX_SIZE)
		{
			
			if (!*dds)
				*dds = surface = D3DCreateTexSurface2(SCREENTEX_SIZE,SCREENTEX_SIZE,1,1);
			else
				surface = *dds;
	
			res = surface->BltFast(0,0,from,&rect,DDBLTFAST_WAIT);

			hdl++;
			dds++;			
		}	
}

	
D3DTLVERTEX *InitScreenVertexList(void)
{
	numRequired = (BLUR_Y/SCREENTEX_SIZE) * (BLUR_X/SCREENTEX_SIZE);

	RECT rect;
	D3DTLVERTEX *me = new D3DTLVERTEX[numRequired*4];
	int i = 0;

	for (i=0; i<numRequired * 4; i+=4)
	{
		me[i].tu = 0;
		me[i].tv = 0;
		
		me[i+1].tu = 1;
		me[i+1].tv = 0;

		me[i+2].tu = 1;
		me[i+2].tv = 1;

		me[i+3].tu = 0;
		me[i+3].tv = 1;
		
		me[i].color = D3DRGBA(1,1,1,0.7);
		me[i+1].color = D3DRGBA(1,1,1,0.7);
		me[i+2].color = D3DRGBA(1,1,1,0.7);
		me[i+3].color = D3DRGBA(1,1,1,0.7);

		me[i].specular = D3DRGBA(0,0,0,1);
		me[i+1].specular = D3DRGBA(0,0,0,1);
		me[i+2].specular = D3DRGBA(0,0,0,1);
		me[i+3].specular = D3DRGBA(0,0,0,1);
		
		me[i].rhw = 1;
		me[i+1].rhw = 1;
		me[i+2].rhw = 1;
		me[i+3].rhw = 1;
		
	}

	i = 0;

	for (rect.top = 0, rect.bottom = SCREENTEX_SIZE; rect.top < BLUR_Y; rect.top += SCREENTEX_SIZE, rect.bottom += SCREENTEX_SIZE)
		for (rect.left = 0, rect.right = SCREENTEX_SIZE; rect.left < BLUR_X; rect.left += SCREENTEX_SIZE, rect.right += SCREENTEX_SIZE)
		{
			me[i+0].sx = rect.left + BLUR_XOFS;
			me[i+0].sy = rect.top;
			me[i+1].sx = rect.right + BLUR_XOFS;
			me[i+1].sy = rect.top;
			me[i+2].sx = rect.right + BLUR_XOFS;
			me[i+2].sy = rect.bottom;
			me[i+3].sx = rect.left + BLUR_XOFS;
			me[i+3].sy = rect.bottom;

			i+=4;
		}

	return me;
}

void fxBlurSurface(LPDIRECTDRAWSURFACE7 screen)
{
	DrawScreenOverlays();
	GrabScreenTextures(screen,tList);
}

#ifdef __cplusplus
}
#endif
