/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>

#include "mdxTiming.h"
#include "mgeReport.h"


MDX_TIMING_VAL timeInfo;

/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void InitTiming(float frameSpeed)
{
	timeInfo.firstTicks = timeGetTime();
	timeInfo.tickCount = 0;
	timeInfo.frameCount = 0;
	timeInfo.frameSpeed = frameSpeed;
	timeInfo.tickModifier = 0;
	timeInfo.maxTickIncrement = 500;	// 500ms default
}

/*	--------------------------------------------------------------------------------
	Function	: 
	Purpose		: 
	Parameters	: 
	Returns		: 
	Info		: 
*/

void UpdateTiming(void)
{
	float mult = (timeInfo.frameSpeed/1000.0F);
	unsigned long currentTicks;
	
	// Ensure at least one millisecond passes to avoid having ticks with a gameSpeed (deltaTime) of zero.
	// I really wanted a better solution than this, but unfortunately due to using fixed point numbers we'd need to rewrite a LOT of the game in order to do a proper fix, so this will have to do.
	do {
		currentTicks = (timeGetTime()+timeInfo.tickModifier) - timeInfo.firstTicks;
	} while (currentTicks == timeInfo.tickCount);
	
	// limit maximum clock increment
	unsigned long intervalTicks = currentTicks - timeInfo.tickCount;
	if (intervalTicks > timeInfo.maxTickIncrement)
	{
		intervalTicks -= timeInfo.maxTickIncrement;
		timeInfo.firstTicks += intervalTicks;
		currentTicks -= intervalTicks;
		
		intervalTicks = timeInfo.maxTickIncrement;
	}
	
	timeInfo.speed = intervalTicks * mult;
	timeInfo.frameCount = (unsigned long)(currentTicks * mult);
	timeInfo.tickCount = currentTicks;
}

