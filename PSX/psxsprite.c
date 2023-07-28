#include <isltex.h>

#include <islpad.h>
#include <islutil.h>
#include "Shell.h"
#include "overlays.h"

#include "types2d.h"
#include "psxsprite.h"
#include "sprite.h"
#include "layout.h"
#include "maths.h"
#include "textures.h"

RECT clipBox1 = {0,0,64,64};
RECT clipBox2 = {0,0,0,64};
RECT *currClipBox;

void DrawSprite ( SPRITEOVERLAY *spr )
{
	int			atbdx,atbdy, w,h;
	POLY_F4		*f4;
	POLY_FT4	*ft4;
	DR_MODE		*dm;
	TextureType	*tPtr;
	uchar alpha,r,g,b;
	int depth;

	depth = 4 - spr->num;

	atbdx = (spr->xPos>>3)-256;
	atbdy = (spr->yPos/(17-PALMODE))-120-PALMODE*8;

	tPtr = spr->tex;

	if((spr->a == 255) && (!(spr->flags & (SPRITE_ADDITIVE | SPRITE_SUBTRACTIVE))))
	{
		alpha = 0;
//		r = (spr->r * 128) >> 8;
//		g = (spr->g * 128) >> 8;
//		b = (spr->b * 128) >> 8;
		r = spr->r>>1;
		g = spr->g>>1;
		b = spr->b>>1;
	}
	else
	{
		alpha = 2;
		r = (spr->r * spr->a) >> 9;
		g = (spr->g * spr->a) >> 9;
		b = (spr->b * spr->a) >> 9;
	}

	if(spr->tex == 0)
	{
		BEGINPRIM(f4,POLY_F4);
		setPolyF4(f4);

		f4->x0 = atbdx;
		f4->y0 = atbdy;
		f4->x1 = atbdx + (spr->width>>3);
		f4->y1 = atbdy;
		f4->x2 = atbdx;
		f4->y2 = atbdy + (spr->height/(17-PALMODE));
		f4->x3 = atbdx + (spr->width>>3);
		f4->y3 = atbdy + (spr->height/(17-PALMODE));
		f4->r0 = r<<1;
		f4->g0 = g<<1;
		f4->b0 = b<<1;
		if(alpha)
			setSemiTrans(f4,1);
		ENDPRIM(f4, depth, POLY_F4);

		if(alpha)
		{
			BEGINPRIM(dm, DR_MODE);
			if(spr->flags & SPRITE_SUBTRACTIVE)
				SetDrawMode(dm, 0,1, ((SEMITRANS_SUB-1)<<5),0);
			else
				SetDrawMode(dm, 0,1, ((SEMITRANS_ADD-1)<<5),0);
			ENDPRIM(dm, depth, DR_MODE);
		}
	}
	else
	{
		w = tPtr->w-1;
		h = tPtr->h-1;

		if (spr->width==4096)
		{	// NEW! 1:1 texels to pixels
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			ft4->x0 = atbdx;
			ft4->y0 = atbdy;
			ft4->x1 = atbdx+w;
			ft4->y1 = atbdy;
			ft4->x2 = atbdx;
			ft4->y2 = atbdy+h;
			ft4->x3 = atbdx+w;
			ft4->y3 = atbdy+h;
			ft4->r0 = r;//( spr->r * 128 ) >> 8;
			ft4->g0 = g;//( spr->g * 128 ) >> 8;
			ft4->b0 = b;//( spr->b * 128 ) >> 8;
			ft4->u0 = tPtr->u0;
			ft4->v0 = tPtr->v0;
			ft4->u1 = tPtr->u1;
			ft4->v1 = tPtr->v1;
			ft4->u2 = tPtr->u2;
			ft4->v2 = tPtr->v2;
			ft4->u3 = tPtr->u3;
			ft4->v3 = tPtr->v3;
			ft4->tpage = tPtr->tpage;
			if(alpha)
			{
				if(spr->flags & SPRITE_SUBTRACTIVE)
				{
					SETSEMIPRIM(ft4, SEMITRANS_SUB);
				}
				else
				{
					SETSEMIPRIM(ft4, SEMITRANS_ADD);
				}
			}
			ft4->clut = tPtr->clut;
			setSemiTrans(ft4, (alpha > 0) ? 1 : 0);
			ENDPRIM(ft4, depth, POLY_FT4);
		}
		else
		{	// Original random scaling method (slightly tidier)
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			ft4->x0 = atbdx;
			ft4->y0 = atbdy;
			ft4->x1 = atbdx + (spr->width>>3);
			ft4->y1 = atbdy;
			ft4->x2 = atbdx;
			ft4->y2 = atbdy + (spr->height/(17-PALMODE));
			ft4->x3 = atbdx + (spr->width>>3);
			ft4->y3 = atbdy + (spr->height/(17-PALMODE));
			ft4->r0 = r;//( spr->r * 128 ) >> 8;
			ft4->g0 = g;//( spr->g * 128 ) >> 8;
			ft4->b0 = b;//( spr->b * 128 ) >> 8;
			ft4->u0 = tPtr->u0;
			ft4->v0 = tPtr->v0;
			ft4->u1 = tPtr->u1;
			ft4->v1 = tPtr->v1;
			ft4->u2 = tPtr->u2;
			ft4->v2 = tPtr->v2;
			ft4->u3 = tPtr->u3;
			ft4->v3 = tPtr->v3;
			ft4->tpage = tPtr->tpage;
			if(alpha)
			{
				if(spr->flags & SPRITE_SUBTRACTIVE)
				{
					SETSEMIPRIM(ft4, SEMITRANS_SUB);
				}
				else
				{
					SETSEMIPRIM(ft4, SEMITRANS_ADD);
				}
			}
			ft4->clut = tPtr->clut;
			setSemiTrans(ft4, (alpha > 0) ? 1 : 0);
			ENDPRIM(ft4, depth, POLY_FT4);
		}
	}
}


void DrawSpriteClipped(SPRITEOVERLAY *spr)
{
	int			atbdx,atbdy, w,h;
	POLY_F4		*f4;
	POLY_FT4	*ft4;
	DR_MODE		*dm;
	TextureType	*tPtr;
	uchar alpha,r,g,b;
	int depth;

	if((currClipBox->x >= currClipBox->w) || (currClipBox->y >= currClipBox->h))
		return;

	depth = 4 - spr->num;

	atbdx = (spr->xPos>>3)-256 + currClipBox->x*2;
	atbdy = (spr->yPos/(17-PALMODE))-120-PALMODE*8 + currClipBox->y;

	tPtr = spr->tex;

	if((spr->a == 255) && (!(spr->flags & (SPRITE_ADDITIVE | SPRITE_SUBTRACTIVE))))
	{
		alpha = 0;
//		r = (spr->r * 128) >> 8;
//		g = (spr->g * 128) >> 8;
//		b = (spr->b * 128) >> 8;
		r = spr->r>>1;
		g = spr->g>>1;
		b = spr->b>>1;
	}
	else
	{
		alpha = 2;
		r = (spr->r * spr->a) >> 9;
		g = (spr->g * spr->a) >> 9;
		b = (spr->b * spr->a) >> 9;
	}


	w = (currClipBox->w - currClipBox->x)*2 - 2;
	h = currClipBox->h - currClipBox->y - 1;

//	w = tPtr->w-1;
//	h = tPtr->h-1;

	BEGINPRIM(ft4, POLY_FT4);
	setPolyFT4(ft4);
	ft4->x0 = atbdx;
	ft4->y0 = atbdy;
	ft4->x1 = atbdx + w;
	ft4->y1 = atbdy;
	ft4->x2 = atbdx;
	ft4->y2 = atbdy + h;
	ft4->x3 = atbdx + w;
	ft4->y3 = atbdy + h;
	ft4->r0 = r;//( spr->r * 128 ) >> 8;
	ft4->g0 = g;//( spr->g * 128 ) >> 8;
	ft4->b0 = b;//( spr->b * 128 ) >> 8;
	ft4->u0 = tPtr->u0 + currClipBox->x;
	ft4->v0 = tPtr->v0 + currClipBox->y;
	ft4->u1 = tPtr->u0 + currClipBox->w - 1;
	ft4->v1 = tPtr->v0 + currClipBox->y;
	ft4->u2 = tPtr->u0 + currClipBox->x;
	ft4->v2 = tPtr->v0 + currClipBox->h - 1;
	ft4->u3 = tPtr->u0 + currClipBox->w - 1;
	ft4->v3 = tPtr->v0 + currClipBox->h - 1;
	ft4->tpage = tPtr->tpage;
	if(alpha)
	{
		if(spr->flags & SPRITE_SUBTRACTIVE)
		{
			SETSEMIPRIM(ft4, SEMITRANS_SUB);
		}
		else
		{
			SETSEMIPRIM(ft4, SEMITRANS_ADD);
		}
	}
	ft4->clut = tPtr->clut;
	setSemiTrans(ft4, (alpha > 0) ? 1 : 0);
	ENDPRIM(ft4, depth, POLY_FT4);
}

void DrawSpriteOverlayRotating ( SPRITEOVERLAY *spr )
{
	int			atbdx,atbdy, w,h;
	POLY_F4		*f4;
	POLY_FT4	*ft4;
	DR_MODE		*dm;
	TextureType	*tPtr;
	uchar alpha,r,g,b;
	int depth;
	fixed	cosine,sine,newX,newY;
	fixed halfWidth,halfHeight;

	cosine = rcos(-spr->angle);
	sine = rsin(-spr->angle);
	halfWidth = spr->width>>1;
	halfHeight = spr->height>>1;


	depth = 4 - spr->num;

	atbdx = spr->xPos-2048;
	atbdy = spr->yPos-((120+PALMODE*8)*(17-PALMODE));

	tPtr = spr->tex;

	if(spr->a == 255)
	{
		alpha = 0;
//		r = (spr->r * 128) >> 8;
//		g = (spr->g * 128) >> 8;
//		b = (spr->b * 128) >> 8;
		r = spr->r>>1;
		g = spr->g>>1;
		b = spr->b>>1;
	}
	else
	{
		alpha = 2;
		r = (spr->r * spr->a) >> 9;
		g = (spr->g * spr->a) >> 9;
		b = (spr->b * spr->a) >> 9;
	}

	if(spr->tex == 0)
	{
		BEGINPRIM(f4,POLY_F4);
		setPolyF4(f4);

		f4->x0 = -halfWidth;
		f4->x1 = halfWidth;
		f4->x2 = -halfWidth;
		f4->x3 = halfWidth;

		f4->y0 = -halfHeight;
		f4->y1 = -halfHeight;
		f4->y2 = halfHeight;
		f4->y3 = halfHeight;


		newX = FMul(f4->x0,cosine) + FMul(f4->y0,sine);
		newY = FMul(f4->y0,cosine) - FMul(f4->x0,sine);

		f4->x0 = (newX + atbdx + halfWidth)>>3;
		f4->y0 = (newY + atbdy + halfHeight)/(17-PALMODE);

		newX = FMul(f4->x1,cosine) + FMul(f4->y1,sine);
		newY = FMul(f4->y1,cosine) - FMul(f4->x1,sine);

		f4->x1 = (newX + atbdx + halfWidth)>>3;
		f4->y1 = (newY + atbdy + halfHeight)/(17-PALMODE);

		newX = FMul(f4->x2,cosine) + FMul(f4->y2,sine);
		newY = FMul(f4->y2,cosine) - FMul(f4->x2,sine);

		f4->x2 = (newX + atbdx + halfWidth)>>3; 
		f4->y2 = (newY + atbdy + halfHeight)/(17-PALMODE);

		newX = FMul(f4->x3,cosine) + FMul(f4->y3,sine);
		newY = FMul(f4->y3,cosine) - FMul(f4->x3,sine);

		f4->x3 = (newX + atbdx + halfWidth)>>3; 
		f4->y3 = (newY + atbdy + halfHeight)/(17-PALMODE);


		f4->r0 = r<<1;
		f4->g0 = g<<1;
		f4->b0 = b<<1;
		if(alpha)
			setSemiTrans(f4,1);

		ENDPRIM(f4, depth, POLY_F4);

		if(alpha)
		{
			BEGINPRIM(dm, DR_MODE);
			if(spr->flags & SPRITE_SUBTRACTIVE)
				SetDrawMode(dm, 0,1, ((SEMITRANS_SUB-1)<<5),0);
			else
				SetDrawMode(dm, 0,1, ((SEMITRANS_ADD-1)<<5),0);
			ENDPRIM(dm, depth, DR_MODE);
		}
	}
	else
	{
		BEGINPRIM(ft4, POLY_FT4);
		setPolyFT4(ft4);


		ft4->x0 = -halfWidth;
		ft4->x1 = halfWidth;
		ft4->x2 = -halfWidth;
		ft4->x3 = +halfWidth;

		ft4->y0 = -halfHeight;
		ft4->y1 = -halfHeight;
		ft4->y2 = halfHeight;
		ft4->y3 = halfHeight;


		newX = FMul(ft4->x0,cosine) + FMul(ft4->y0,sine);
		newY = FMul(ft4->y0,cosine) - FMul(ft4->x0,sine);

		ft4->x0 = (newX + atbdx + halfWidth)>>3; 
		ft4->y0 = (newY + atbdy + halfHeight)/(17 - PALMODE);

		newX = FMul(ft4->x1,cosine) + FMul(ft4->y1,sine);
		newY = FMul(ft4->y1,cosine) - FMul(ft4->x1,sine);

		ft4->x1 = (newX + atbdx + halfWidth)>>3; 
		ft4->y1 = (newY + atbdy + halfHeight)/(17 - PALMODE);

		newX = FMul(ft4->x2,cosine) + FMul(ft4->y2,sine);
		newY = FMul(ft4->y2,cosine) - FMul(ft4->x2,sine);

		ft4->x2 = (newX + atbdx + halfWidth)>>3; 
		ft4->y2 = (newY + atbdy + halfHeight)/(17 - PALMODE);

		newX = FMul(ft4->x3,cosine) + FMul(ft4->y3,sine);
		newY = FMul(ft4->y3,cosine) - FMul(ft4->x3,sine);

		ft4->x3 = (newX + atbdx + halfWidth)>>3; 
		ft4->y3 = (newY + atbdy + halfHeight)/(17 - PALMODE);

		ft4->r0 = r;//( spr->r * 128 ) >> 8;
		ft4->g0 = g;//( spr->g * 128 ) >> 8;
		ft4->b0 = b;//( spr->b * 128 ) >> 8;
		ft4->u0 = tPtr->u0;
		ft4->v0 = tPtr->v0;
		ft4->u1 = tPtr->u1;
		ft4->v1 = tPtr->v1;
		ft4->u2 = tPtr->u2;
		ft4->v2 = tPtr->v2;
		ft4->u3 = tPtr->u3;
		ft4->v3 = tPtr->v3;
		ft4->tpage = tPtr->tpage;
		if(alpha)
		{
			if(spr->flags & SPRITE_SUBTRACTIVE)
			{
				SETSEMIPRIM(ft4, SEMITRANS_SUB);
			}
			else
			{
				SETSEMIPRIM(ft4, SEMITRANS_ADD);
			}
		}
		ft4->clut = tPtr->clut;
		setSemiTrans(ft4, (alpha > 0) ? 1 : 0);
		ENDPRIM(ft4, depth, POLY_FT4);
	}
}



void PrintSpriteOverlays ( char num )
{
	int counter;
	SPRITEOVERLAY *cur;

	for ( counter = 0; counter < spriteOverlayList.alloc; counter++ )
	{
		cur = &spriteOverlayList.block [ counter ];

		if((cur->draw) && (cur->used) && ((loadingDisplay == 0) || (cur->flags & SPRITE_LOADING)))
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
		
			if ( !cur->angle )
			{
				if(cur->xPos > 4095)
					continue;
				if(cur->flags & SPRITE_CLIP1)
				{
					currClipBox = &clipBox1;
					DrawSpriteClipped(cur);
				}
				else if(cur->flags & SPRITE_CLIP2)
				{
					currClipBox = &clipBox2;
					DrawSpriteClipped(cur);
				}
				else
					DrawSprite ( cur );
			}
			else
				DrawSpriteOverlayRotating ( cur );
		}
	}
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
void DrawTiledBackdrop(int scroll)
{
	int x,y;
	int			atbdx,atbdy, w,h;
	POLY_FT4	*ft4;
	int xRes = 512,yRes = PALMODE ? 256 : 240;
	static int xScroll = 0;
	static int yScroll = 0;
	TextureType *tex;

	w = tileTexture[0]->w-1;
	h = tileTexture[0]->h-1;

	for(x = -4;x < xTile + 4;x++)
	{
		for(y = -2;y < yTile + 2;y++)
		{
			tex = tileTexture[(((x + 4) MOD 2) + ((y + 2) MOD 2) * 2 + ((x + 4)/2) * 2) MOD 4];

			atbdx = (x*xRes)/xTile - xRes/2 + xScroll;
			atbdy = (y*yRes)/yTile - yRes/2 + yScroll;

			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			ft4->x0 = atbdx;
			ft4->y0 = atbdy;
			ft4->x1 = atbdx + xRes/xTile + 1;
			ft4->y1 = atbdy;
			ft4->x2 = atbdx;
			ft4->y2 = atbdy + yRes/yTile;
			ft4->x3 = atbdx + xRes/xTile + 1;
			ft4->y3 = atbdy + yRes/yTile;
			ft4->r0 = 128;
			ft4->g0 = 128;
			ft4->b0 = 128;
			ft4->u0 = tex->u0;
			ft4->v0 = tex->v0;
			ft4->u1 = tex->u1;
			ft4->v1 = tex->v1;
			ft4->u2 = tex->u2;
			ft4->v2 = tex->v2;
			ft4->u3 = tex->u3;
			ft4->v3 = tex->v3;
			ft4->tpage = tex->tpage;
			ft4->clut = tex->clut;
			ENDPRIM(ft4, 1023, POLY_FT4);
		}
	}
	if(scroll)
	{
		xScroll++;//= FMul(gameSpeed,xScrollSpeed);
		if(xScroll > ((xRes*4)/xTile))
			xScroll -= ((xRes*4)/xTile);
		yScroll++;//= FMul(gameSpeed,yScrollSpeed);
		if(yScroll > ((yRes*2)/yTile))
			yScroll -= ((yRes*2)/yTile);
	}
}

void FreeTiledBackdrop()
{
	tileTexture[0] = NULL;
}
