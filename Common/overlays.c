/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: overlays.c
	Programmer	: Andy Eder
	Date		: 20/04/99 10:24:40

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI

#include <islutil.h>
#include "ultra64.h"
#include "overlays.h"

#include "maths.h"
#include "newpsx.h"
#include <islmem.h>
#include "Main.h"

#ifdef PSX_VERSION
#include "textures.h"
#else
#include <pcmisc.h>
#endif

#define SPRITELIST_TEMP_IDONTGIVEASHIT_NUMBER	120



/*	--------------------------------------------------------------------------------
	Function		: CreateAndAddSpriteOverlay
	Purpose			: creates and adds a sprite overlay to the list
	Parameters		: short,short,char*,short,short,char,char,char,char,short
	Returns			: SPRITEOVERLAY *
	Info			: 
*/
SPRITEOVERLAY *CreateAndAddSpriteOverlay(short x,short y,char *txtrName,short width,short height,unsigned char alpha,short flags )
{
	int j;
	SPRITEOVERLAY *newItem;

	//newItem = (SPRITEOVERLAY *)MALLOC0(sizeof(SPRITEOVERLAY));
	if (spriteOverlayList.numEntries == spriteOverlayList.alloc)
	{
		utilPrintf("CreateAndAddSpriteOverlay(): Couldn't allocate\n");
		return NULL;
	}

	for(j = 0;j < spriteOverlayList.alloc;j++)
		if(spriteOverlayList.block[j].used == 0)
			break;

	newItem = &spriteOverlayList.block[j];
	spriteOverlayList.numEntries++;

	newItem->xPos		= x;
	newItem->yPos		= y;
	newItem->xPosTo		= x;
	newItem->yPosTo		= y;
	newItem->width		= width;
	newItem->height		= height;
	newItem->draw		= 1;
	newItem->flags		= flags;
	newItem->num		= 0;

	newItem->r			= 255;
	newItem->g			= 255;
	newItem->b			= 255;
	newItem->a			= alpha;

	newItem->speed		= 50;
	newItem->angle		= 0;

	newItem->used		= 1;

	if( txtrName )
		if( !(newItem->tex = FindTexture(txtrName)) )
			utilPrintf("Could Not Find Texture : %s\n", txtrName );

	return newItem;
}


void SubSpriteOverlay(SPRITEOVERLAY *spr)
{
	if((spr) && (spr->used))
	{
		spr->used = 0;
		spr->draw = 0;
		spriteOverlayList.numEntries--;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: InitSpriteOverlayLinkedList
	Purpose			: initialises the sprite overlay linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitSpriteOverlayLinkedList()
{
	int entries = SPRITELIST_TEMP_IDONTGIVEASHIT_NUMBER;

	spriteOverlayList.block = MALLOC0(sizeof(SPRITEOVERLAY)*entries);
	spriteOverlayList.alloc = entries;
	spriteOverlayList.numEntries = 0;
}

/*	--------------------------------------------------------------------------------
	Function		: FreeSpriteOverlayLinkedList
	Purpose			: frees the sprite overlay linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeSpriteOverlayLinkedList()
{
	FREE(spriteOverlayList.block);
	InitSpriteOverlayLinkedList();
}

/*	--------------------------------------------------------------------------------
	Function		: EnableSpriteOverlay
	Purpose			: enables a sprite overlay for viewing
	Parameters		: SPRITEOVERLAY *
	Returns			: void
	Info			: 
*/
void EnableSpriteOverlay(SPRITEOVERLAY *sOver)
{
	if( sOver ) sOver->draw = 1;
}


/*	--------------------------------------------------------------------------------
	Function		: DisableSpriteOverlay
	Purpose			: disables a sprite overlay
	Parameters		: SPRITEOVERLAY *
	Returns			: void
	Info			: 
*/
void DisableSpriteOverlay(SPRITEOVERLAY *sOver)
{
	if( sOver ) sOver->draw = 0;
}

