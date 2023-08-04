/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: drawloop.cpp
	Programmer	: David Swift
	Date		: 16 Jun 00

----------------------------------------------------------------------------------------------- */


#include <math.h>
#include <islutil.h>
#include <islpad.h>

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>
#include <mdxException.h>
#include <softstation.h>

// common
#include "frogmove.h"
#include "cam.h"
#include "tongue.h"
#include "specfx.h"
#include "menus.h"

// pc
#include "drawloop.h"
#include "fadeout.h"
#include "pcsprite.h"
#include "pcgfx.h"
#include "backdrop.h"
#include "ptexture.h"
#include "dx_sound.h"
#include "banks.h"
#include "controll.h"
#include "pcmisc.h"
#include "main.h"
#include "..\resource.h"
#include "fxBlur.h"
#include "game.h"

/*
#include "..\network.h"
#include "..\netchat.h"
#include "..\netgame.h"
*/

#include "editor.h"

extern "C" {MDX_LANDSCAPE *world;}

/*	--------------------------------------------------------------------------------
	Function	: WinMain
	Purpose		: Application startup and shutdown
	Parameters	: the usual...
	Returns		: success
*/

#define CAMVECTSCALE (1.0f/40960.0f)

void CalcViewMatrix(long uDate)
{
	guLookAtF (vMatrix.matrix,
		currCamTarget.vx*CAMVECTSCALE, currCamTarget.vy*CAMVECTSCALE, currCamTarget.vz*CAMVECTSCALE,
		currCamSource.vx*CAMVECTSCALE, currCamSource.vy*CAMVECTSCALE, currCamSource.vz*CAMVECTSCALE,
		camVect.vx*CAMVECTSCALE, camVect.vy*CAMVECTSCALE, camVect.vz*CAMVECTSCALE,uDate,aspectRatio);

//	sheenCam.vx = currCamSource.vx*CAMVECTSCALE;
//	sheenCam.vy = currCamSource.vy*CAMVECTSCALE;
//	sheenCam.vz = currCamSource.vz*CAMVECTSCALE;
}


/*	--------------------------------------------------------------------------------
	Function	: DrawBackground
	Purpose		: 
	Parameters	: void
	Returns		: void
*/
void DrawBackground(void)
{
	
	float oF = farClip;
//	float oFs = fStart, oFe = fEnd;
	if (!drawGame)
		return;

	SwapFrame(0);
	backGnd->actor->visible = 1;
	
	noClipping = 1;
	farClip = 124000;
	((MDX_ACTOR *)backGnd->actor->actualActor)->pos.vx = currCamSource.vx / 40960.0;
	((MDX_ACTOR *)backGnd->actor->actualActor)->pos.vy = currCamSource.vy / 40960.0;
	((MDX_ACTOR *)backGnd->actor->actualActor)->pos.vz = currCamSource.vz / 40960.0;
		
	XformActor(((MDX_ACTOR *)backGnd->actor->actualActor),1);
	DrawActor(((MDX_ACTOR *)backGnd->actor->actualActor));

	noClipping = 0;

	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
	//pDirect3DDevice->SetRenderState(pDirect3DDevice,D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);	// clamp textures
	//pDirect3DDevice->lpVtbl->SetRenderState(pDirect3DDevice,D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_NEAREST);//D3DFILTER_LINEAR);

//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,TRUE);
//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF,0);
//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,D3DCMP_NOTEQUAL);

	//D3DSetupRenderstates(xluAddRS);

	DrawBatchedPolys();
	//D3DSetupRenderstates(xluSemiRS);

	// Draw the second mavis frame set, Transparent objects (non water objects)
//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
	//pDirect3DDevice->lpVtbl->SetRenderState(pDirect3DDevice,D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);	// wrap textures
	//pDirect3DDevice->lpVtbl->SetRenderState(pDirect3DDevice,D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_NEAREST);
	
	BlankFrame;

	farClip = oF;
}


/* -----------------------------------------------------------------------
   Function : DrawLoop
   Purpose : main draw loop pass
   Parameters : rectangle, colour, mdx texture pointer, u0,v0 pair, u1,v1 pair
   Returns : 1 draw error else 0 okay
   Info : 
*/
extern int winActive;
long DrawLoop(void)
{
	POINT	t;

	if(!winActive)
		return 0;

	if(gameState.mode == ARTVIEWER_MODE || gameState.mode == STARTUP_MODE || gameState.mode == TEASERSCREEN_MODE )
	{
		if( rHardware )
			BeginDraw();

		DrawBackdrop();

		// Can't use fading in SW - it uses a poly which fucks with the backdrop...
		if( rHardware )
		{
			DrawScreenTransition();
			EndDraw();
		}
		else // But we do have to complete the fade
		{
			fadingOut = 0;
			fadeProc = NULL;
		}

		DDrawFlip();

		return 0;
	}

	// Just to get functionality... ;)
	StartTimer (2,"Viewing, bg, fog");
//	DrawActorList();

	D3DSetupRenderstates(D3DDefaultRenderstates);

	if ((gameState.mode != FRONTEND_MODE) && (editorOk || fixedPos))
		CalcViewMatrix(0);
	else
		CalcViewMatrix(1);

	//	changedView = 1;

	SetupFogParams(fog.min,0,0,0,0);
	DrawBackdrop();

	BeginDraw();

	if(tileTexture[0])
		DrawTiledBackdrop();

	if((backGnd) && (rHardware))
		DrawBackground();
	//EndDraw();

	farClip = (float)fog.max*0.1;
//	if (fog.mode && fog.min>0)
//	{
//		SetupFogParams(fog.min,fog.r/255.0,fog.g/255.0,fog.b/255.0,1);
//		fogEnable = 1;
//	}
	
	BlankAllFrames();
	SwapFrame(MA_FRAME_NORMAL);
	EndTimer(2);

	if (drawGame)
	{
		D3DSetupRenderstates(cullCWRS);

	//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_NEAREST);

		StartTimer(14,"Landscape");
		if (world && drawLandscape)
			DrawLandscape(world);
		EndTimer(14);

		
		StartTimer(1,"Actors and Shadows");
		// Draw Actors before shadows
		D3DSetupRenderstates(normalZRS);
		ActorListDraw(0);

		if((!backdrop) && (!tileTexture[0]))
		{
			DrawAllFrames();
			BlankAllFrames();

			D3DSetupRenderstates(xluZRS);
			D3DSetupRenderstates(normalAlphaCmpRS);
			ProcessShadows();

			//BeginDraw();
			DrawBatchedPolys();
			//EndDraw();

			BlankAllFrames();
			D3DSetupRenderstates(normalZRS);
			SwapFrame(0);
		}

		// Draw actors after shadows
		ActorListDraw(1);
		EndTimer(1);

		if( gameState.multi == SINGLEPLAYER )
		{
			UpdateFrogTongue(0);
			UpdateFrogCroak(0);
		}
		
		StartTimer(15,"DAF");
		if (rHardware)
		{
			DrawAllFrames();
			BlankAllFrames();
		}
		EndTimer(15);

		//BeginDraw();
		
		StartTimer(16,"Sprites,text & SpecFX");
	//	pDirect3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_LINEAR);
		D3DSetupRenderstates(xluZRS);
		D3DSetupRenderstates(cullNoneRS);
		
		// Draw Sprites
		if(sprList.count)
			PrintSprites();

		D3DSetupRenderstates(xluZRS);
		// FX and shadows
		if((gameState.mode != MULTI_WINRACE_MODE) && (gameState.mode != MULTI_WINBATTLE_MODE) && (gameState.mode != MULTI_WINCOLLECT_MODE) && (gameState.mode != LEVELCOMPLETE_MODE))
			DrawSpecialFX();

		// Light halos
	//	CheckHaloPoints();
	//	DrawHalos();
	}
	else
	{
		D3DSetupRenderstates(xluZRS);
		D3DSetupRenderstates(cullNoneRS);
	}

	if (rHardware)
	{
		SwapFrame(MA_FRAME_NORMAL);
		DrawBatchedPolys();
		BlankFrame;

		D3DSetupRenderstates(xluAddRS);
		SwapFrame(MA_FRAME_ADDITIVE);
		DrawBatchedPolys();
		BlankFrame;

		D3DSetupRenderstates(xluSemiRS);
		SwapFrame(MA_FRAME_NORMAL);

		D3DSetupRenderstates(noZRS);  // And disable z tests
	}
	else
	{
//		D3DSetupRenderstates(xluAddRS);

		DrawSoftwarePolys();
		//EndDraw();
		ClearSoftwareSortBuffer();
//		DrawAllFrames();
//		BlankAllFrames();
	}

	D3DSetupRenderstates(cullNoneRS);

	if(!fadeText)
		DrawScreenTransition();

//	D3DSetupRenderstates(cullCWRS);

	PrintSpriteOverlays(0);	
	PrintTextOverlays();

	PrintSpriteOverlays(1);	
	PrintSpriteOverlays(2);	

	if(fadeText)
		DrawScreenTransition();

#ifndef FINAL_MASTER
	if (editorOk)
		DrawEditor();
#endif

	EndDraw();
	EndTimer(16);

	EndTimer(0);

	if (textureDraw)
		ShowTextures();
	if( showSounds )
		ShowSounds( );

	if (consoleDraw)
		PrintConsole();
	if (timerDraw)
		PrintTimers();
	
	ClearTimers();
	StartTimer(0,"Everything");


	// ** Flip the screen
	
	StartTimer(17,"Flip");

	if( screenshotEnable )
		if (KEYPRESS(DIK_F9))
			ScreenShot();

	//GrabSurfaceToTexture(100, 50, GetTexEntryFromCRC(UpdateCRC("febwood.bmp")),surface[RENDER_SRF]);	
//	fxBlurSurface(surface[RENDER_SRF]);

	// *ASL* 13/06/2000
	if( !rHardware )
		SurfaceDraw();

	if((rHardware) || (gameState.mode != STARTUP_MODE))
		DDrawFlip();
	EndTimer(17);

// This ain't printing to the screen but to the textures - used specifically for the storybook in the frontend

	if (grabToTexture == 1)
	{
		DrawPageB();
		GrabSurfaceToTexture(0, 0, GetTexEntryFromCRC(UpdateCRC("page256b.bmp")),surface[RENDER_SRF]);	
		grabToTexture = 0;
	}
	else if (grabToTexture == 3)
	{
		DrawPageB();
		GrabSurfaceToTexture(0, 0, GetTexEntryFromCRC(UpdateCRC("page256a.bmp")),surface[RENDER_SRF]);	
		grabToTexture = 2;
	}

	StartTimer(18,"Clear");
	D3DClearView();
	EndTimer(18);

	GetCursorPos(&t);
	camZ = t.x*8;
	camY = t.y*8;

	CleanBufferSamples();

	StartTimer(19,"PText");
	if((gameState.mode != PAUSE_MODE ) && (gameState.mode != GAMEOVER_MODE))
		ProcessProcTextures( );
	EndTimer(19);
	
	UpdateAnimatingTextures();
	//UpdateTextureText();

	return 0;
}
