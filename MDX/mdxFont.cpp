/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: mdxFont.cpp
	Programmer	: David Swift (reluctantly)
	Date		: 18 May 00

----------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <gelf.h>
#include <softstation.h>

#include "mdxException.h"	// for memory allocation, OBVIOUSLY...
#include "mgeReport.h"		// for 'reporting' stuff?

#include "mdxD3D.h"
#include "mdxMath.h"
#include "mdxFont.h"
#include "mdxTexture.h"
#include "mdxPoly.h"
#include "mdxddraw.h"
#include "mdxpoly.h"
#include <language.h>		// yes yes yes yes yes yes yes yes . yes
#include "controll.h"
#include "layout.h"

#define FONT_TEXTURE_SIZE		256
#define FONT_TEMP_SURFACES		12


/* -----------------------------------------------------------------------
   Stucture : MDX_FONTCHAR
   Purpose : font character
   Info :
*/
struct MDX_FONTCHAR
{
	LPDIRECTDRAWSURFACE7	surf;				// hardware font character surface
	float					coords[4];			// hardware texture cordinates
	short					width;				// charater pixel width
	unsigned long			*softData;			// software font data
};


/* -----------------------------------------------------------------------
   Stucture : MDX_FONT
   Purpose : font
   Info :
*/
typedef struct _MDX_FONT
{
	LPDIRECTDRAWSURFACE7	*surf;				// all font surfaces
	long					numSurfs;			// number of surfaces used
	MDX_FONTCHAR			characters[256];	// entry for each character used in the font
	long					width;				// source font width
	long					height;				// source font height
} MDX_FONT;


// *ASL* 15/06/2000
// alphabet string for all character fonts - important this is kept as unsigned chars rather than chars!!!
const unsigned char alphabet[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=+[]{}:;\"'|\\,<.>/?"
		"\xe0\xe8\xec\xf2\xf9\xc0\xc8\xcc\xd2\xd9\xe1\xe9\xed\xf3\xfa\xfd\xc1\xc9\xcd\xd3\xda\xdd\xe2\xea\xee\xf4\xfb\xc2\xca\xce\xd4\xdb\xe3\xf1\xf5\xc3\xd1\xd5\xe4\xeb\xef\xf6\xfc\xff\xc4\xcb\xcf\xd6\xdc\xe5\xc5\xe6\xc6\xe7\xc7\xf0\xd0\xf8\xd8\xbf\xa1\xdf©";

long CalcStringWidth(const char *string,MDX_FONT *font, float scale);
long GetCharWidth(unsigned char c, MDX_FONT *font, float scale);


#define FONT_TRANSVAL	0xf81f


/* -----------------------------------------------------------------------
   Function: InitFont
   Purpose : initialises a font from a bitmap file
   Parameters : font filename pointer
   Returns : mdx font pointer or NULL error
   Info :
*/

MDX_FONT *InitFont(const char *filename)
{
	unsigned short	*tData, *scratch;
	unsigned short	transparent;
	int				i, pptr = -1, bmpWidth, bmpHeight, charSize;
	int				currSurf = 0;
	int				txpos = 0;
	POINT			charUV = { 0, 0 };
	MDX_FONT		*font;
	int				alphaLen;


	LPDIRECTDRAWSURFACE7 surfaces[FONT_TEMP_SURFACES];

	tData = (unsigned short *)gelfLoad_BMP((char*)filename,NULL,(void**)&pptr,&bmpWidth,&bmpHeight,NULL,GELF_IFORMAT_16BPP555,GELF_IMAGEDATA_TOPDOWN);
	if (!tData)
		return NULL;

	// take transparent colour as the top-left pixel colour
	transparent = tData[0];

	font = (MDX_FONT*)AllocMem(sizeof(MDX_FONT));
	font->width  = bmpWidth;
	font->height = bmpHeight;
	for (i=0; i<256; i++)
	{
		font->characters[i].width = 0;
		font->characters[i].softData = NULL;
	}

	charSize = bmpHeight;

	//for (charSize=1;charSize<bmpHeight;charSize<<=1);	
	scratch = (unsigned short*)AllocMem(2*FONT_TEXTURE_SIZE*FONT_TEXTURE_SIZE);
	for (int clear=0;clear<FONT_TEXTURE_SIZE*FONT_TEXTURE_SIZE;clear++)
		scratch[clear]=FONT_TRANSVAL;

	//surfaces[0] = D3DCreateTexSurface(FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, FONT_TRANSVAL, 0, 1);
	surfaces[currSurf] = D3DCreateTexSurface(FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, FONT_TRANSVAL, 0, 1);

	// build all font characters
	alphaLen = strlen((const char *)alphabet);
	for (i=0; i<alphaLen; i++)
	{
		int yscan, found = 0;

		// find first scan containing a coloured pixel
		while (txpos<bmpWidth)
		{
			for (yscan=0;yscan<bmpHeight;yscan++)
				if (tData[yscan*bmpWidth + txpos] != transparent)
				{
					found=1;
					break;
				}

			if (found)
				break;
			else
				txpos++;
		}

		int left = txpos, width = 1;
		txpos++;

		// find first scan NOT containing a coloured pixel, incrementing width
		while (txpos<bmpWidth)
		{
			found = 0;
			for (yscan=0;yscan<bmpHeight;yscan++)
				if (tData[yscan*bmpWidth + txpos] != transparent)
				{
					found=1;
					break;
				}
			
			if (!found)
				break;
				
			width++, txpos++;

		}

		if (charUV.x+width >= FONT_TEXTURE_SIZE)
		{
			charUV.x = 0;
			charUV.y += charSize;

			if (charUV.y >= (FONT_TEXTURE_SIZE-charSize))
			{
				DDrawCopyToSurface(surfaces[currSurf], (unsigned short *)scratch,0,FONT_TEXTURE_SIZE,FONT_TEXTURE_SIZE,0);
				for (int clear=0;clear<FONT_TEXTURE_SIZE*FONT_TEXTURE_SIZE;clear++) scratch[clear]=FONT_TRANSVAL;
				charUV.y = 0;

				if ((++currSurf) == FONT_TEMP_SURFACES)
					break;

				surfaces[currSurf] = D3DCreateTexSurface(FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, FONT_TRANSVAL, 0, 1);
			}
		}

		// copy stuff into texture scratch buffer

		for (int y=0; y<bmpHeight; y++)
			for (int x=0; x<width; x++)
			{
				unsigned short *dt;
				//unsigned long d,r,g,b;
				dt = &tData[(x+left)+(y*bmpWidth)];

				if (*dt==transparent)
					*dt = FONT_TRANSVAL;
				//else
				//	*dt |= 0x8000;	//set alpha bit for ARGB 1555? - ds

				scratch[(y+charUV.y)*FONT_TEXTURE_SIZE+(x+charUV.x)] = *dt;
			}

		MDX_FONTCHAR *c = &font->characters[alphabet[i]];

		c->surf = surfaces[currSurf];
		c->coords[0] = charUV.x/(float)FONT_TEXTURE_SIZE;
		c->coords[1] = charUV.y/(float)FONT_TEXTURE_SIZE;
		c->coords[2] = (charUV.x + width)/(float)FONT_TEXTURE_SIZE;
		c->coords[3] = (charUV.y + charSize)/(float)FONT_TEXTURE_SIZE;
		c->width = width;
		c->softData = NULL;					// null software font data

		charUV.x += width+1;


		// *ASL* 15/06/2000
		// ** Now we initialise the software texture font. For each character entry
		// ** we hold a pointer to its corresponding 24bit rgb data.

		if (!rHardware)
		{
			MDX_FONTCHAR	*fontChar;
			int				cx, cy;
			unsigned short	rgb555;
			unsigned long	*cp;

			// hash character font entry
			fontChar = c;

			// allocate rgb buffer for font character
			if (fontChar->softData == NULL)
				if ((fontChar->softData = (unsigned long *)AllocMem(sizeof(unsigned long) * width * charSize)) != NULL)
				{
					// convert font character into 24bit rgb
					cp = fontChar->softData;
					for (cy=0; cy<charSize; cy++)
						for (cx=0; cx<width; cx++)
						{
							if ((rgb555 = tData[(cx + left) + (cy*bmpWidth)]) == FONT_TRANSVAL)
								*cp++ = 0xff00ff;
							else
								*cp++ = ((((((rgb555 & 0x7c00)>>10)+1)<<3)-1)<<16) | 
										((((((rgb555 & 0x03e0)>>5)+1)<<3)-1)<<8) | 
										((((rgb555 & 0x001f)+1)<<3)-1);
						}
				}
		}

		if (txpos == bmpWidth)
			break;
	}

	DDrawCopyToSurface(surfaces[currSurf], (unsigned short *)scratch,0,FONT_TEXTURE_SIZE,FONT_TEXTURE_SIZE,0);

	currSurf++;

	// copy surfaces into the right place
	font->surf = (LPDIRECTDRAWSURFACE7*)AllocMem(sizeof(LPDIRECTDRAWSURFACE7)*currSurf);
	memcpy(font->surf, surfaces, sizeof(LPDIRECTDRAWSURFACE7)*currSurf);

	font->numSurfs = currSurf;
	font->characters[' '].width = font->characters['!'].width;

	FreeMem(scratch);

	// release the gelf image file
	gelfDefaultFree(tData);

	dp("Font '%s' loaded, %d %dx%d textures used\n", filename, currSurf, FONT_TEXTURE_SIZE,FONT_TEXTURE_SIZE);
	return font;
}


/* -----------------------------------------------------------------------
   Function: ShutdownFont
   Purpose : shut down and release font
   Parameters : mdx font pointer
   Returns : 
   Info :
*/

void ShutdownFont(MDX_FONT *mdxFont)
{
	int	l;

	if (mdxFont == NULL)
		return;

	// release font surface buffers
	if (mdxFont->surf)
		FreeMem(mdxFont->surf);
	// release all font characters
	for (l=0; l<256; l++)
		if (mdxFont->characters[l].softData)
		{
			FreeMem(mdxFont->characters[l].softData);
			mdxFont->characters[l].softData = NULL;
		}
	// release the font structure
	FreeMem(mdxFont);
}



/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
void InitFontSystem(void)
{
	
}

#if 0
void CopyCharSoftware(MDX_FONT *font, char c, int x, int y)
{
	MDX_FONTCHAR *ch = &font->characters[c];

	short *py = softScreen + (y<<8)+(y<<6)+(x>>1); //(y*640 + x;
	short *cy = font->softData + ch->softCoord;

	for (int i=(font->height>>1); i; i--)
	{
		short *px = py;
		short *cx = cy;

		for (int j=(ch->width>>1)-1; j; j--)
		{
			unsigned short p = (unsigned short)*cx; cx+=2;
			if (p != FONT_TRANSVAL)
				*(px++) = p;
		}

		cy += (font->width<<1);
		py += 640; //640;
	}
}
#endif


// *ASL* 14/06/2000
/* -----------------------------------------------------------------------
   Function : DrawFontCharAtLoc
   Purpose : draw font charater ch at location xy
   Parameters : x, y, char, colour, mdx texture font, scale
   Returns : character pixel width
   Info : 
*/

long DrawFontCharAtLoc(long x, long y, unsigned char ch, unsigned long colour, MDX_FONT *font, float scale)
{
	RECT			rc;
	MDX_FONTCHAR	*mdxChar;
	MDX_TEXENTRY	tmap;

	if (font == NULL)
		return 0;
	mdxChar = &font->characters[ch];

	rc.left = x;
	rc.right = x + (mdxChar->width*scale);
	rc.top = y;
	rc.bottom = y + (font->height*scale);

	if (rHardware)
	{
		if (mdxChar->surf != NULL)
			DrawTexturedRect(rc, colour, mdxChar->surf, mdxChar->coords[0], mdxChar->coords[1], mdxChar->coords[2], mdxChar->coords[3]);
	}
	else
	{
		if (mdxChar->softData)
		{
			memset(&tmap, 0, sizeof(tmap));
			tmap.softData = (long *)mdxChar->softData;
			tmap.xSize = mdxChar->width;
			tmap.ySize = font->height;
			tmap.keyed = 1;
			mdxPolyDrawTextureRect(rc, colour, &tmap, 0.0f, 0.0f, 1.0f, 1.0f);
		}
	}
	return mdxChar->width*scale;
}



long DrawFontStringAtLoc(long x,long y,char *c,unsigned long color, MDX_FONT *font, float scale,long centredX,long centredY)
{
	char *str,*nullStr = "";

	if (!font || !c)
		return 0;

	long cx = x;

	if (centredX)
		cx = centredX-CalcStringWidth(c,font,scale)/2;

	while (*c)
	{
		switch(*c)
		{
			case '@':
				c++;
				switch(*c)
				{
					case 'T':
						str = DIKStrings[keymap[6].key][gameTextLang];
						break;
					case 'X':
						str = DIKStrings[keymap[4].key][gameTextLang];
						break;
					case 'S':
						str = DIKStrings[keymap[5].key][gameTextLang];
						break;
					case 'C':
						str = DIKStrings[keymap[7].key][gameTextLang];
						break;
					default:
						str = nullStr;
						break;
				}

				cx = DrawFontStringAtLoc(cx,y,str,color,font,scale,0,0);
				c++;
				break;

			default:
				cx += DrawFontCharAtLoc(cx,y,(unsigned char)*c,color,font,scale);
				c++;
				break;
		}
	}

	return cx;
}



/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
long GetCharWidth(unsigned char c, MDX_FONT *font, float scale)
{
	if (!font)
		return 0;

	return font->characters[c].width;
}

/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
long CalcStringWidth(const char *string,MDX_FONT *font, float scale)
{
	char *str;

	if (!font || !string)
		return 0;

	long width = 0;
	char *c = (char*)string;
	
	while (*c)
	{
		switch(*c)
		{
			case '@':
				c++;
				switch(*c)
				{
					case 'T':
						str = DIKStrings[keymap[6].key][gameTextLang];
						break;
					case 'X':
						str = DIKStrings[keymap[4].key][gameTextLang];
						break;
					case 'S':
						str = DIKStrings[keymap[5].key][gameTextLang];
						break;
					case 'C':
						str = DIKStrings[keymap[7].key][gameTextLang];
						break;
				}
				width += CalcStringWidth(str,font,scale);
				c++;
				break;

			default:
				width += font->characters[(unsigned char)*c].width*scale;
				c++;
				break;
		}
	}

	return width;
}


/*	--------------------------------------------------------------------------------
	Function		: 
	Purpose			: 
	Parameters		: 
	Returns			: 
	Info			: 
*/
long WrapStringToArray(const char* str, long maxWidth, char* buffer, long bufferSize, char** array, long arraySize, MDX_FONT *font)
{
	char *wordStart, *lineStart, *p;
	char **line = array;
	long lines = 0, lineWidth = 0, lineChars = 0, wordChars = 0;

	memset(array, 0, sizeof(char*)*arraySize);

	lineStart = wordStart = p = (char*)str;
	*line = buffer;

	while (1)
	{
		unsigned char c = (unsigned char)*p;
		long charWidth;
		
		if(c == '@')
		{
			p++;
			lineChars++;
			wordChars++;
			c = (unsigned char)*p;
			switch(c)
			{
				case 'T':
					charWidth = CalcStringWidth(DIKStrings[keymap[6].key][gameTextLang],font,1.0f);
					break;
				case 'X':
					charWidth = CalcStringWidth(DIKStrings[keymap[4].key][gameTextLang],font,1.0f);
					break;
				case 'S':
					charWidth = CalcStringWidth(DIKStrings[keymap[5].key][gameTextLang],font,1.0f);
					break;
				case 'C':
					charWidth = CalcStringWidth(DIKStrings[keymap[7].key][gameTextLang],font,1.0f);
					break;
			}
		}
		else
			charWidth = GetCharWidth(c, font, 1.0f);
		
		if (lineWidth+charWidth >= maxWidth || c == 0)
		{
			if (c) // && wordChars < lineChars/2)
				lineChars -= wordChars;		

			if (lines == arraySize-1)
			{
				strcpy(*line, lineStart);
				lines++;
				break;
			}
			else
			{			
				memcpy(*line, lineStart, lineChars);
				*((*line)+lineChars) = 0;

				lines++;

				if (c)
				{
					*(line+1) = *line + lineChars + 1;
					line++;
					lineStart = p = wordStart;
					lineWidth = lineChars = 0;
					continue;
				}
				else
				{
					while (lines < arraySize)
						(array[lines++]) = *line + lineChars;
					break;	// we're done!
				}
			}
		}

		lineChars++;

		if (c == ' ')
		{
			wordChars = 0; wordStart = p+1;
		}			
		else
		{
			wordChars++;
		}
		
		lineWidth += charWidth;
		p++;
	}

	return lines;
}
