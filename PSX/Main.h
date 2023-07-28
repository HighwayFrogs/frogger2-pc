#ifndef _MAIN_H_INCLUDE
#define _MAIN_H_INCLUDE

#include <isltex.h>
#include <islfont.h>
#include "fixed.h"
#include "sonylibs.h"
#include "types.h"

#define VIDEO_INIT_AND_MALLOC 1

enum
{
	GAME_OVERLAY,
	VIDEO_OVERLAY,
	LANG_OVERLAY,
};

void LoadCodeOverlay(int num);
//void initialiseCrypt();

extern int skipTextOverlaysSpecFX;
extern long turbo;

extern GsRVIEW2	camera;
extern ULONG	frame;
extern char baseDirectory[256];
extern psFont *font;
extern psFont *fontSmall;

extern long textEntry;	
extern char textString[255];

extern int drawLandscape;
extern long drawGame;


extern TextureAnimType* timerAnim;
extern int animFrame;

#define ASSERT(A)	if(!(A)) {utilPrintf("Debug ASSERT failed at line %d, in "__FILE__, __LINE__);CRASH;}
#define MALLOC0(S)	memoryAllocateZero(S, __FILE__, __LINE__)
#define MALLOC02(S,X)	memoryAllocateZero(S, X, __LINE__)

void *memoryAllocateZero(unsigned long size, char *file, int line);
void MainDrawFunction ( void );


void videoInit ( int otDepth, int maxPrims, int flags );

void SetCurrentDisplayPage ( int page1, int page2 );

void SendOTDisp ( void );

void ResetDrawingEnvironment ( void );

void MainReset ( void );

void DisplayErrorMessage ( char *message );

void DisplayOnScreenInfo ( void );

#endif
