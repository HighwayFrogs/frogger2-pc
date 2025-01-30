/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: textoverlays.h
	Programmer	: Andy Eder
	Date		: 16/04/99 09:31:49

----------------------------------------------------------------------------------------------- */

#ifndef TEXTOVERLAYS_H_INCLUDED
#define TEXTOVERLAYS_H_INCLUDED

#include "types2d.h"


//----- [ DEFINES / FLAGS ] --------------------------------------------------------------------//

#define TEXTOVERLAY_NORMAL			0
#define TEXTOVERLAY_WAVEALL			(1 << 0)
#define TEXTOVERLAY_WAVECHARS		(1 << 1)
#define TEXTOVERLAY_WAVEDECAY		(1 << 2)
#define TEXTOVERLAY_LIFE			(1 << 3)
#define TEXTOVERLAY_SHADOW			(1 << 4)
#define TEXTOVERLAY_PIXELS			(1 << 5)
#define TEXTOVERLAY_PAUSED			(1 << 6)
#define TEXTOVERLAY_LOADING			(1 << 7)


//----- [ GLOBALS ] ----------------------------------------------------------------------------//

typedef struct TAGTEXTOVERLAYLIST
{
	int			numEntries, alloc;
	TEXTOVERLAY	*block;

} TEXTOVERLAYLIST;

#ifdef __cplusplus
extern "C" {
#endif

extern TEXTOVERLAYLIST textOverlayList;


//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

TEXTOVERLAY *CreateAndAddTextOverlay(short x, short y, char *text, char centred, UBYTE alpha,
									 psFont *fontPtr, char flags/*, fixed waveAmplitude*/);

extern long grabToTexture;

void PrintTextOverlays();
void EnableTextOverlay(TEXTOVERLAY *tover);
void DisableTextOverlay(TEXTOVERLAY *tover);

void SubTextOverlay(TEXTOVERLAY*);	// doesn't deallocate - please don't use!

void InitTextOverlayLinkedList(long num);
void FreeTextOverlayLinkedList();
void PrintTextAsOverlay(TEXTOVERLAY *tOver);

long StringWrap(const char* str, long maxWidth, char* buffer, long bufferSize, char** array, long arraySize, psFont *font);

#ifdef PC_VERSION
extern int fontFitToWidth(psFont *font, int width, char *text, char *buffer);
#endif

//>>
// [ANDYE]
extern TEXTOVERLAY *pTxtOvrCDAMInfo;
extern char szCDAMInfo[32];
//>>

#ifdef __cplusplus
}
#endif

#endif
