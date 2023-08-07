/*

	This file is part of the M libraries,

	File		: mliReport.h
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXDDRAW_H_INCLUDED
#define MDXDDRAW_H_INCLUDED

#ifdef __cplusplus

extern "C"
{
#endif

#define RELEASE(x) if (x) {x->Release(); x=NULL;}
#define DDINIT(x) {memset(&x,0,sizeof(x)); x.dwSize = sizeof(x);}

enum 
{
	PRIMARY_SRF,
	RENDER_SRF,
	ZBUFFER_SRF,
	SPARE_SRF,
	NUM_SRF
};

extern LPDIRECTDRAW7		pDirectDraw7;
extern LPDIRECTDRAWCLIPPER	pClipper;
extern LPDIRECTDRAWSURFACE7	surface[NUM_SRF];
extern unsigned long		rXRes, rYRes, rBitDepth, r565, rHardware, rFullscreen, rScale, rFlipOK, rAltZBuffer;
extern HWND					rWin;
extern char					rVideoDevice[256];

extern LPDIRECTDRAWSURFACE7	backdrop;

/*	--------------------------------------------------------------------------------
	Function	: DDrawInitObject
	Purpose		: Initialise Directdraw objects
	Parameters	: Guid of device
	Returns		: success
	Info		: 
*/

unsigned long DDrawInitObject (DWORD resolution);

/*	--------------------------------------------------------------------------------
	Function	: DDrawInitObject
	Purpose		: Initialise Directdraw objects
	Parameters	: Guid of device
	Returns		: success
	Info		: 
*/

unsigned long DDrawShutdown (void);

/*	--------------------------------------------------------------------------------
	Function	: DDrawCreateSurfaces
	Purpose		: Create all our surfaces
	Parameters	: xRes,yRes & Bitdepth, whether we will want a 3D capable render surface, and whether we would want a zbuffer
	Returns		: success
	Info		: 
*/

unsigned long DDrawCreateSurfaces(HWND window, unsigned long xRes, unsigned long yRes, unsigned long bitDepth, unsigned long want3D, unsigned long zBits);

/*	--------------------------------------------------------------------------------
	Function	: DDrawAttachSurface
	Purpose		: Attach two surfaces
	Parameters	: A is surface that B attaches to.
	Returns		: success
	Info		: 
*/

unsigned long DDrawAttachSurface(unsigned long srfA, unsigned long srfB);

/*	--------------------------------------------------------------------------------
	Function	: DDrawSetupWindow
	Purpose		: Setup for widowing
	Parameters	: The window that will contain the view of the primary surface, and whether to scale to that window if not fullscreen
	Returns		: success
	Info		: 
*/

unsigned long DDrawSetupWindow(HWND window, unsigned long scaled);

/*	--------------------------------------------------------------------------------
	Function	: DDrawSetupWindow
	Purpose		: Setup for widowing
	Parameters	: The window that will contain the view of the primary surface
	Returns		: success
	Info		: 
*/

void DDrawFlip(void);

/*	--------------------------------------------------------------------------------
	Function	: ClearSurface
	Purpose		: Clear a surface (Depth or color)
	Parameters	: Surface Number, value to fill with, DDBLT_DEPTHFILL or DDBLT_COLORFILL
	Returns		: success
	Info		: 
*/

void DDrawClearSurface(unsigned long srfN, unsigned long value, unsigned long fillType);

/*	--------------------------------------------------------------------------------
	Function	: ClearSurface
	Purpose		: Clear a surface (Depth or color)
	Parameters	: Surface Number, value to fill with, DDBLT_DEPTHFILL or DDBLT_COLORFILL
	Returns		: success
	Info		: 
*/

WNDPROC SetUserVideoProc(WNDPROC proc);


void mdxLoadBackdrop(const char* filename);

void mdxDrawBackdrop();
void mdxFreeBackdrop();
void ScreenShot();

#ifdef __cplusplus
}
#endif

#endif

