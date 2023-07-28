/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: types2d.h
	Programmer	: Andy Eder
	Date		: 20/04/99 09:13:49

----------------------------------------------------------------------------------------------- */

#ifndef TYPES2DGFX_H_INCLUDED
#define TYPES2DGFX_H_INCLUDED

#include <isltex.h>
#include <islfont.h>

#include "types.h"



//----- [ DEFINES ] ----------------------------------------------------------------------------//

#define MAX_SPR_FRAMES	32

//----- [ DATA STRUCTURES ] --------------------------------------------------------------------//


typedef struct TAGTEXTOVERLAY
{
	char		*text;
	char		flags;
	short		xPos,yPos;
	short		xPosTo, yPosTo;
	short		centred;
	short		tWidth;

	fixed		speed;
	fixed		scale;

	UBYTE		r,g,b,a,oa;

	UBYTE		draw;

	psFont		*font;
	short		used;

} TEXTOVERLAY;



typedef struct TAGSPRITEOVERLAY
{
	short		xPos, yPos;
	short		xPosTo, yPosTo;
	short		width;
	short		height;
	short		num;
	short		flags;
	fixed		speed;
	fixed		angle;
	
	UBYTE		r,g,b,a;
	UBYTE		draw;

	TextureType	*tex;

	short		used;
} SPRITEOVERLAY;


typedef struct TAGSPRITE3D
{
	struct		TAGSPRITE3D *next;

	TextureType		*texture;
	fixed		xPos;
	fixed		yPos;
	short		zPos;

	fixed		scaleX;
	fixed		scaleY;

	UBYTE		r,g,b,a;
	UBYTE		draw;

	Sprite		spr;
	Bitmap		*bmp;

} SPRITE3D;



#endif
