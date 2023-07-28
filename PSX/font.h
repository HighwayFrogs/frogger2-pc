/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islfont.h:		Font support

************************************************************************************/

#ifndef __ISLFONT_H__
#define __ISLFONT_H__

typedef struct _psFont
{
	int				numchars;
	int				height;
	int				dataoffset[256];
	int				width[256];
	int				pixelwidth[256];
	TextureType		*txPtr;
	unsigned short	clut;
	unsigned char	charlookup[256];
	unsigned char	alpha;
	
} psFont;


/**************************************************************************
	FUNCTION:	fontLoad()
	PURPOSE:	Load font into VRAM ready for use
	PARAMETERS:	Filename
	RETURNS:	Ptr to font info
**************************************************************************/

psFont *fontLoad(char *fontname);


/**************************************************************************
	FUNCTION:	fontUnload()
	PURPOSE:	Unload font from VRAM and free info
	PARAMETERS:	Font ptr
	RETURNS:	
**************************************************************************/

void fontUnload(psFont *font);


/**************************************************************************
	FUNCTION:	fontPrint()
	PURPOSE:	Display string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b
	RETURNS:	
**************************************************************************/

void fontPrintScaled(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b,int scale);


/**************************************************************************
	FUNCTION:	fontExtentW()
	PURPOSE:	Get width of string using font
	PARAMETERS:	font ptr, string
	RETURNS:	Width in pixels
**************************************************************************/

int fontExtentWScaled(psFont *font, char *text,int scale);


/**************************************************************************
	FUNCTION:	fontFitToWidth()
	PURPOSE:	Wrap string to width into buffer
	PARAMETERS:	font ptr, width, string, buffer
	RETURNS:	Number of lines
**************************************************************************/

int fontFitToWidth(psFont *font, int width, char *text, char *buffer);


/**************************************************************************
	FUNCTION:	fontPrintN()
	PURPOSE:	Display (partial) string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b, max chars
	RETURNS:	
**************************************************************************/

//void fontPrintN(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b, int n);


/**************************************************************************
	FUNCTION:	fontRegisterButtonSprites()
	PURPOSE:	Set sprites to be used for button display
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void fontInitButtonSprites();


/**************************************************************************
	FUNCTION:	fontRegisterOtherSprites()
	PURPOSE:	Set sprites to be used for display
	PARAMETERS:	character code ,pointers to sprite
	RETURNS:	
**************************************************************************/

void fontRegisterOtherSprites(char code, TextureType *sprite);
void fontDispSprite(TextureType *tex, short x,short y,unsigned char r,unsigned char g,unsigned char b,unsigned char alpha);


#endif //__ISLFONT_H__

