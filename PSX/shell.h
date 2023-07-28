/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	shell.h:		Basic shell definitions

************************************************************************************/

#ifndef __SHELL_H__
#define __SHELL_H__

#include <stddef.h>
#include <libgte.h>
#include <libgpu.h>

// some extra type definitions
#define BYTE	char
#define UBYTE	unsigned char
#define uchar	unsigned char
#define SHORT	short
#define USHORT	unsigned short
#define LONG	long
#define ULONG	unsigned long
#define VOID	void

// this is our display page structure, containing our drawing environment, display environment,
// our order table and primitive buffer

typedef struct _displayPageType {
	DISPENV			dispenv;
	DRAWENV			drawenv;
	unsigned long	*ot;
	char			*primBuffer, *primPtr;
} displayPageType;



// our double-buffered display pages, and a pointer to the current one

extern displayPageType displayPage[2], *currentDisplayPage;

extern ULONG frame;

// some macros to make life easier for us

#define BEGINPRIM(p,t)		(p) = (t *)currentDisplayPage->primPtr;
#define ENDPRIM(p,d,t)		{addPrim(currentDisplayPage->ot+(d), (p)); currentDisplayPage->primPtr += sizeof(t);}
#define SETSEMIPRIM(p,m)	(p)->tpage |= (((m)-1)<<5);

enum {
	SEMITRANS_NONE,
	SEMITRANS_SEMI,
	SEMITRANS_ADD,
	SEMITRANS_SUB,
};


#endif

