/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: sprite.c
	Programmer	: Jim Hubbard
	Date		: 25/02/00

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2

#include <isltex.h>
#include <islmem.h>
#include <islutil.h>
#include <stdio.h>

#include "layout.h"
#include "game.h"
#include "sprite.h"
#ifdef PSX_VERSION
#include "textures.h"
#include "specfx.h"
#include "psxsprite.h"
#endif
#include "maths.h"
#include "newpsx.h"
#include "Shell.h"
#include "Main.h"
#include "frogger.h"


#ifdef PC_VERSION
int maxSprites	= 512;
#else
int maxSprites	= 180;
#endif

/*	--------------------------------------------------------------------------------
	Function		: AllocateSprites
	Purpose			: Find a number of sprites and return a sublist
	Parameters		: number of sprites
	Returns			: pointer to first one
	Info			: 
*/
SPRITE *AllocateSprites( int number )
{
	SPRITE *s;
	// Return if allocation is impossible for any reason
	if( (number <= 0) || (sprList.stackPtr-number < 0) || (number >= maxSprites-sprList.count) ) return NULL;

	// Now we can go and allocate sprites with gay abandon
	while( number-- )
	{
		s = sprList.stack[sprList.stackPtr--];

		s->prev	= &sprList.head;
		s->next	= sprList.head.next;
		sprList.head.next->prev	= s;
		sprList.head.next = s;
		s->draw = 1;

		sprList.count++;
	}

	return sprList.stack[sprList.stackPtr+1];
}


/*	--------------------------------------------------------------------------------
	Function		: DeallocateSprites
	Purpose			: Remove sprites from list and flag as unused
	Parameters		: number of sprites
	Returns			: 
	Info			: 
*/
void DeallocateSprites( SPRITE *head, int number )
{
	SPRITE *s=head, *t;

	if( !s || !s->next || (number<=0) || (sprList.stackPtr+number >= maxSprites) ) return;

	while( number-- )
	{
		t = s->next;

		s->prev->next	= s->next;
		s->next->prev	= s->prev;
		s->next = s->prev = NULL;
		s->draw = 0;

		sprList.count--;
		sprList.stack[++sprList.stackPtr] = s;

		s = t;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: FreeSpriteList
	Purpose			: Deallocate all sprites and set list to empty
	Parameters		: 
	Returns			: 
	Info			: 
*/
void FreeSpriteList( )
{
	int i;

	if( sprList.array )
	{
		// Remove all sprites in array from sprite list so they don't get removed after deallocation
		for( i=0; i<maxSprites; i++ )
		{
			if( !sprList.array[i].next ) continue;

			sprList.array[i].prev->next	= sprList.array[i].next;
			sprList.array[i].next->prev	= sprList.array[i].prev;
			sprList.array[i].next = NULL;
			sprList.array[i].draw = 0;
		}

		FREE( sprList.array );
		sprList.array = NULL;
	}

	if( sprList.stack )
	{
		FREE( sprList.stack );
		sprList.stack = NULL;
	}

	sprList.count = 0;
	sprList.stackPtr = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: InitSpriteList
	Purpose			: Set up static array and some bits of data.
	Parameters		: 
	Returns			: 
	Info			: 
*/
void InitSpriteList( )
{
	int i;

#ifdef PSX_VERSION
	if ( ( player[0].worldNum == WORLDID_HALLOWEEN ) && ( player[0].levelNum == LEVELID_HALLOWEEN2 ) )
		maxSprites = 256;
	else
		maxSprites = 180;
	// ENDELSEIF
#endif

	sprList.head.next = sprList.head.prev = &sprList.head;

	// Allocate a big bunch of sprites
	sprList.array = (SPRITE *)MALLOC0( sizeof(SPRITE)*maxSprites );
	sprList.stack = (SPRITE **)MALLOC0( sizeof(SPRITE*)*maxSprites );

	// Initially, all sprites are available
	for( i=0; i<maxSprites; i++ )
		sprList.stack[i] = &sprList.array[i];

	sprList.count = 0;
	sprList.stackPtr = i-1;
}






#ifdef PSX_VERSION

#define MAX_SPRITEWIDTH 256

int Print3DSprite( SPRITE *spr )
{
	POLY_FT4 *pp;
	LONG width, height, spritez;
	SVECTOR tempVect;

	tempVect.vx = -spr->pos.vx;
	tempVect.vy = -spr->pos.vy;
	tempVect.vz = spr->pos.vz;

	pp = (POLY_FT4 *)(currentDisplayPage->primPtr);
 	setPolyFT4(pp);
 	setRGB0(pp, spr->r>>1, spr->g>>1, spr->b>>1 );

// ------------- scary scaling-and-transform-in-one-go code from the Action Man people...
	width = spr->texture->w;
	gte_SetLDDQB(0);		// clear offset control reg (C2_DQB)

	gte_ldv0(&tempVect);
	gte_SetLDDQA(width);	// shove sprite width into control reg (C2_DQA)
	gte_rtps();				// do the rtps
	gte_stsxy(&pp->x0);		// get screen x and y
	gte_stsz(&spritez);		// get screen z
// ------------- end of scaling-and-transform

	// Sprite depth check
	if( spritez <= 0 || spritez >= fog.max )
		return 0;

	width = (spr->scaleX*SCALEX) / spritez;
	if(width < 2 || width > MAX_SPRITEWIDTH ) // To stop very small (flickery) and very big (slow) sprites
		return 0;	// better max-distance check // JIM: Not really. A good thing to do but not a substitute for an actual distance check.

 	pp->x1 = pp->x3 = pp->x0 + width;
 	pp->x0 = pp->x2 = pp->x0 - width;
   	if (pp->x1<=-256) return 0;
   	if (pp->x0>=256) return 0;

	// Scaled height of sprite. This is different from width - probably due to screen res.
	// NOTE: Random multiply that looks more or less OK :)
	height = (spr->scaleY*SCALEY) / spritez;

	pp->y2=pp->y3=pp->y0+height;
	pp->y1=pp->y0=pp->y0-height;
   	if (pp->y2<= -128) return 0;
   	if (pp->y0>= 128) return 0;

	*(USHORT*)&pp->u0=			*(USHORT*)&((TextureType*)spr->texture)->u0;	// u0,v0, and clut info
	*(USHORT*)&pp->u1=			*(USHORT*)&((TextureType*)spr->texture)->u1;	// u1,v1, and tpage info
	*(USHORT*)&pp->u2=			*(USHORT*)&((TextureType*)spr->texture)->u2;	// plus pad1
	*(USHORT*)&pp->u3=			*(USHORT*)&((TextureType*)spr->texture)->u3;	// plus pad2
	*(USHORT*)&pp->clut=		*(USHORT*)&((TextureType*)spr->texture)->clut;	// plus pad2
	*(USHORT*)&pp->tpage=		*(USHORT*)&((TextureType*)spr->texture)->tpage;	// plus pad2

	//Only do additive if the 'sprite' fades or is translucent
	if( spr->flags & SPRITE_TRANSLUCENT )
	{
	  	pp->r0 = ((spr->a*(short)pp->r0)>>8);
	  	pp->g0 = ((spr->a*(short)pp->g0)>>8);
	  	pp->b0 = ((spr->a*(short)pp->b0)>>8);

		// Set semi
		setSemiTrans(pp, 1);

		// Make additive
		if( spr->flags & SPRITE_ADDITIVE )
			pp->tpage |= 32;
		else
			pp->tpage |= 64;
	}

 	addPrim(currentDisplayPage->ot+(spritez>>4), pp);
 	currentDisplayPage->primPtr += sizeof(POLY_FT4);
	return 1;
}

int Print3DSpriteRotating( SPRITE *spr )
{
	POLY_FT4 *pp;
	LONG width, height, spritez;
	SVECTOR tempVect;

	fixed cosine,sine,newX,newY;
	int atbdx, atbdy;

	cosine = rcos(spr->angle);
	sine = rsin(spr->angle);

	tempVect.vx = -spr->pos.vx;
	tempVect.vy = -spr->pos.vy;
	tempVect.vz = spr->pos.vz;

	pp = (POLY_FT4 *)(currentDisplayPage->primPtr);
 	setPolyFT4(pp);
 	setRGB0(pp, spr->r>>1, spr->g>>1, spr->b>>1 );

// ------------- scary scaling-and-transform-in-one-go code from the Action Man people...
	width = spr->texture->w;
	gte_SetLDDQB(0);		// clear offset control reg (C2_DQB)

	gte_ldv0(&tempVect);
	gte_SetLDDQA(width);	// shove sprite width into control reg (C2_DQA)
	gte_rtps();				// do the rtps
	gte_stsxy(&pp->x0);		// get screen x and y
	gte_stsz(&spritez);		// get screen z
// ------------- end of scaling-and-transform

	// Sprite depth check
	if( spritez <= 0 || spritez >= fog.max ) 
		return 0;

	width = (spr->scaleX*SCALEX) / spritez;
	if( width < 2 || width > MAX_SPRITEWIDTH )
		return 0;	// better max-distance check // JIM: Not really. A good thing to do but not a substitute for an actual distance check.

	// Scaled height of sprite. This is different from width - probably due to screen res.
	// NOTE: Random multiply that looks more or less OK :)
	height = (spr->scaleY*SCALEX) / spritez;

	atbdx = pp->x0 - 2048;
	atbdy = pp->y0 - ((120+PALMODE*8)*(17-PALMODE));

	pp->x0 = -width;
	pp->x1 = width;
	pp->x2 = -width;
	pp->x3 = width;

	pp->y0 = -height;
	pp->y1 = -height;
	pp->y2 = height;
	pp->y3 = height;

	newX = FMul(pp->x0,cosine) + FMul(pp->y0,sine);
	newY = FMul(pp->y0,cosine) - FMul(pp->x0,sine);
	pp->x0 = newX + atbdx;
	pp->y0 = ((newY - height)>>1) + atbdy;

	newX = FMul(pp->x1,cosine) + FMul(pp->y1,sine);
	newY = FMul(pp->y1,cosine) - FMul(pp->x1,sine);
	pp->x1 = newX + atbdx;
	pp->y1 = ((newY - height)>>1) + atbdy;

	newX = FMul(pp->x2,cosine) + FMul(pp->y2,sine);
	newY = FMul(pp->y2,cosine) - FMul(pp->x2,sine);
	pp->x2 = newX + atbdx;
	pp->y2 = ((newY - height)>>1) + atbdy;

	newX = FMul(pp->x3,cosine) + FMul(pp->y3,sine);
	newY = FMul(pp->y3,cosine) - FMul(pp->x3,sine);
	pp->x3 = newX + atbdx;
	pp->y3 = ((newY - height)>>1) + atbdy;

	*(USHORT*)&pp->u0=			*(USHORT*)&((TextureType*)spr->texture)->u0;	// u0,v0, and clut info
	*(USHORT*)&pp->u1=			*(USHORT*)&((TextureType*)spr->texture)->u1;	// u1,v1, and tpage info
	*(USHORT*)&pp->u2=			*(USHORT*)&((TextureType*)spr->texture)->u2;	// plus pad1
	*(USHORT*)&pp->u3=			*(USHORT*)&((TextureType*)spr->texture)->u3;	// plus pad2
	*(USHORT*)&pp->clut=		*(USHORT*)&((TextureType*)spr->texture)->clut;	// plus pad2
	*(USHORT*)&pp->tpage=		*(USHORT*)&((TextureType*)spr->texture)->tpage;	// plus pad2

	//Only do additive if the 'sprite' fades or is translucent
	if( spr->flags & SPRITE_TRANSLUCENT )
	{
	  	pp->r0 = ((spr->a*(short)pp->r0)>>8);
	  	pp->g0 = ((spr->a*(short)pp->g0)>>8);
	  	pp->b0 = ((spr->a*(short)pp->b0)>>8);

		// Set semi
		setSemiTrans(pp, 1);

		// Make additive
		if( spr->flags & SPRITE_ADDITIVE )
			pp->tpage |= 32;
		else
		{
			pp->tpage |= 64;
			pp->r0>>=1;
			pp->g0>>=1;
			pp->b0>>=1;
		}
	}

 	addPrim(currentDisplayPage->ot+(spritez>>4), pp);
 	currentDisplayPage->primPtr += sizeof(POLY_FT4);

	return 1;
}


int Print3D3DSprite ( SPECFX *fx, SVECTOR *vect ) 
{
	POLY_FT4 *pp;
	long spritez;
	short check = PALMODE?128:120;
	SVECTOR tempVect[4];

	tempVect[0].vx = -vect[0].vx;
	tempVect[0].vy = -vect[0].vy;
	tempVect[0].vz = vect[0].vz;

	tempVect[1].vx = -vect[1].vx;
	tempVect[1].vy = -vect[1].vy;
	tempVect[1].vz = vect[1].vz;

	tempVect[3].vx = -vect[2].vx;
	tempVect[3].vy = -vect[2].vy;
	tempVect[3].vz = vect[2].vz;

	tempVect[2].vx = -vect[3].vx;
	tempVect[2].vy = -vect[3].vy;
	tempVect[2].vz = vect[3].vz;

 	pp = (POLY_FT4 *)(currentDisplayPage->primPtr);

 	setPolyFT4(pp);

 	setRGB0(pp, fx->r>>1, fx->g>>1, fx->b>>1 );//(colour&255),((colour>>8)&255),((colour>>16)&255));

	gte_SetLDDQB(0);			// clear offset control reg (C2_DQB)

 	gte_ldv0(&tempVect[0]);		//vert);
 	gte_rtps();					// do the rtps
 	gte_stsxy(&pp->x0);			// get screen x and y
 
 	gte_ldv0(&tempVect[1]);		//vert);
 	gte_rtps();					// do the rtps
 	gte_stsxy(&pp->x1);			// get screen x and y
 
 	gte_ldv0(&tempVect[2]);		//vert);
 	gte_rtps();					// do the rtps
 	gte_stsxy(&pp->x2);			// get screen x and y

	gte_ldv0(&tempVect[3]);		//vert);  								
	gte_rtps();					// do the rtps							
	gte_stsxy(&pp->x3);			// get screen x and y	

	gte_stsz(&spritez);			// get screen z

	spritez += fx->zDepthOff<<2;		//mm allow for z depth to be altered // JIM: Now using screen z so scale offset too

	if( spritez <= 0 || spritez >= fog.max ) 
		return 0;

	if( !(pp->x0 > -256 || pp->x0 < 256 ||
		pp->x2 > -256 || pp->x2 < 256 ||
		pp->y0 > -check || pp->y0 < check ||
		pp->y2 > -check || pp->y2 < check) )
		return;

//	if (pp->x0<-256) return 0;
//   	if (pp->x1>256) return 0;
 
//   	if (pp->y1< -128) return 0;
//   	if (pp->y2> 128) return 0;

	*(USHORT*)&pp->u0=			*(USHORT*)&((TextureType*)fx->tex)->u0;	// u0,v0, and clut info
	*(USHORT*)&pp->u1=			*(USHORT*)&((TextureType*)fx->tex)->u1;	// u1,v1, and tpage info
	*(USHORT*)&pp->u2=			*(USHORT*)&((TextureType*)fx->tex)->u2;	// plus pad1
	*(USHORT*)&pp->u3=			*(USHORT*)&((TextureType*)fx->tex)->u3;	// plus pad2
	*(USHORT*)&pp->clut=		*(USHORT*)&((TextureType*)fx->tex)->clut;	// plus pad2
	*(USHORT*)&pp->tpage=		*(USHORT*)&((TextureType*)fx->tex)->tpage;	// plus pad2

	if(fx->flags & SPRITE_TRANSLUCENT )//Only do additive if the 'sprite' fades or is translucent
	{
	  	pp->r0 = ((fx->a*(short)pp->r0)>>8);
	  	pp->g0 = ((fx->a*(short)pp->g0)>>8);
	  	pp->b0 = ((fx->a*(short)pp->b0)>>8);

		pp->code |= 2;
		pp->tpage |= 32;
	}

	if(fx->flags & SPRITE_SUBTRACTIVE )//Subtractive for shadows
	{
		pp->r0 = ((fx->a*(short)pp->r0)>>8);
		pp->g0 = ((fx->a*(short)pp->g0)>>8);
		pp->b0 = ((fx->a*(short)pp->b0)>>8);

		pp->code |= 2;
		pp->tpage |= 64;
	}
	
 	addPrim(currentDisplayPage->ot+(spritez>>4), pp);
 	currentDisplayPage->primPtr += sizeof(POLY_FT4);

	return 1;
}


void PrintSprites ( void )
{
	SPRITE *cur;
	
	for( cur = sprList.head.next; cur != &sprList.head; cur = cur->next )
	{
		if ( !cur )
			break;

		if( !cur->draw )
			continue;

		if( !(cur->flags & SPRITE_FLAGS_ROTATE) )
		{
			Print3DSprite( cur );
		}
		else
		{
			if((gameState.mode != PAUSE_MODE) && (gameState.mode != GAMEOVER_MODE))
			{
				cur->angle += (cur->angleInc * gameSpeed)>>12;
				if( cur->angle >= 4096 ) 
					cur->angle -= 4096;
				else if( cur->angle < 0 ) 
					cur->angle += 4096;
			}

			Print3DSpriteRotating( cur );
		}
 	}
}

#endif