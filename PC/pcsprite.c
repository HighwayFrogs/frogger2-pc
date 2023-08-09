/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: pcsprite.c
	Programmer	: Jim
	Date		: 16/03/00
	Purpose		: Sprite sorting and drawing functions

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <mdx.h>

#include <islmem.h>

#include "frogger.h"
#include "sprite.h"
#include "types2d.h"
#include "overlays.h"
#include "Main.h"
#include "pcmisc.h"
#include "pcgfx.h"
#include "pcsprite.h"
#include "layout.h"

#define FIXED_TO_RADS				0.001525879

#define MAX_ARRAY_SPRITES			768
#define SPRITE_ZSORT_DRAWDISTANCE	450

#define SPRITE_DEPTH_OFF -50

int numSortArraySprites = 0;
SPRITE *spriteSortArray[MAX_ARRAY_SPRITES];

int SpriteZCompare(const void *arg1,const void *arg2);
void ZSortSpriteList();

// PC Sprite drawing stuff
void PrintSprite(SPRITE *sprite);

void DrawSpriteOverlay( float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour );


//----- [ PRINTING STUFF ] -----------------------------------------------------------------------

long	SPRITECLIPLEFT		=10;
long	SPRITECLIPTOP		=10;
long	SPRITECLIPRIGHT		=630;
long	SPRITECLIPBOTTOM	=470;

float OVERLAY_X;
float OVERLAY_Y;
float spriteMul;

int numSprites;

void AddObjectsSpritesToSpriteList(MDX_OBJECT *obj,short flags)
{
	SPRITE *sprite;
	int i;

	if(obj->numSprites > 0)
	{
		for(i=0; i<obj->numSprites; i++)
		{
			sprite = AllocateSprites( 1 );

			sprite->texture = (TextureType *)obj->sprites[i].textureID;
			SetVectorSV(&sprite->pos,&zero);

			if((obj->sprites[i].flags & SPRITE_DONE) == 0)
				obj->sprites[i].flags |= SPRITE_DONE;

			sprite->r = sprite->g = sprite->b = 255;
			sprite->a = 128;
			if(obj->xlu < 255)
				sprite->flags = SPRITE_TRANSLUCENT;
			else
				sprite->flags = 0;

			sprite->flags |= flags;

			sprite->offsetX = -32 / 2;
			sprite->offsetY = -32 / 2;

			//AddSprite(sprite,NULL);
			obj->sprites[i].sprite = (MDX_SPRITE *)sprite;
		}
	}

	if(obj->children)
		AddObjectsSpritesToSpriteList(obj->children,flags);

	if(obj->next)
		AddObjectsSpritesToSpriteList(obj->next,flags);
}
/*	--------------------------------------------------------------------------------
    Function		: PrintSprites
	Parameters		: 
	Returns			: 
	Purpose			: Sort and draw all sprites
*/
void PrintSprites()
{
	int i;
	SPRITE *cur;
	MDX_VECTOR tVect;
	
	// transform sprites to screen coords ready for sorting
	for(cur = sprList.head.next; cur != &sprList.head && numSortArraySprites < MAX_ARRAY_SPRITES; cur = cur->next)
	{
		SetVectorRS( &tVect, &cur->pos );
		ScaleVector( &tVect, 0.1 );

		XfmPoint(&cur->sc,&tVect,NULL);
	}
	ZSortSpriteList();

	softDepthOff = SPRITE_DEPTH_OFF;

	// draw from the newly sorted static array
	i = numSortArraySprites;
	while(i--) if( spriteSortArray[i]->draw ) 
		PrintSprite(spriteSortArray[i]);

	softDepthOff = 0;
}


/*	--------------------------------------------------------------------------------
    Function		: PrintSpriteOverlays
	Parameters		: 
	Returns			: 
	Purpose			: Draw all sprite overlays
*/
void PrintSpriteOverlays(long num)
{
	SPRITEOVERLAY *cur;
	int n;
	MDX_TEXENTRY *tEntry;
	D3DCOLOR col;

	if( !drawOverlays || !spriteOverlayList.numEntries ) return;

	n = spriteOverlayList.alloc;
	cur = spriteOverlayList.block + (n-1);
	
	for(;n; n--,cur--)
	{
		if (cur->num!=num)
			continue;

		// update the sprite animation if an animated sprite overlay
		if(cur->used && cur->draw && cur->a)
		{
			// Go to destination, if specified
			fixed spd = FMul(gameSpeed, cur->speed)>>12;

			if( Fabs(cur->xPosTo-cur->xPos) )
			{
				cur->xPos += (cur->xPosTo > cur->xPos)?spd:-spd;
				
				if( Fabs(cur->xPosTo-cur->xPos) < Fabs(spd) )
				{
					cur->speed = 0;
					cur->xPos = cur->xPosTo;
				}
			}
			else if( Fabs(cur->yPosTo-cur->yPos) )
			{
				cur->yPos += (cur->yPosTo > cur->yPos)?spd:-spd;

				if( Fabs(cur->yPosTo-cur->yPos) < Fabs(spd) )
				{
					cur->speed = 0;
					cur->yPos = cur->yPosTo;
				}
			}

			tEntry = ((MDX_TEXENTRY *)cur->tex);
//			if( (tEntry = ((MDX_TEXENTRY *)cur->tex)) )
			{
				col = RGBA_MAKE(cur->r,cur->g,cur->b,cur->a);

				if (cur->flags & SPRITE_ADDITIVE)
					D3DSetupRenderstates(xluAddRS);
				else if (cur->flags & SPRITE_SUBTRACTIVE)
				{
					D3DSetupRenderstates(xluSubRS);
					col = RGBA_MAKE((cur->r*cur->a)>>8,(cur->g*cur->a)>>8,(cur->b*cur->a)>>8,1);
				}
				else
					D3DSetupRenderstates(xluSemiRS);

				if( !cur->angle )
				{
					DrawSpriteOverlay( cur->xPos*OVERLAY_X, cur->yPos*OVERLAY_Y, 0, cur->width*OVERLAY_X, cur->height*OVERLAY_Y, 
						0, 0, 1, 1, tEntry, col);
				}
				else
				{
					DrawSpriteOverlayRotating( cur->xPos*OVERLAY_X, cur->yPos*OVERLAY_Y, 0, cur->width*OVERLAY_X, cur->height*OVERLAY_Y, 0, 0, 1, 1, 
						(float)cur->angle*-FIXED_TO_RADS, tEntry, col);
				}

				D3DSetupRenderstates(xluSemiRS);
			}

			numSprites++;
		}
	}
}


/*	--------------------------------------------------------------------------------
    Function		: DrawSpriteOverlay
	Parameters		: 
	Returns			: 
	Purpose			: Draw an alpha-ed, non-rotating sprite overlay
*/
void DrawSpriteOverlay( float x, float y, float z, float xs, float ys, float u1, float v1, float u2, float v2, MDX_TEXENTRY *tex, DWORD colour )
{
	D3DTLVERTEX v[4];
	float x2 = (x+xs), y2 = (y+ys);

	if (x < SPRITECLIPLEFT)
	{
		if (x2 < SPRITECLIPLEFT) return;
		u1 += (u2-u1) * (SPRITECLIPLEFT-x)/xs;	// clip u
		xs += x-SPRITECLIPLEFT; x = SPRITECLIPLEFT;
	}
	if (x2 > SPRITECLIPRIGHT)
	{
		if (x > SPRITECLIPRIGHT) return;
		u2 += (u2-u1) * (SPRITECLIPRIGHT-x2)/xs;	// clip u
		xs -= (x-SPRITECLIPRIGHT);
		x2 = SPRITECLIPRIGHT;
	}

	if (y < SPRITECLIPTOP)
	{
		if (y2 < SPRITECLIPTOP) return;
		v1 += (v2-v1) * (SPRITECLIPTOP-y)/ys;	// clip v
		ys += y-SPRITECLIPTOP; y = SPRITECLIPTOP;
	}
	if (y2 > SPRITECLIPBOTTOM)
	{
		if (y > SPRITECLIPBOTTOM) return;
		v2 += (v2-v1) * (SPRITECLIPBOTTOM-y2)/ys;	// clip v
		ys -= (y-SPRITECLIPBOTTOM);
		y2 = SPRITECLIPBOTTOM;
	}

	v[0].sx = x; v[0].sy = y; v[0].sz = z; v[0].rhw = 0;
	v[0].color = colour; v[0].specular = D3DRGBA(0,0,0,1);
	v[0].tu = u1; v[0].tv = v1;	v[0].rhw = 1;

	v[1].sx = x2; v[1].sy = y; v[1].sz = z; v[1].rhw = 0;
	v[1].color = v[0].color; v[1].specular = D3DRGBA(0,0,0,1);
	v[1].tu = u2; v[1].tv = v1;	v[1].rhw = 1;
	
	v[2].sx = x2; v[2].sy = y2; v[2].sz = z; v[2].rhw = 0;
	v[2].color = v[0].color; v[2].specular = D3DRGBA(0,0,0,1);
	v[2].tu = u2; v[2].tv = v2;	v[2].rhw = 1;

	v[3].sx = x; v[3].sy = y2; v[3].sz = z; v[3].rhw = 0;
	v[3].color = v[0].color; v[3].specular = D3DRGBA(0,0,0,1);
	v[3].tu = u1; v[3].tv = v2;	v[3].rhw = 1;

	SetTexture(tex);

	DrawPoly( D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX,
		v, 4, spIndices, 6,
		D3DDP_WAIT);

	SetTexture(NULL);
}


/*	--------------------------------------------------------------------------------
    Function		: PrintSprite
	Parameters		: Sprite
	Returns			: 
	Purpose			: Choose which sprite drawing function to use and set render states
*/
void PrintSprite(SPRITE *sprite)
{
	MDX_VECTOR sc, s = {1,1,0};
	float distx,disty;
	MDX_TEXENTRY *tEntry;

	if((!sprite->texture) || (sprite->scaleX == 0) || (sprite->scaleY == 0))
		return;

	SetVector( &sc, &sprite->sc );
	
	tEntry = ((MDX_TEXENTRY *)sprite->texture);

	if( sc.vz )
	{
		distx = disty = (FOV)/(sc.vz+DIST);
		distx *= (sprite->scaleX/(64.0));
		disty *= (sprite->scaleY/(64.0));
		distx *= spriteMul;
		disty *= spriteMul;
		numSprites++;

		if (sprite->flags & SPRITE_ADDITIVE)
		{
			D3DSetupRenderstates(xluAddRS);
			SwapFrame(3);
		}
		else
			SwapFrame(0);

		if(sprite->flags & SPRITE_FLAGS_ROTATE)
		{
			// rotate the little blighters
			sprite->angle += (sprite->angleInc * gameSpeed)>>12;
			if( sprite->angle >= 4096 ) 
				sprite->angle -= 4096;
			else if( sprite->angle < 0 ) 
				sprite->angle += 4096;

			DrawAlphaSpriteRotating( &sc,(float)sprite->angle*FIXED_TO_RADS,sc.vx+sprite->offsetX*distx,sc.vy+sprite->offsetY*disty,sc.vz*0.00025,32*distx,32*disty,
				0,0,1,1,tEntry,D3DRGBA(sprite->r/255.0,sprite->g/255.0,sprite->b/255.0,sprite->a/255.0) );
		}
		else
		{
			DrawAlphaSprite(sc.vx+sprite->offsetX*distx,sc.vy+sprite->offsetY*disty,sc.vz*0.00025,32*distx,32*disty,
				0,0,1,1,tEntry,D3DRGBA(sprite->r/255.0,sprite->g/255.0,sprite->b/255.0,sprite->a/255.0) );
		}

		D3DSetupRenderstates(xluSemiRS);
	}
}


//----- [ SORTING STUFF ] -----------------------------------------------------------------------

/*	--------------------------------------------------------------------------------
	Function		: InitSpriteSortArray
	Purpose			: initialises the sprite sort array
	Parameters		: int
	Returns			: void
	Info			: 
*/
void InitSpriteSortArray( )
{
	int i=MAX_ARRAY_SPRITES;
	
	while( i-- ) spriteSortArray[i] = NULL;
	numSortArraySprites = 0;

	spriteMul = (float)rXRes/(float)640.0;
}


/*	--------------------------------------------------------------------------------
	Function		: SpriteZCompare
	Purpose			: function to compare transformed sprite z-values for sorting
	Parameters		: const void *,const void *
	Returns			: int
	Info			: -1 if less than, 0 if equal to, 1 if greater than
*/
int SpriteZCompare(const void *arg1,const void *arg2)
{
	SPRITE *s1 = *(SPRITE **)arg1;
	SPRITE *s2 = *(SPRITE **)arg2;

	if(s1->sc.vz < s2->sc.vz)
		return -1;
	
	if(s1->sc.vz == s2->sc.vz)
		return 0;

	return 1;
}


/*	--------------------------------------------------------------------------------
	Function		: ZSortSpriteList
	Purpose			: sorts the sprites based on z-distance
	Parameters		: 
	Returns			: void
	Info			: list to sort is specified in srcList
*/
void ZSortSpriteList()
{
	SPRITE *cur;
	MDX_VECTOR frogXfm;
		
	if( !sprList.count )
		return;

	XfmPoint(&frogXfm,(MDX_VECTOR *)&frog[0]->actor->position,NULL);

	// traverse through sprite list and create the quick sort array
	numSortArraySprites = 0;
	for(cur = sprList.head.next; cur != &sprList.head && numSortArraySprites < MAX_ARRAY_SPRITES; cur = cur->next)
		if((cur->sc.vz - frogXfm.vz) < farClip )
			spriteSortArray[numSortArraySprites++] = cur;

	if( sprList.count > 1 )
		qsort( (void *)spriteSortArray, numSortArraySprites, sizeof(SPRITE*), SpriteZCompare );
}

TextureType *tileTexture[4] = {NULL,NULL,NULL,NULL};

void InitTiledBackdrop(char *filename)
{
	char tempStr[32];
	int i;

	for(i = 0;i < 4;i++)
	{
		sprintf(tempStr,"%s0%d",filename,i+1);
		tileTexture[i] = FindTexture(tempStr);
	}
}

int xTile = 5;
int yTile = 4;
float xScrollSpeed = 0.001;
float yScrollSpeed = 0.0006;
void DrawTiledBackdrop()
{
	int x,y;
	RECT r;
	float xRes = OVERLAY_X*4096;
	float yRes = OVERLAY_Y*4096;
	static float xScroll = 0;
	static float yScroll = 0;

	for(x = -4;x < xTile + 4;x++)
	{
		for(y = -2;y < yTile + 2;y++)
		{
			r.left = (x*xRes)/xTile + xScroll;
			r.right = ((x+1)*xRes)/xTile + xScroll;
			r.top = (y*yRes)/yTile + yScroll;
			r.bottom = ((y+1)*yRes)/yTile + yScroll;

//			DrawTexturedRect(r,D3DRGBA(1,1,1,1),((MDX_TEXENTRY *)tileTexture)->surf,0,0,1,1);
			mdxPolyDrawTextureRect(r, D3DRGBA(1,1,1,1), (MDX_TEXENTRY *)tileTexture[(((x + 4) MOD 2) + ((y + 2) MOD 2) * 2 + ((x + 4)/2) * 2) MOD 4], 0,0, 1,1);
		}
	}

//	xScrollSpeed = sin((float)actFrameCount/60.0)*0.001;
//	yScrollSpeed = cos((float)actFrameCount/81.0)*0.0006;

	xScroll += (float)gameSpeed * xScrollSpeed;
	if(xScroll > (xRes*4)/(float)xTile)
		xScroll -= (xRes*4)/(float)xTile;
	else if(xScroll < 0)
		xScroll += (xRes*4)/(float)xTile;
	yScroll += (float)gameSpeed * yScrollSpeed;
	if(yScroll > (yRes*2)/(float)yTile)
		yScroll -= (yRes*2)/(float)yTile;
	else if(yScroll < 0)
		yScroll += (yRes*2)/(float)yTile;
}

void FreeTiledBackdrop()
{
	tileTexture[0] = NULL;
}
