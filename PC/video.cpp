/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: video.h
	Programmer	: David Swift
	Date		: 28 May 00, a bloody sunday no less

----------------------------------------------------------------------------------------------- */

#include <islutil.h>
#include "story.h"
#include "video.h"

#ifdef USE_BINK_VIDEO
#include <windows.h>
#include <d3d.h>
#include <mdx.h>
#include <stdio.h>

#include "layout.h"
#include "islpad.h"
#include "controll.h"
#include "pcaudio.h"
#include "dx_sound.h"
#include "main.h"
#include "game.h"
#include <bink.h>

static HBINK bink=0;
static s32 surfacetype=0;
bool blitVideo = 0;

#define VIDEOPATH "Video\\"

long (*oldLoop)() = 0;

LPDIRECTDRAWSURFACE7 tmpSrf;

#define videoSurface surface[PRIMARY_SRF]
long RunVideoPlayback()
{
	if (!BinkWait(bink))
	{
		// decompress a frame
		BinkDoFrame(bink);

		DDSURFACEDESC2 ddsd; ddsd.dwSize = sizeof(DDSURFACEDESC2);
		HRESULT res;
		RECT r;

		if (blitVideo)
		{
			if (!rFullscreen)
			{
				// Here's some scaling fun ..
				// if we're running in a window, we want to keep the right aspect ratio but fit
				// the whole video in the window.
				// 640x400 is 8:5 ratio, so that's what all the 8s and 5s mean

				RECT clientr;
				int width, height;

				GetClientRect(mdxWinInfo.hWndMain, &clientr);

				width = clientr.right;
				height = clientr.bottom;

				ClientToScreen(mdxWinInfo.hWndMain, (POINT*)&clientr.left);
				clientr.right += clientr.left;
				clientr.bottom += clientr.top;
				
				// pick the proportionally biggest dimension (width or height)
				// centre the rectangle in this direction, spanning the other

				if (width*5 < height*8)
				{
					r.bottom = (width*5)/8;
					r.left = clientr.left;
					r.right = clientr.right;
					r.top = ((height-r.bottom)/2)+clientr.top;
					r.bottom += r.top;
				}
				else
				{
					r.right = (height*8)/5;
					r.top = clientr.top;
					r.bottom = clientr.bottom;
					r.left = ((width-r.right)/2)+clientr.left;
					r.right += r.left;
				}
			}
			else
			{
				r.bottom = (rXRes*5)/8;

				if (r.bottom > rYRes) //This prevents integer underflow that prevents cutscenes from showing in fullscreen mode
				{
					r.bottom = rYRes;
					r.right = (rYRes*8)/5;
					r.top = 0;
					r.left = (rXRes - r.right)/2;
					r.right += r.left;
				}

				else
				{
					r.right = rXRes;
					r.left = 0;
					r.top = (rYRes-r.bottom)/2;
					r.bottom += r.top;
				}
			}

			if ((res = tmpSrf->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, 0)) != DD_OK)
			{
				utilPrintf("RunVideoPlayback(): Failed locking temporary surface\n");
				return 1;
			}

			BinkCopyToBuffer(bink, ddsd.lpSurface, ddsd.lPitch, bink->Height,
				0, 0, surfacetype);

			RECT sr;
			sr.left = 0;
			sr.right = bink->Width;
			sr.top = 0;
			sr.bottom = bink->Height;

			tmpSrf->Unlock(NULL);

			surface[PRIMARY_SRF]->Blt(&r, tmpSrf, &sr, DDBLT_WAIT, 0);
		}
		else
		{
			if ((res = videoSurface->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, 0)) != DD_OK)
			{
				utilPrintf("RunVideoPlayback(): Failed locking primary surface\n");
				return 1;
			}

			// copy the data onto the screen .. hm
			BinkCopyToBuffer(bink, ddsd.lpSurface, ddsd.lPitch, bink->Height,
				(rXRes-bink->Width)/2, (rYRes-bink->Height)/2, surfacetype);

			videoSurface->Unlock(NULL);
		}

		ProcessUserInput();

		if (bink->FrameNum == bink->Frames || padData.debounce[0])
		{
			// end playback mode..
			BinkClose(bink); bink=NULL;

			tmpSrf->Release();

			if(padData.debounce[0] & PAD_START)
				quitAllVideo = YES;
			//AppLoop = oldLoop;
			return 1;
		}
		else
			BinkNextFrame(bink);
	}


	return 0;
}


DWORD CALLBACK VideoThreadProc(LPVOID foo)
{
	while (!RunVideoPlayback());
	
	return 0;
}


long StartVideoPlayback(int num)
{
	char path[MAX_PATH];
	HRESULT res;
	DDSURFACEDESC2	ddsd;

	BinkSoundUseDirectSound(lpDS);
	ZeroMemory(path, MAX_PATH);
	sprintf(path, "%s" VIDEOPATH "%s.bik", baseDirectory, fmv[num].name);

	bink = BinkOpen(path, 0);
	if (!bink)
	{
		utilPrintf("StartVideoPlayback(): Bink failed opening '%s'\n", path);
		ZeroMemory(path, MAX_PATH);
		sprintf(path, "%s" VIDEOPATH "%s.bik", cdromDrive, fmv[num].name);
	}
	

	if (bink)
	{
		utilPrintf("StartVideoPlayback(): Starting video: '%s'\n", path);
	}
	else
	{
		utilPrintf("StartVideoPlayback(): Bink failed opening '%s', skipping video playback\n", path);
		return 0;
	}

	DDINIT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	if (rHardware)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;// | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bink->Width;
	ddsd.dwHeight = bink->Height;
	if ((res = pDirectDraw7->CreateSurface(&ddsd, &tmpSrf, NULL))!= DD_OK)
	{
		dp("Failed creating temporary surface\n");
		ddShowError(res);
		return 0;
	}

	surfacetype = BinkDDSurfaceType(videoSurface);

	if (!rFullscreen || !(rXRes == 640 && rYRes == 480))
	{
		blitVideo = 1;
	}

	DDBLTFX bltfx;
	bltfx.dwSize = sizeof(DDBLTFX);
	bltfx.dwFillColor = 0;

	videoSurface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&bltfx);

	//oldLoop = AppLoop;
	//AppLoop = RunVideoPlayback;

	DWORD id;
	HANDLE videoThread = CreateThread(0, 0, VideoThreadProc, 0, 0, &id);


	MSG msg;
	while (WaitForSingleObject(videoThread, 100) == WAIT_TIMEOUT)
	{
		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(msg.message == WM_QUIT || msg.message == WM_CLOSE)
			{
				ExitProcess(0);
				return 1;
			}

			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
		// do nothing, every second, until the thread is finished.		
	}

	return 1;
}

/* ----------------------------------------------------------------------- */

#else // if we're compiling without video support..

long RunVideoPlayback()
{
	return 1;
}

long StartVideoPlayback(int num)
{
	utilPrintf("StartVideoPlayback(): No video support compiled, skipping '%s'\n", fmv[num].name);
	return 0;
}

#endif // INCLUDE_VIDEO