/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: overlays.h
	Programmer	: Andy Eder
	Date		: 20/04/99 10:08:52

----------------------------------------------------------------------------------------------- */

#ifndef OVERLAYS_H_INCLUDED
#define OVERLAYS_H_INCLUDED


#include "types2d.h"

//----- [ DEFINES / FLAGS ] --------------------------------------------------------------------//

// overlay flags
#define OVERLAY_IA				(1 << 0)

// sprite animation flags
#define ANIMATION_FORWARDS		(1 << 0)
#define ANIMATION_BACKWARDS		(1 << 1)
#define ANIMATION_CYCLE			(1 << 2)
#define ANIMATION_PINGPONG		(1 << 3)
#define ANIMATION_RANDOMFRAMES	(1 << 4)
#define ANIMATION_PLAYONCE		(1 << 5)
#define ANIMATION_RANDOMSTART	(1 << 6)


//----- [ GLOBALS ] ----------------------------------------------------------------------------//


typedef struct TAGSPRITEOVERLAYLIST
{
	int				numEntries, alloc;
	SPRITEOVERLAY	*block;
	
} SPRITEOVERLAYLIST;

#ifdef __cplusplus
extern "C" {
#endif

extern SPRITEOVERLAYLIST spriteOverlayList;

//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

SPRITEOVERLAY *CreateAndAddSpriteOverlay(short x,short y,char *txtrName,short width,short height,unsigned char alpha,short flags );

void EnableSpriteOverlay(SPRITEOVERLAY *sOver);
void DisableSpriteOverlay(SPRITEOVERLAY *sOver);

void InitSpriteOverlayLinkedList();
void FreeSpriteOverlayLinkedList();
void SubSpriteOverlay(SPRITEOVERLAY *spr);


#ifdef __cplusplus
}
#endif

#endif
