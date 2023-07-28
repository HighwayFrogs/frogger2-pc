#ifndef _BACKDROP_H_INCLUDE
#define _BACKDROP_H_INCLUDE

#include "Shell.h"
#include "SonyLibs.h"

typedef struct _BACKDROP
{
	char *data;
	RECT rect;
	char old_red;
	char init;

} BACKDROP;

extern BACKDROP backDrop;

void InitBackdrop ( char * const filename );

void DrawBackDrop ( int execute, int drawBothScreens );
void FreeBackdrop ( void );

#endif