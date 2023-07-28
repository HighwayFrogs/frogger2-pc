/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXWINDOWS_H_INCLUDED
#define MDXWINDOWS_H_INCLUDED

#ifdef __cplusplus

extern "C"
{
#endif

typedef struct TAG_MDX_WININFO
{
	HWND hWndMain;
	HINSTANCE hInstance;
} MDX_WININFO;

extern MDX_WININFO mdxWinInfo;
extern unsigned long consoleDraw;
extern unsigned long timerDraw;
extern unsigned long textureDraw;
extern long windowActive;
extern long displayDebugInfo;
extern long (*AppLoop)();

int WindowsInitialise(HINSTANCE hInstance, char *appName, long debugMode);
void RunWindowsLoop();
WNDPROC mdxSetUserWndProc(WNDPROC);

#ifdef __cplusplus
}
#endif

#endif