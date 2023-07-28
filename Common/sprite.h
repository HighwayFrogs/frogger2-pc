/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: sprite.h
	Programmer	: Jim Hubbard
	Date		: 25/02/00

----------------------------------------------------------------------------------------------- */

#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

////////	Basic sprite stuff
#define SPRITE_TRANSLUCENT						(1 << 0)
#define SPRITE_DONE								(1 << 1)
#define SPRITE_FLAGS_COLOUR_BLEND				(1 << 2)
#define SPRITE_FLAGS_COLOUR_BLEND_AFTERLIGHT	(1 << 3)
#define SPRITE_FLAGS_ROTATE						(1 << 4)
#define SPRITE_ADDITIVE							(1 << 5)
#define SPRITE_SUBTRACTIVE						(1 << 6)
#define SPRITE_LOADING							(1 << 7)
#define SPRITE_CLIP1							(1 << 8)
#define SPRITE_CLIP2							(1 << 9)


typedef struct TAGSPRITE
{
	struct TAGSPRITE *next,*prev;

	TextureType *texture;
	SVECTOR pos;
	short scaleX;
	short scaleY;
	short flags;
	UBYTE r,g,b,a;
//	UBYTE red2,green2,blue2,alpha2;
	BYTE  offsetX;
	BYTE  offsetY;

	char draw;

	short angle, angleInc;
	short arrayIndex;

#ifdef PC_VERSION
	MDX_VECTOR sc;
#endif

}SPRITE;


typedef struct OBJECTSPRITE
{
	TextureType		*textureID;
	SPRITE		*sprite;		//actual sprite in sprite list
	short		x, y, z;
	short		sx;	//xsize - 32 is normal
	short		sy;	//ysize - 32 is normal
	short		ox;	//offset from centre
	short		oy;	//offset from centre
	short		flags;

}OBJECTSPRITE;


typedef struct
{
	// Statically allocated array of sprites
	SPRITE *array;
	// Number of sprites allocated
	short count;

	// Defines and data for the sprite allocation stack
	SPRITE **stack;
	short stackPtr;

	// Static head of list
	SPRITE head;

} SPRITELIST;


extern SPRITELIST sprList;


SPRITE *AllocateSprites( int number );
void DeallocateSprites( SPRITE *head, int number );

void InitSpriteList( );
void FreeSpriteList( );



////////	Fog related
enum
{
	FOG_OFF,
	FOG_TO_COLOUR,
};

typedef struct
{
	UBYTE r,g,b,mode;
	short min,max;
}FOG;

extern FOG fog;

#ifdef __cplusplus
}
#endif

#endif
