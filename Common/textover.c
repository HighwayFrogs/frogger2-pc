/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: textoverlays.c
	Programmer	: Andy Eder
	Date		: 16/04/99 09:32:23

----------------------------------------------------------------------------------------------- */

//#define F3DEX_GBI_2

#ifndef PSX_VERSION
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#endif

#include <islutil.h>
#include "textover.h"
#include "maths.h"
#include "types2d.h"
#include "main.h"
#include <islmem.h>
#include "layout.h"
#include "define.h"
#include "game.h"
#include "training.h"

#ifdef PC_VERSION
#include <mdx.h>
#include <pcmisc.h>
#endif

//>>
// [ANDYE]
TEXTOVERLAY *pTxtOvrCDAMInfo = NULL;
char szCDAMInfo[32] = { 0 };
//>>

long grabToTexture = 0;

//----- [ GLOBALS ] ----------------------------------------------------------------------------//



//----- [ TEXTOVERLAY HANDLING ] ---------------------------------------------------------------//

/*	--------------------------------------------------------------------------------
	Function		: CreateAndAddTextOverlay
	Purpose			: creates and adds a text overlay string to the list
	Parameters		:
	Returns			: void
	Info			:
*/
TEXTOVERLAY *CreateAndAddTextOverlay(short x, short y, char *text, char centred, unsigned char alpha,
									 psFont *fontPtr, char flags/*, fixed waveAmplitude*/)
{
	TEXTOVERLAY *newItem;
	int j;

	if( !fontPtr ) fontPtr = font;

	//newItem = (TEXTOVERLAY *)MALLOC0(sizeof(TEXTOVERLAY));
	//AddTextOverlay(newItem);

	if (textOverlayList.numEntries == textOverlayList.alloc)
	{
		utilPrintf("CreateAndAddTextOverlay(): Ran out of overlays to allocate!\n");
		return NULL;
	}

	if(textOverlayList.block[textOverlayList.numEntries].used == 0)
		j = textOverlayList.numEntries;
	else
	{
		for(j = 0;j < textOverlayList.alloc;j++)
			if(textOverlayList.block[j].used == 0)
				break;
	}
	newItem = &textOverlayList.block[j];
	textOverlayList.numEntries++;

	newItem->text		= text;
	newItem->flags		= flags;
	newItem->draw		= 1;
	newItem->font		= fontPtr;
	newItem->scale		= 4096;
		
	newItem->xPos		= x;
	newItem->yPos		= y;
	newItem->xPosTo		= x;
	newItem->yPosTo		= y;
	newItem->centred	= centred;
	newItem->r			= 255;
	newItem->g			= 255;	// these were 128... why?
	newItem->b			= 255;
	newItem->a			= alpha;

	newItem->used		= 1;

//	newItem->kill		= NO;
//	newItem->life		= NO;

//	newItem->waveAmplitude	= waveAmplitude;
//	newItem->waveStart		= 0;

	return newItem;
}


/*	--------------------------------------------------------------------------------
	Function		: PrintTextOverlayList
	Purpose			: prints the text overlay list
	Parameters		: none
	Returns			: void
	Info			:
*/

#ifndef PSX_VERSION
extern MDX_FONT *pcFont;
extern MDX_FONT *pcFont2;
#endif

uchar fontAlpha = 2;

void PrintText(TEXTOVERLAY *cur,short xPos,short yPos,uchar r,uchar g,uchar b,uchar a)
{
#ifdef PSX_VERSION
	uchar alphaStore = cur->font->alpha;

//	if(a == 255)
//		cur->font->alpha = 0;
//	else
	{
		cur->font->alpha = fontAlpha;
		r = (r * a)>>9;
		g = (g * a)>>9;
		b = (b * a)>>9;
	}

	if ( cur->centred )
	{
		int length = fontExtentWScaled(cur->font, cur->text,cur->scale)/2;
		cur->tWidth = length;

		if((xPos + length*2 < -255) || (xPos - length > 255))
			return;
		fontPrintScaled(cur->font, xPos-length,yPos, cur->text, r,g,b,cur->scale);

/*
#if PALMODE==1
		fontPrint(cur->font, -256+(xPos>>3)-length,-128+(yPos>>4), cur->text, r,g,b);
#else
		fontPrint(cur->font, -256+(xPos>>3)-length,-120+((yPos*15)>>8), cur->text, r,g,b);
#endif
*/
	}
	else
	{
		int length;

		length = fontExtentWScaled(cur->font, cur->text,cur->scale);
		cur->tWidth = (length*640)/512;

		if((xPos + length < -255) || (xPos > 255))
			return;
		fontPrintScaled(cur->font, xPos,yPos, cur->text, r,g,b,cur->scale);

/*
#if PALMODE==1
		fontPrint(cur->font, -256+(xPos>>3),-128+(yPos>>4), cur->text, r,g,b);
#else
		fontPrint(cur->font, -256+(xPos>>3),-120+((yPos*15)>>8), cur->text, r,g,b);
#endif
*/
	}

	cur->font->alpha = alphaStore;

#else
	float scale;

	if( !rHardware && rXRes < 640 && (MDX_FONT *)cur->font == pcFontSmall )
		scale = 1.0f;
	else
	{
		if( cur == train )
			scale = 1.0f;
		else
			scale = ((float)cur->scale)/4096.0 * ((float)rXRes/640.0);
	}


	if (cur->centred)
	{
		cur->tWidth = DrawFontStringAtLoc(xPos,yPos,cur->text,
		RGBA_MAKE(r,g,b,a), (MDX_FONT *)cur->font,scale,xPos,0);
	}
	else
	{
		cur->tWidth = DrawFontStringAtLoc(xPos,yPos,cur->text,
		RGBA_MAKE(r,g,b,a), (MDX_FONT *)cur->font,scale,0,0) - xPos;
	}

	cur->tWidth *= (640.0/(float)rXRes);
#endif
}

void PrintTextOverlays()
{
	TEXTOVERLAY *cur;
	long n;

	if(textOverlayList.numEntries == 0)
		return;

	for(cur=textOverlayList.block, n=textOverlayList.alloc; n; cur++, n--)
	{

#ifdef PSX_VERSION
		if((cur->used) && (cur->draw) && ((gameState.mode != PAUSE_MODE) || (cur->flags & TEXTOVERLAY_PAUSED)) && ((loadingDisplay == 0) || (cur->flags & TEXTOVERLAY_LOADING)))
#else
		if((cur->used) && (cur->draw) && ((gameState.mode != PAUSE_MODE) || (cur->flags & TEXTOVERLAY_PAUSED)))
#endif
		{
			if(cur->speed)
			{
				fixed spd = FRoundRandomHack(FMul(gameSpeed, cur->speed));

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
			}

#ifdef PC_VERSION
			if(cur->flags & TEXTOVERLAY_SHADOW)
				PrintText(cur,cur->xPos*OVERLAY_X,cur->yPos*OVERLAY_Y + (cur->font == font ? 6 : 4),0,0,0,(cur->a * 7)/10.0);
			PrintText(cur,cur->xPos*OVERLAY_X,cur->yPos*OVERLAY_Y,cur->r,cur->g,cur->b,cur->a);
#endif
#ifdef PSX_VERSION
			if(cur->flags & TEXTOVERLAY_PIXELS)
			{
				if(PALMODE)
					PrintText(cur,-256+cur->xPos,-128+cur->yPos,cur->r,cur->g,cur->b,cur->a);
				else
					PrintText(cur,-256+cur->xPos,-120+cur->yPos,cur->r,cur->g,cur->b,cur->a);
				if((cur->flags & TEXTOVERLAY_SHADOW) && (cur->a > 120))
				{
					fontAlpha = 1;
					if(PALMODE)
						PrintText(cur,-256+cur->xPos,-128+cur->yPos + (cur->font == font ? 3 : 2),0,0,0,(cur->a * 7)/10);
					else
						PrintText(cur,-256+cur->xPos,-120+cur->yPos + (cur->font == font ? 3 : 2),0,0,0,(cur->a * 7)/10);
					fontAlpha = 2;
				}
			}
			else
			{
				if(PALMODE)
					PrintText(cur,-256+(cur->xPos>>3),-128+(cur->yPos>>4),cur->r,cur->g,cur->b,cur->a);
				else
					PrintText(cur,-256+(cur->xPos>>3),-120+((cur->yPos*15)>>8),cur->r,cur->g,cur->b,cur->a);
				if((cur->flags & TEXTOVERLAY_SHADOW) && (cur->a > 120))
				{
					fontAlpha = 1;
					if(PALMODE)
						PrintText(cur,-256 + (cur->xPos>>3),-128 + (cur->yPos>>4) + (cur->font == font ? 3 : 2),0,0,0,(cur->a * 7)/10);
					else
						PrintText(cur,-256 + (cur->xPos>>3),-120 + ((cur->yPos*15)>>8) + (cur->font == font ? 3 : 2),0,0,0,(cur->a * 7)/10);
					fontAlpha = 2;
				}
			}
#endif

			//utilPrintf("cur->xPos : %d, cur->yPos : %d, cur->xPosTo : %d, cur->yPosTo : %d\n", cur->xPos, cur->yPos, cur->xPosTo, cur->yPosTo);
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: EnableTextOverlay
	Purpose			: enables a text overlay for display
	Parameters		: TEXTOVERLAY *
	Returns			: none
	Info			:
*/
void EnableTextOverlay(TEXTOVERLAY *tover)
{
	if( tover ) tover->draw = 1;
}

/*	--------------------------------------------------------------------------------
	Function		: DisableTextOverlay
	Purpose			: disables a text overlay from displaying
	Parameters		: TEXTOVERLAY *
	Returns			: none
	Info			:
*/
void DisableTextOverlay(TEXTOVERLAY *tover)
{
	if (tover) tover->draw = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: InitTextOverlayLinkedList
	Purpose			: initialises the text overlay linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitTextOverlayLinkedList(long num)
{
	if (textOverlayList.block)
		FREE(textOverlayList.block);
	textOverlayList.numEntries = 0;
	textOverlayList.alloc = num;
	textOverlayList.block = MALLOC0(sizeof(TEXTOVERLAY)*textOverlayList.alloc);

    //>>
    // [ANDYE]
    pTxtOvrCDAMInfo = CreateAndAddTextOverlay(32, 0, szCDAMInfo, NO, 255, fontSmall, TEXTOVERLAY_NORMAL | TEXTOVERLAY_PAUSED);
    if (pTxtOvrCDAMInfo)
    {
        pTxtOvrCDAMInfo->b = 0;
        pTxtOvrCDAMInfo->scale = 4096;
    }
    //>>
}

/*	--------------------------------------------------------------------------------
	Function		: FreeTextOverlayLinkedList
	Purpose			: frees the text overlay linked list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeTextOverlayLinkedList()
{
    if (textOverlayList.block != NULL)
    {
        FREE(textOverlayList.block);
        textOverlayList.block = NULL;
    }
	//InitTextOverlayLinkedList();

    //>>
    // [ANDYE]
    pTxtOvrCDAMInfo = NULL;
    //>>
}

/*	--------------------------------------------------------------------------------
	Function		: SubTextOverlay
	Purpose			: removes a text overlay from the list
	Parameters		: TEXTOVERLAY *
	Returns			: void
	Info			: 
*/
void SubTextOverlay(TEXTOVERLAY *tOver)
{
	if((tOver) && (tOver->used))
	{
		tOver->used = 0;
		tOver->draw = 0;
		textOverlayList.numEntries--;
	}
}



/*	--------------------------------------------------------------------------------
	Function		: StringWrap
	Purpose			: wraps a string into a buffer and saves a pointer to the start of each line in an array
	Parameters		: string, max width of string (overlay units), work buffer, size of buffer, array, length of array, font
	Returns			: void
	Info			: 
*/
long StringWrap(const char* str, long maxWidth, char* buffer, long bufferSize, char** array, long arraySize, psFont *font)

#ifdef PC_VERSION
{
	return WrapStringToArray(str, (maxWidth*rXRes)>>12, buffer, bufferSize, array, arraySize, (MDX_FONT*)font);
}
#else
{
	int i, lines; char *c;
	lines = fontFitToWidth(font, maxWidth>>3, (char*)str, buffer);

	//bb
	if(lines>arraySize)
		utilPrintf("\nERROR ! ! !  OVERRUN IN StringWrap()\n\n");
	
	// fontFitToWidth returns a tokenised char* buffer, each line null-terminated..
	// we want to strip these lines out for ease of access
	for (i=0, c=buffer; i<lines; i++)
	{
		array[i] = c;
		c += strlen(c)+1;
	}

	while (i<arraySize)	array[i++] = c-1;

	return lines;
}
#endif

#ifdef PC_VERSION
/**************************************************************************
	FUNCTION:	fontFitToWidth()
	PURPOSE:	Wrap string to width into buffer
	PARAMETERS:	font ptr, width, string, buffer
	RETURNS:	Number of lines
**************************************************************************/

int fontFitToWidth(psFont *font, int width, char *text, char *buffer)
{
	char	buf[256], *bufPtr, *outPtr = buffer;
	int		pixLen, lines = 0;

	memset(buf, 0, sizeof(buf));
	bufPtr = buf;
	while(1)
	{
		if (*text>=' ')
			*bufPtr++ = *text;
		pixLen = CalcStringWidth(buf, (MDX_FONT *)font, (float)rXRes/640.0);//fontExtentWScaled(font, buf,4096);
		if ((pixLen>width)||(*text=='\0'))
		{
			if (pixLen>width)
			{
				while(*bufPtr!=' ')
				{
					bufPtr--;
					text--;
				}
				text++;
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
#endif