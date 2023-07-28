/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: fadeout.h
	Programmer	: David
	Date		: 
	Purpose		: Transition effects 'n shit

----------------------------------------------------------------------------------------------- */

#ifndef _FADEOUT_H
#define _FADEOUT_H

#ifdef __cplusplus
extern "C"
{
#endif

extern int fadingOut, keepFade;
extern int fadeText;
extern int flashScreen;

void ScreenFade(int start, int end, long time);
void DrawScreenTransition(void);

extern int (*fadeProc)(void);


#ifdef __cplusplus
}
#endif

#endif