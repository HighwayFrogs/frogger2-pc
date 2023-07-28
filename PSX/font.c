/******************************************************************************************
	AM2 PS   (c) 1999-2001 ISL

	psfont.c:			Font support
******************************************************************************************/

#include "shell.h"
#include <isltex.h>
#include <islmem.h>
#include <islfile.h>
#include "font.h"
#include "textures.h"


// Richard's funky macros for calculating vram related stuff

#define VRAM_PAGECOLS		8
#define VRAM_PAGEROWS		2
#define VRAM_PAGES			16
#define VRAM_PAGEW			32
#define VRAM_PAGEH			32
#define VRAM_SETX(X)		(X)
#define VRAM_SETY(Y)		((Y)*VRAM_PAGEW)
#define VRAM_SETXY(X,Y)		((X)+((Y)*VRAM_PAGEW))
#define VRAM_SETPAGE(P)		((P)<<16)
#define VRAM_GETX(HND)		((HND) & (VRAM_PAGEW-1))
#define VRAM_GETY(HND)		(((HND)/VRAM_PAGEW) & (VRAM_PAGEW-1))
#define VRAM_GETXY(HND)		((HND) & 0xffff)
#define VRAM_GETPAGE(HND)	((HND)>>16)
#define VRAM_CALCVRAMX(HND)	(512+((VRAM_GETPAGE(HND)%(VRAM_PAGECOLS))*64)+(VRAM_GETX(HND)*2))
#define VRAM_CALCVRAMY(HND)	(((VRAM_GETPAGE(HND)/(VRAM_PAGECOLS))*256)+(VRAM_GETY(HND)*8))

#define ANG2FIX(a)		(((a)*4096)/360)

TextureType	*buttonSprites[6];

static TextureType	*otherSprites[16];
static char			otherChars[16];
static int			numOtherSprites;


unsigned char *alphabet = "©∞ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=+[]{}:;\"'|\\,<.>/?"
					"\xe0\xe8\xec\xf2\xf9\xc0\xc8\xcc\xd2\xd9\xe1\xe9\xed\xf3\xfa\xfd\xc1\xc9\xcd\xd3\xda\xdd\xe2\xea\xee\xf4\xfb\xc2\xca\xce\xd4\xdb\xe3\xf1\xf5\xc3\xd1\xd5\xe4\xeb\xef\xf6\xfc\xff\xc4\xcb\xcf\xd6\xdc\xe5\xc5\xe6\xc6\xe7\xc7\xf0\xd0\xf8\xd8\xbf\xa1\xdf";

#define MALLOC0(S)	memoryAllocateZero(S, __FILE__, __LINE__)


static void fontDownload(psFont *font, char *fontdata, int character)
{
	int			    w,h, handle;
	RECT			rect;
	unsigned char	*offset;
	TextureType		*txPtr = &font->txPtr[character];
	
	w = (font->width[character]+7)/8;
	h = (font->height+7)/8;
	handle = textureVRAMalloc(w,h);
	rect.x = VRAM_CALCVRAMX(handle);
	rect.y = VRAM_CALCVRAMY(handle);
	rect.w = (font->width[character]+3)/4;
	rect.h = font->height;

	offset = fontdata + font->dataoffset[character];
	DrawSync(0);
	LoadImage(&rect,(unsigned long *)offset);
	DrawSync(0);

	txPtr->tpage = getTPage(0,0,rect.x,rect.y);
	txPtr->x = VRAM_GETX(handle)*8;
	txPtr->y = rect.y;
	txPtr->w = font->width[character];
	txPtr->h = font->height;
   	txPtr->u0 = txPtr->x;
   	txPtr->v0 = txPtr->y;
   	txPtr->u1 = txPtr->x+font->width[character]-1;
   	txPtr->v1 = txPtr->y;
   	txPtr->u2 = txPtr->x;
   	txPtr->v2 = txPtr->y+font->height-1;
   	txPtr->u3 = txPtr->x+font->width[character]-1;
   	txPtr->v3 = txPtr->y+font->height-1;
	txPtr->clut = font->clut;
	txPtr->handle = handle;
}


/**************************************************************************
	FUNCTION:	fontLoad()
	PURPOSE:	Load font into VRAM ready for use
	PARAMETERS:	Filename
	RETURNS:	Ptr to font info
**************************************************************************/

psFont *fontLoad(char *fontname)
{
	int				loop;
	unsigned char	*fontdata;
	int				*fontptr;
	psFont			*font;
//	unsigned char			*str;

	numOtherSprites = 0;

	font = (psFont *)MALLOC0(sizeof(psFont));
	if ((fontdata = fileLoad(fontname, NULL))==NULL)
		return NULL;

	fontptr = (int *)fontdata;
	font->numchars = *fontptr++;
	font->height = *fontptr++;
	font->clut = textureAddCLUT16((unsigned short *)fontptr);

	fontptr = (int *)fontdata + 10;

	font->txPtr = (TextureType *)MALLOC0(sizeof(TextureType)*font->numchars);
	memset(font->charlookup, -1, 256);
/*	str = "‡ËÏÚ˘¿»Ã“Ÿ·ÈÌÛ˙˝¡…Õ”⁄›‚ÍÓÙ˚¬ Œ‘€„Òı√—’‰ÎÔˆ¸ˇƒÀœ÷‹Â≈Ê∆Á«–¯ÿø°ﬂ";
	for(loop=0; loop<strlen(str); loop++)
	{
		debugPrintf("\\x%02x\n", str[loop]);
	}
	debugPrintf("FONT: ");*/
	for(loop = 0; loop < font->numchars; loop++)
	{
		font->dataoffset[loop] = *fontptr++;
		font->width[loop] = *fontptr++;
		font->pixelwidth[loop] = *fontptr++;
		fontDownload(font, fontdata, loop);
		font->charlookup[alphabet[loop]] = loop;
//		debugPrintf("%c ", alphabet[loop]);
	}
//	debugPrintf("\n");
	FREE(fontdata);

	font->alpha = 0;

	return font;
}


/**************************************************************************
	FUNCTION:	fontUnload()
	PURPOSE:	Unload font from VRAM and free info
	PARAMETERS:	Font ptr
	RETURNS:	
**************************************************************************/

void fontUnload(psFont *font)
{
	int loop;

	for(loop=0; loop<font->numchars; loop++)
		textureUnload(&font->txPtr[loop]);
	FREE(font->txPtr);
	font->txPtr = NULL;
	FREE(font);
}


static void fontDispChar(TextureType *tex, short x,short y, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha,int scale)
{
	POLY_FT4 	*ft4; 

	BEGINPRIM(ft4, POLY_FT4);
	setPolyFT4(ft4);
	setXYWH(ft4, x,y, FMul(tex->w,scale),FMul(tex->h,scale)-1);
	setRGB0(ft4, r,g,b);
	setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
	ft4->tpage = tex->tpage;
	setSemiTrans(ft4, (alpha > 0) ? 1 : 0);
	if(alpha)
		SETSEMIPRIM(ft4, alpha);
	ft4->clut = tex->clut;
	ENDPRIM(ft4, 4, POLY_FT4);
}


/**************************************************************************
	FUNCTION:	fontPrint()
	PURPOSE:	Display string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b
	RETURNS:	
**************************************************************************/

extern psFont *fontSmall;
void fontPrintScaled(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b,int scale)
{
	unsigned char	*strPtr, c;
	int				cx,cy, loop,yAdd;
	TextureType		*letter;
	POLY_FT4 	*ft4; 





	strPtr = text;
	cx = x;
	cy = y;

	if(font == fontSmall)
		yAdd = -3;
	else
		yAdd = 0;

	while(*strPtr)
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			y += font->height;
			cx = x;
			cy = y;
			break;
		case ' ':
			x += font->height/2;
			break;
		case '@':
			strPtr++;
			switch(*strPtr)
			{
			case 'X':
			    fontDispSprite(buttonSprites[2], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[2]->w+2;
				break;
			case 'C':
			   	fontDispSprite(buttonSprites[1], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[1]->w+2;
				break;
			case 'S':
			   	fontDispSprite(buttonSprites[3], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[3]->w+2;
				break;
			case 'T':
			   	fontDispSprite(buttonSprites[0], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[0]->w+2;
				break;
			case 'L':
			   	fontDispSprite(buttonSprites[4], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[4]->w+2;
				break;
			case 'R':
			   	fontDispSprite(buttonSprites[5], x,y + yAdd,r,g,b,font->alpha);
				x += buttonSprites[5]->w+2;
				break;
			}

/*			for(loop = 0; loop < numOtherSprites; loop ++)
			{
				if((*(strPtr+1)) == otherChars[loop])
				{
					fontDispSprite(otherSprites[loop], x+3,y-3,r,g,b,font->alpha);
					strPtr++;
					x += otherSprites[loop]->w+2;
					break;
				}
			}
*/
			break;

		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
			{
				letter = &font->txPtr[loop];
				if ((x>-350)&&(x<320))
				{
					BEGINPRIM(ft4, POLY_FT4);
					setPolyFT4(ft4);
					setXYWH(ft4, x,y/*-((c>127)?(1):(0))*/, FMul(letter->w,scale),FMul(letter->h,scale)-1);
					setRGB0(ft4, r,g,b);
					setUVWH(ft4, letter->x,letter->y, letter->w-1, letter->h-1);
					ft4->tpage = letter->tpage;
					setSemiTrans(ft4, (font->alpha > 0) ? 1 : 0);
					if(font->alpha)
						SETSEMIPRIM(ft4, font->alpha);
					ft4->clut = letter->clut;
					ENDPRIM(ft4, 4, POLY_FT4);

//					fontDispChar(letter, x,y-((c>127)?(1):(0)), r,g,b, font->alpha,scale);
				}
				x += FMul(font->pixelwidth[loop]+2,scale);
			}
		}
		strPtr++;
	}
}


/**************************************************************************
	FUNCTION:	fontExtentW()
	PURPOSE:	Get width of string using font
	PARAMETERS:	font ptr, string
	RETURNS:	Width in pixels
**************************************************************************/

int fontExtentWScaled(psFont *font, char *text,int scale)
{
	unsigned char	*strPtr, c;
	int		loop, x, maxW = 0;

	strPtr = text;
	x = 0;
	while(*strPtr)
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			if (x>maxW)
				maxW = x;
			x = 0;
			break;
		case ' ':
			x += font->height/2;
			break;
		case '@':
			switch(*(strPtr+1))
			{
			case 'X':
				strPtr++;
				x += buttonSprites[2]->w+2;
				break;
			case 'C':
				strPtr++;
				x += buttonSprites[1]->w+2;
				break;
			case 'S':
				strPtr++;
				x += buttonSprites[3]->w+2;
				break;
			case 'T':
				strPtr++;
				x += buttonSprites[0]->w+2;
				break;
			case 'L':
				strPtr++;
				x += buttonSprites[4]->w+2;
				break;
			case 'R':
				strPtr++;
				x += buttonSprites[5]->w+2;
				break;
			}

			for(loop = 0; loop < numOtherSprites; loop ++)
			{
				if((*(strPtr+1)) == otherChars[loop])
				{
					strPtr++;
					x += otherSprites[loop]->w+2;
					break;
				}
			}

			break;
		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
				x += font->pixelwidth[loop]+2;
		}
		strPtr++;
	}
	if (x>maxW)
		maxW = x;
	return FMul(maxW,scale);
}


/**************************************************************************
	FUNCTION:	fontFitToWidth()
	PURPOSE:	Wrap string to width into buffer
	PARAMETERS:	font ptr, width, string, buffer
	RETURNS:	Number of lines
**************************************************************************/

int fontFitToWidth(psFont *font, int width, char *text, char *buffer)
{
	char	buf[256], *bufPtr, *outPtr = buffer;
	int		pixLen, lines = 0,space = 0;

	memset(buf, 0, sizeof(buf));
	bufPtr = buf;
	while(1)
	{
		if (*text>=' ')
		{
			*bufPtr++ = *text;
			if(*text == ' ')
				space = 1;
		}
		pixLen = fontExtentWScaled(font, buf,4096);
		if ((pixLen>width)||(*text=='\0'))
		{
			if(pixLen>width)
			{
				if(space)
				{
					space = 0;
					while(*bufPtr!=' ')
					{
						bufPtr--;
						text--;
					}
					text++;
				}
				else
				{
					bufPtr--;
					text--;
				}
			}
			*bufPtr = '\0';
			strcpy(outPtr, buf);
			outPtr += strlen(buf)+1;
			lines++;
			bufPtr = buf;
			memset(buf, 0, sizeof(buf));
			if (*text=='\0')
				break;
		}
		text++;
	}
/*	debugPrintf("%d lines: \n", lines);
	outPtr = buffer;
	for(pixLen=0; pixLen<lines; pixLen++)
	{
		debugPrintf("  '%s'\n", outPtr);
		outPtr += strlen(outPtr)+1;
	}*/
	return lines;
}



/**************************************************************************
	FUNCTION:	fontPrintN()
	PURPOSE:	Display (partial) string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b, max chars
	RETURNS:	
**************************************************************************/

void fontPrintN(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b, int n,int scale)
{
	unsigned char		*strPtr, c;
	int			cx,cy, loop;
	TextureType	*letter;

	strPtr = text;
	cx = x;
	cy = y;
	while ((*strPtr) && (n>0))
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			y += font->height;
			cx = x;
			cy = y;
			break;
		case ' ':
			x += font->height/2;
			break;
		case '@':
			switch(*(strPtr+1))
			{
			case 'X':
			    fontDispSprite(buttonSprites[2], x,y-3,r,g,b,font->alpha);
				strPtr++;
				x += buttonSprites[2]->w;
				break;
			case 'C':
			   	fontDispSprite(buttonSprites[1], x,y-3,r,g,b,font->alpha);
				strPtr++;
				x += buttonSprites[1]->w;
				break;
			case 'S':
			   	fontDispSprite(buttonSprites[3], x,y-3,r,g,b,font->alpha);
				strPtr++;
				x += buttonSprites[3]->w;
				break;
			case 'T':
			   	fontDispSprite(buttonSprites[0], x,y-3,r,g,b,font->alpha);
				strPtr++;
				x += buttonSprites[0]->w;
				break;
			}

			for(loop = 0; loop < numOtherSprites; loop ++)
			{
				if((*(strPtr+1)) == otherChars[loop])
				{
					fontDispSprite(otherSprites[loop], x,y-3,r,g,b,font->alpha);
					strPtr++;
					x += otherSprites[loop]->w;
					break;
				}
			}

			break;
		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
			{
				letter = &font->txPtr[loop];
				if ((x>-350)&&(x<320))
					fontDispChar(letter, x,y/*-((c>127)?(1):(0))*/, r,g,b,font->alpha,scale);
				x += font->pixelwidth[loop]+2;
			}
		}
		strPtr++;
		n--;
	}
}

void fontDispSprite(TextureType *tex, short x,short y,unsigned char r,unsigned char g,unsigned char b,unsigned char alpha)
{
	POLY_FT4 	*si;
	USHORT		w,h;

	BEGINPRIM(si, POLY_FT4);

	w = tex->w;
	h = tex->h;

	si->x0 = x;
	si->y0 = y;
	si->x1 = x+w-1;
	si->y1 = y;
	si->x2 = x;
	si->y2 = y+h-1;
	si->x3 = x+w-1;
	si->y3 = y+h-1;
		
	si->r0 = r;
	si->g0 = g;
	si->b0 = b;
	si->u0 = tex->u0;
	si->v0 = tex->v0;
	si->u1 = tex->u1;
	si->v1 = tex->v1;
	si->u2 = tex->u2;
	si->v2 = tex->v2;
	si->u3 = tex->u3;
	si->v3 = tex->v3;
	si->tpage = tex->tpage;
	si->clut = tex->clut;

	setPolyFT4(si);
	setSemiTrans(si, (alpha > 0) ? 1 : 0);
	if(alpha)
		SETSEMIPRIM(si, alpha);
	ENDPRIM(si, 4, POLY_FT4);
}


void fontInitButtonSprites()
{
	buttonSprites[0] = FindTexture("BUT_TRIA");
	buttonSprites[1] = FindTexture("BUT_CIRC");
	buttonSprites[2] = FindTexture("BUT_CROS");
	buttonSprites[3] = FindTexture("BUT_SQUA");
	buttonSprites[4] = FindTexture("BUT_LEFT");
	buttonSprites[5] = FindTexture("BUT_RIGH");

/*
	buttonSprites[0] = triangle;
	buttonSprites[1] = circle;
	buttonSprites[2] = cross;
	buttonSprites[3] = square;
*/
}


void fontRegisterOtherSprites(char code, TextureType *sprite)
{

	if(numOtherSprites < 16)
	{
		otherSprites[numOtherSprites] = sprite;
		otherChars[numOtherSprites] = code;
		numOtherSprites ++;
	}
}
