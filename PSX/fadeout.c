/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: fadeout.cpp
	Programmer	: David
	Date		: 
	Purpose		: Transition effects 'n shit

----------------------------------------------------------------------------------------------- */

#include "fadeout.h"
#include "layout.h"
#include "sonylibs.h"
#include "shell.h"
#include "LoadingBar.h"


long fadeoutStart, fadeoutLength;
int fadingOut = 0, keepFade = 0;

int fadedOutCount;

int fadeText = YES;
int flashScreen = NO;

int (*fadeProc)(void);

int startIntensity = 0, endIntensity = 255;

/*	-------------------------------------------------------------------------------
	Function:	ScreenFade
	Params:		start, end intensity (0-255), time in frames
	returns:	
*/
int DrawScreenFade(void)
{
//	RECT r = { 0, 0, rXRes, rYRes };
	POLY_F4 *f4;
	DR_MODE *dm;
	int col;
	int halfX=256, halfY,depth;
	int count;

	if(loadingDisplay)
		count = loadFrameCount;
	else
		count = actFrameCount;
	
	if (count > (fadeoutStart+fadeoutLength))
	{
		col = endIntensity;
		fadedOutCount++;
	}
	else
		col = startIntensity + ((endIntensity-startIntensity)*(long)(count-fadeoutStart))/fadeoutLength;

//		col = startIntensity + ((endIntensity-startIntensity)*(count-fadeoutStart))/fadeoutLength;

//	D3DSetupRenderstates(xluSubRS);
//	DrawFlatRect(r, RGBA_MAKE(col, col, col, 255));
//	D3DSetupRenderstates(xluSemiRS);

	#if PALMODE==1
		halfY = 128;
	#else
		halfY = 120;
	#endif

	if(fadeText)
		depth = 0;
	else
		depth = 5;

	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	f4->x0 = -halfX;
	f4->y0 = -halfY;
	f4->x1 = halfX;
	f4->y1 = -halfY;
	f4->x2 = -halfX;
	f4->y2 = halfY;
	f4->x3 = halfX;
	f4->y3 = halfY;
	f4->r0 = col;
	f4->g0 = col;
	f4->b0 = col;
	setSemiTrans(f4,1);
	ENDPRIM(f4, depth, POLY_F4);
	BEGINPRIM(dm, DR_MODE);
	if(flashScreen)
		SetDrawMode(dm, 0,1, ((SEMITRANS_ADD-1)<<5),0);
	else
		SetDrawMode(dm, 0,1, ((SEMITRANS_SUB-1)<<5),0);
	ENDPRIM(dm, depth, DR_MODE);

	

	return (fadedOutCount > 1);
}


/*	-------------------------------------------------------------------------------
	Function:	ScreenFade
	Params:		start, end intensity (0-255), time in frames
	returns:	
*/
void ScreenFade(int start, int end, long time)
{
	fadeProc = DrawScreenFade;
	if(loadingDisplay)
		fadeoutStart = loadFrameCount;
	else
		fadeoutStart = actFrameCount;
	fadeoutLength = time;
	fadingOut = 1;

	// cause we're using subtractive
	startIntensity = 255-start;
	endIntensity = 255-end;
	fadedOutCount = 0;
	fadeText = YES;
	flashScreen = NO;
}


void StopFadeout()
{
	fadeProc = NULL;
	fadingOut = 0;
}

/*	-------------------------------------------------------------------------------
	Function:	MainWndProc
	Params:		As WNDPROC
	returns:	0 for success, other to pass on to mdx default handler
*/
void DrawScreenTransition(void)
{
//	if (KEYPRESS(DIK_F))
//		ScreenFade(0, 255, 60);

	if (fadeProc)
		if (fadeProc())
		{
			fadingOut = 0;
			
			if (!keepFade)
				fadeProc = NULL;
		}
}
