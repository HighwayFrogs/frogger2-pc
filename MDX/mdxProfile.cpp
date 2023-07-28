/*

	This file is part of the M libraries,

	File		: 
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
#include "mdxMath.h"
#include "mdxTexture.h"
#include "mdxObject.h"
#include "mdxActor.h"
#include "mdxCRC.h"
#include "mgeReport.h"
#include "mdxPoly.h"
#include "mdxWindows.h"
#include "mdxTiming.h"
#include "gelf.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_TIMERS 20
#define FRAMES_DISP 5
long tStart[MAX_TIMERS];
long tTotal[MAX_TIMERS];
long tEnd[MAX_TIMERS];
long sStart[MAX_TIMERS];
long sTotal[MAX_TIMERS];
long sEnd[MAX_TIMERS];
long tS;
float timeToUpdate;

long tHold[MAX_TIMERS];

char *tName[MAX_TIMERS];
float timerscale = 0.15*1000;
long pFreq;

#define MAX_SAMPLES 32000

long graphSTimes[MAX_SAMPLES];
long graphSValues[MAX_SAMPLES];
long graphNumPoints;
long graphMax;
long graphStart;
long graphEnd;

void InitProfile(void)
{
	LARGE_INTEGER c;

	QueryPerformanceFrequency(&c);
	pFreq = c.QuadPart;
}

void StartGraph(void)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	graphNumPoints = 0;
	graphStart = c.QuadPart;
}

void EndGraph(long maxVal)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	
	graphEnd = c.QuadPart;
	graphMax = maxVal;
}

void SampleGraph(long value)
{
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);

	if( graphNumPoints > MAX_SAMPLES )
		return;

	graphSTimes[graphNumPoints] = c.QuadPart;
	graphSValues[graphNumPoints++] = value;
}

long GetTimer(void)
{
	LARGE_INTEGER pCount;
	QueryPerformanceCounter(&pCount);
	return pCount.QuadPart;

}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

long first = 1;

void ClearTimers(void)
{
	FILE *fp;
	if (first)
	{
		fp = fopen("c:\\hold.hif","rb");
	}
	if (timeToUpdate<=0)
			timeToUpdate = 60;
		else
			timeToUpdate--;
	
	for (int i=0; i<MAX_TIMERS; i++)
	{
		tStart[i] = 0;
		tName[i] = 0;
		tEnd[i] = 0;
		tTotal[i] = 0;
		
		sTotal[i] = 0;

		if (timeToUpdate<=0)
		{
			sStart[i] = 0;
			sEnd[i] = 0;
			
		}		
		//tS = timeGetTime();
		tS = GetTimer();
		if (first)
		{
			if (fp)
				fread (&tHold[i],4,1,fp);
		}
	}

		
	if (first)
	{
		if (fp)
			fclose(fp);
		first = 0;
	}
}

void HoldTimers(void)
{
	FILE *fp = fopen("c:\\hold.hif","wb");
	
	for (int i=0; i<MAX_TIMERS; i++)
	{
		tHold[i] = tTotal[i];

		if (fp)
			fwrite (&tHold[i],4,1,fp);
	
	}

	if (fp)
		fclose(fp);	
	
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void StartTimer(int number,char *name)
{
	if (number<MAX_TIMERS)
	{
		tName[number] = name;
		tStart[number] = GetTimer();

		if ((timeToUpdate<=0) && sStart[number]==0)
			pDirectDraw7->GetScanLine((unsigned long *)&sStart[number]);
	}
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

void EndTimer(int number)
{
	if (number<MAX_TIMERS)
	{
		tEnd[number] = GetTimer();
		tTotal[number]+=tEnd[number]-tStart[number];		

		if (timeToUpdate<=0)
		{
			pDirectDraw7->GetScanLine((unsigned long *)&sEnd[number]);			
		}
	//	else
	//		sTotal[number]+=sEnd[number]+(480-sStart[number]);		
	}
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/

unsigned long timeY = 200;
D3DDEVINFO_TEXTURING t;
D3DDEVINFO_TEXTUREMANAGER t2;
long thrashes = 0;

void PrintTimers(void)
{
	RECT r;
	int i;
	HDC hdc;

	pDirect3DDevice->GetInfo(D3DDEVINFOID_TEXTUREMANAGER ,&t2,sizeof(t2));
//		dp("Release D3D installed, couldn't retrieve texture info\n");

	if (t2.bThrashing)
		thrashes++;

	//BeginDraw();
	
	float gMult = 560.0/(graphEnd-graphStart);
	float gVMult = 100.0/graphMax;
	
	r.top = timeY+50-(200/MAX_TIMERS);
	r.bottom = timeY+49+((MAX_TIMERS+2)*(200/MAX_TIMERS));
	r.left = 40;
	r.right = 600;
	DrawFlatRect(r, D3DRGBA(0,0.1,0,0.8));


	r.right = 45;
	DrawFlatRect(r, D3DRGBA(0,0,1,0.8));

	r.left = 595;
	r.right = 600;
	DrawFlatRect(r, D3DRGBA(0,0,1,0.8));
	
	r.bottom = r.top+9;
	r.bottom -= 37;
	r.left = 40;
	r.right = 600;
	DrawFlatRect(r, D3DRGBA(0,0,1,0.8));
	
	r.bottom = timeY+49+((MAX_TIMERS+2)*(200/MAX_TIMERS));
	r.top = r.bottom - 5;
	DrawFlatRect(r, D3DRGBA(0,0,1,0.8));

	if (graphNumPoints>3)
		for (i=0; i<graphNumPoints-1; i++)
		{
			long xAim = 40+((graphSTimes[i+1] - graphStart) * gMult);
			long xVal = 40+((graphSTimes[i] - graphStart) * gMult);
			
			r.left = xVal;
			r.right = xAim;
			r.bottom = timeY+49+((MAX_TIMERS+2)*(200/MAX_TIMERS));

			if ((graphSValues[i]*gVMult) > graphMax)
				r.top = r.bottom - graphMax;
			else
				r.top = r.bottom - (graphSValues[i]*gVMult);
			float c = (r.bottom-r.top)/(float)graphMax;
			
			DrawFlatRect(r, D3DRGBA(1,c,0,0.5));			
		}

	
for ( i=0; i<MAX_TIMERS; i++)
	{
		r.top = timeY+50+(i*(200/MAX_TIMERS));
		r.bottom = timeY+49+((i+1)*(200/MAX_TIMERS));
		r.left = (50+tHold[i]/timerscale);;
		r.right = (52+tHold[i]/timerscale);
		
		DrawFlatRect(r, D3DRGBA(1,0.5,1,0.8) );
	}
	
	for (i=0; i<MAX_TIMERS; i++)
	{
		r.top = timeY+50+(i*(200/MAX_TIMERS));
		r.bottom = timeY+49+((i+1)*(200/MAX_TIMERS));
		r.left = 50;
		r.right = 52;
		DrawFlatRect(r, D3DRGBA(1,1,1,0.8) );
		//r.left = 50+(tStart[i] - tStart[0])/timerscale;
		r.right = (r.left+tTotal[i]/timerscale);
		DrawFlatRect(r, D3DRGBA(1,0,0,0.5) );

		r.left = 50+(tStart[i] - tStart[0])/timerscale;
		r.right = (r.left+tTotal[i]/timerscale);
		DrawFlatRect(r, D3DRGBA(0,1,0,0.25) );

	}


	if (timerDraw==2)
	{
		//EndDraw();
			
		HRESULT res = IDirectDrawSurface4_GetDC(surface[RENDER_SRF], &hdc);

		
		if ((res == DD_OK))
		{
			char tText[256];
			long tPC = 0,cPC;
			HFONT hfnt, hOldFont;      
			hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT); 

			
/*
dwNumEvicts 
Number of textures that were removed during the last frame. 
dwNumVidCreates 
Number of textures that were created in video memory during the last frame. 
dwNumTexturesUsed 
Total number of textures used during the last frame. 
dwNumUsedTexInVid 
Number of video memory textures that were used during the last frame. 
dwWorkingSet 
Number of textures currently resident in video memory. 
dwWorkingSetBytes 
Number of bytes currently allocated by textures resident in video memory. 
dwTotalManaged 
Total number of managed textures. 
dwTotalBytes 
Total number of bytes allocated for managed textures. 
dwLastPri 
Priority of last evicted texture. 
*/
				
			if (hOldFont = (HFONT)SelectObject(hdc, hfnt)) 
			{
				SetBkMode(hdc, TRANSPARENT);
				
				sprintf(tText,"Texturing info: %lu Thrashes, %lu Used Last Frame, %lu Textures in Vid Ram, %lu Total Textures = %lu Bytes",thrashes, t2.dwNumTexturesUsed, t2.dwWorkingSet, t2.dwTotalManaged, t2.dwTotalBytes);
				SetTextColor(hdc, RGB(55,0,0));
				TextOut(hdc, 61, timeY+20, tText, strlen(tText));
				SetTextColor(hdc, RGB(255,255,255));
				TextOut(hdc, 60, timeY+20, tText, strlen(tText));
					
				for (i=0; i<MAX_TIMERS; i++)
					if (tName[i])
					{		
						cPC = (tTotal[i]*100)/tTotal[0];
						sprintf(tText,"(%05lu) [%03lu - %03lu]  %02lu%% - %s",tTotal[i],sStart[i],sEnd[i],cPC,tName[i]);
						tPC+=cPC;
						SetTextColor(hdc, RGB(55,0,0));
						TextOut(hdc, 61, timeY+49+(i*(200/MAX_TIMERS)), tText, strlen(tText));
						SetTextColor(hdc, RGB(255,255,255));
						TextOut(hdc, 60, timeY+48+(i*(200/MAX_TIMERS)), tText, strlen(tText));
					}

				tPC-=100;
				
				sprintf(tText,"%02lu%%",tPC);
				SetTextColor(hdc, RGB(255,255,255));
				TextOut(hdc, 61, timeY, tText, strlen(tText));
						
				SelectObject(hdc, hOldFont); 
			}

			IDirectDrawSurface4_ReleaseDC(surface[RENDER_SRF], hdc);	
		}

		//BeginDraw();
	}


	for (i=0; i<FRAMES_DISP; i++)
	{
		r.top = timeY+50+(0*(200/MAX_TIMERS));
		r.bottom = timeY+49+((MAX_TIMERS)*(200/MAX_TIMERS));
	
		r.left = (50+((i*pFreq)/60)/timerscale);;
		r.right = (52+((i*pFreq)/60)/timerscale);

		DrawFlatRect(r, D3DRGBA(1,1,1,0.5) );
	}

	//EndDraw();
	
}

#ifdef __cplusplus
}
#endif
