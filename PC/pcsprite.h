/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: pcsprite.h
	Programmer	: Jim
	Date		: 16/03/00
	Purpose		: Sprite sorting and printing

----------------------------------------------------------------------------------------------- */

#ifndef PCSPRITE_H_INCLUDED
#define PCSPRITE_H_INCLUDED

#include "sprite.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int numSprites;

void PrintSpriteOverlays(long);
void PrintSprites();

// PC Sprite sorting stuff
void InitSpriteSortArray( );

extern long	SPRITECLIPLEFT		;
extern long	SPRITECLIPTOP		;
extern long	SPRITECLIPRIGHT		;
extern long	SPRITECLIPBOTTOM	;

extern TextureType *tileTexture[4];

void AddObjectsSpritesToSpriteList(MDX_OBJECT *obj,short flags);
void FreeTiledBackdrop();
void InitTiledBackdrop(char *filename);
void DrawTiledBackdrop();

#ifdef __cplusplus
}
#endif


#endif
