/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	isltex.c			Texture and VRAM management

************************************************************************************/
   
#include <stddef.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <string.h>
#include <stdio.h>
#include "isltex.h"
#include <islmem.h>
#include "file.h"
#include <islpad.h>
#include "font.h"
#include <islutil.h>
#include "shell.h"

extern psFont	*font;

static int	VRAM_PALETTES;
static int	VRAM_256PALETTES;

// Richard's funky macros for calculating vram related stuff

#define VRAM_STARTX			512
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

#define MALLOC0(S)	memoryAllocateZero(S, __FILE__, __LINE__)

TextureBankType *texBank[MAXTEXBANKS];

unsigned char	VRAMblock[VRAM_PAGES][VRAM_PAGEW*VRAM_PAGEH];

unsigned short	*VRAMpalBlock;
int				*VRAMpalHandle;
unsigned short	*VRAMpalCLUT;
unsigned long	*VRAMpalCRC;

unsigned long	*VRAMpal256CRC;
unsigned long	*VRAMpal256CLUT;
unsigned short	*VRAMpal256Block;

// workspace for the vram viewer to display the current palette
unsigned short	currentPal16[16];
unsigned short	currentPal256[256];


#ifdef _DEBUG
unsigned int	palsused = 0;
#endif

void textureVRAMprint()
{
	int		page, x,y;
	unsigned char	*bPtr;

	bPtr = (unsigned char *)VRAMblock;
	for(page=0; page<VRAM_PAGES; page++)
	{
		printf("PAGE %d:\n  ", page);
		for(y=0; y<VRAM_PAGEH; y++)
		{
			for(x=0; x<VRAM_PAGEW; x++)
				printf("%02x|", *bPtr++);
			printf("\n  ");
		}
	}
}


int textureVRAMalloc(short w, short h)
{
	int		page, x,y, xx,yy;
	unsigned char	found;

	for(page = 0; page < VRAM_PAGES; page ++)
	{
		for(y = 0; y <= VRAM_PAGEH - h; y ++)
		{
			for(x = 0; x <= VRAM_PAGEW - w; x ++)
			{
				if (VRAMblock[page][VRAM_SETXY(x, y)] == 0)
				{
					found = 0;
					for(yy = y; yy < y + h; yy ++)
					{
						for(xx = x; xx < x + w; xx ++)
						{
							found |= (VRAMblock[page][VRAM_SETXY(xx, yy)]);
						}
					}
					if (!found)
					{
						for(yy = y; yy < y + h; yy ++)
						{
							for(xx = x; xx < x + w; xx ++)
							{
								VRAMblock[page][VRAM_SETXY(xx, yy)] = 1;
							}
						}
//						printf("Allocated %dx%d, from page %d @ %d,%d\n", w,h,page,x,y);
						return (VRAM_SETPAGE(page) | VRAM_SETXY(x, y));
					}
				}
			}
		}
	}
	printf("VRAM alloc FAILED!\n");
	return NULL;
}


void textureVRAMfree(int handle, short w, short h)
{
	int		xx,yy;
	int		page, x,y;

	page = VRAM_GETPAGE(handle);
	x = VRAM_GETX(handle);
	y = VRAM_GETY(handle);
//	printf("Freed VRAM [0x%x] %dx%d, from page %d @ %d,%d\n", handle,w,h,page,x,y);
	for(yy=y; yy<y+h; yy++)
	{
		for(xx=x; xx<x+w; xx++)
		{
			VRAMblock[page][VRAM_SETXY(xx,yy)] = 0;
		}
	}
}

/**************************************************************************
	FUNCTION:	textureInit256ClutSpace()
	PURPOSE:	Allocate some space for 256 colour palettes
	PARAMETERS:	
	RETURNS:
**************************************************************************/

void textureInit256ClutSpace(int numCluts)
{
	int		page, x,i;

	// get multiple of 8
	numCluts = (numCluts + 7) / 8;

	for(page = 0; page < 4; page ++)
	{
		for(x = 0; x < VRAM_PAGEW; x ++)
		{
			for(i = 0; i < numCluts; i ++)
			{
				VRAMblock[page][VRAM_SETXY(x, i)] = 1;
			}
		}
	}

	for(i = 0; i < VRAM_256PALETTES; i ++)
	{
		VRAMpal256Block[i] = 0;
		VRAMpal256CRC[i] = 0;
	}
}


/**************************************************************************
	FUNCTION:	textureInitialise()
	PURPOSE:	Initialise VRAM/texture handling
	PARAMETERS:	Number of 16 colour palettes, number of 256 colour palettes
	RETURNS:	
**************************************************************************/

void textureInitialise(int num16Cluts, int num256Cluts)
{
	int				page;
	RECT			rect;
	unsigned char	r,g,b;
	int				loop;
	int				mallocSize;

	for(loop=0; loop<MAXTEXBANKS; loop++)
		texBank[loop] = NULL;

	if(num16Cluts > 0)
	{
#ifdef _DEBUG
	printf("\ntextureInitialise: allocating space for %d 16-colour cluts", num16Cluts);
#endif
		
		mallocSize = (sizeof(unsigned short) * num16Cluts) + (sizeof(int) * (num16Cluts / 32)) +
			(sizeof(unsigned short) * num16Cluts) + (sizeof(unsigned long) * num16Cluts);
		
		VRAMpalBlock = MALLOC0(mallocSize);
		VRAMpalHandle = (int *)((unsigned char *)VRAMpalBlock + (sizeof(unsigned short) * num16Cluts));
		VRAMpalCLUT = (unsigned short *)((unsigned char *)VRAMpalHandle + (sizeof(int) * (num16Cluts / 32)));
		VRAMpalCRC = (unsigned long *)((unsigned char *)VRAMpalCLUT + (sizeof(unsigned short) * num16Cluts));
/*
		VRAMpalBlock = MALLOC(sizeof(unsigned short) * num16Cluts);
		VRAMpalHandle = MALLOC(sizeof(int) * (num16Cluts / 32));
		VRAMpalCLUT = MALLOC(sizeof(unsigned short) * num16Cluts);
		VRAMpalCRC = MALLOC(sizeof(unsigned long) * num16Cluts);
*/
		memset(VRAMpalBlock, 0, sizeof(unsigned short) * num16Cluts);
		memset(VRAMpalHandle, 0, sizeof(int) * (num16Cluts / 32));
		memset(VRAMpalCLUT, 0, sizeof(unsigned short) * num16Cluts);
		memset(VRAMpalCRC, 0, sizeof(unsigned long) * num16Cluts);

		VRAM_PALETTES = num16Cluts;
	}
	else
	{
		VRAM_PALETTES = 0;
	}

	if(num256Cluts > 0)
	{
#ifdef _DEBUG
	printf("\ntextureInitialise: allocating space for %d 256-colour cluts", num256Cluts);
#endif
		mallocSize = (sizeof(unsigned short) * num256Cluts) + (sizeof(unsigned long) * num256Cluts) +
			(sizeof(unsigned long) * num256Cluts);

		VRAMpal256Block = MALLOC0(mallocSize);
		VRAMpal256CLUT = (unsigned long *)((unsigned char *)VRAMpal256Block + (sizeof(unsigned short) * num256Cluts));
		VRAMpal256CRC = (unsigned long *)((unsigned char *)VRAMpal256CLUT + (sizeof(unsigned long) * num256Cluts));

/*
		VRAMpal256Block = MALLOC(sizeof(unsigned short) * num256Cluts);
		VRAMpal256CLUT = MALLOC(sizeof(unsigned long) * num256Cluts);
		VRAMpal256CRC = MALLOC(sizeof(unsigned long) * num256Cluts);
*/
		memset(VRAMpal256Block, 0, sizeof(unsigned short) * num256Cluts);
		memset(VRAMpal256CLUT, 0, sizeof(unsigned short) * num256Cluts);
		memset(VRAMpal256CRC, 0, sizeof(unsigned long) * num256Cluts);

		VRAM_256PALETTES = num256Cluts;
	}
	else
	{
		VRAM_256PALETTES = 0;
	}

	memset(VRAMblock, 0, sizeof(VRAMblock));
	
	rect.x = rect.y = 0;
	rect.w = 512;
	rect.h = 512;
	ClearImage(&rect,0,0,0);
	for(page=0; page<VRAM_PAGES; page++)
	{
		if ((page+page/VRAM_PAGECOLS) & 1)
		{
			r = g = b = 64;
		}
		else
		{
			r = g = b = 100;
		}
		rect.x = 512+(page%VRAM_PAGECOLS)*64;
		rect.y = (page/VRAM_PAGECOLS)*256;
		rect.w = 64;
		rect.h = 256;
	  	ClearImage(&rect,r,g,b);
		DrawSync(0);
	}

	// allocate some space for 256 colour cluts if needed
	if(num256Cluts)
	{
		textureInit256ClutSpace(num256Cluts);
	}

	// reset the palette counter
#ifdef _DEBUG
	palsused = 0;
#endif

}


void textureDestroy()
{

	if(VRAM_PALETTES)
	{
		FREE(VRAMpalBlock);
		//FREE(VRAMpalHandle);
		//FREE(VRAMpalCLUT);
		//FREE(VRAMpalCRC);
	}

	if(VRAM_256PALETTES)
	{
		FREE(VRAMpal256Block);
		//FREE(VRAMpal256CLUT);
		//FREE(VRAMpal256CRC);
	}

}


unsigned short textureAddCLUT16(unsigned short *palette)
{
	int				pal,col,hnd, r,g,b;
	RECT			rect;
	unsigned long	palCRC;

	for(col=0; col<16; col++)								// Mask out magenta + mark transparencies
	{
		r = (palette[col]>>10) & 31;
		g = (palette[col]>>5) & 31;
		b = palette[col] & 31;

		// close enough to magenta
		if ((r > 25) && (g < 5) && (b > 25))
			palette[col] = 0x0000;
		else
			palette[col] |= 0x8000;
	}

	palCRC = utilBlockCRC((char *)palette, (2 * 16));

   	for(pal=0; pal<VRAM_PALETTES; pal++)					// Search for matching palette
   	{
		if (VRAMpalBlock[pal])
		{
			if(palCRC == VRAMpalCRC[pal])	// Found match
			{
				VRAMpalBlock[pal]++;
				hnd = VRAMpalHandle[pal/32];
				return VRAMpalCLUT[pal];
			}
		}
	}

	for(pal=0; pal<VRAM_PALETTES; pal++)					// Search for new palette slot
	{
		if (VRAMpalBlock[pal]==0)
		{
			VRAMpalBlock[pal] = 1;
			hnd = VRAMpalHandle[pal/32];
			if (((pal & 31)==0) && (hnd==0))				// Is new area needed?
			{
				hnd = VRAMpalHandle[pal/32] = textureVRAMalloc(32,1);
//				VRAMblock[VRAM_GETPAGE(hnd)][VRAM_GETXY(hnd)+31] = 1;// THIS IS NOT NEEDED IF VRAM BORDERS ARE OFF
//				printf("New palette area needed - allocated 0x%x\n", hnd);
			}
			rect.x = VRAM_CALCVRAMX(hnd)+16*(pal & 3);		// Copy up the palette
			rect.y = VRAM_CALCVRAMY(hnd)+((pal/4) & 7);
  //			printf("Placing palette @ %d,%d\n", rect.x,rect.y);
			rect.w = 16;
			rect.h = 1;
			DrawSync(0);
			LoadImage(&rect, (unsigned long *)palette);
			DrawSync(0);
			VRAMpalCRC[pal] = palCRC;	// keep checksum of this palette
			VRAMpalCLUT[pal] = getClut(rect.x,rect.y);
#ifdef _DEBUG
			palsused++;
			//printf("Palette added: Currently used: %d\n", palsused);
#endif
			return VRAMpalCLUT[pal];
		}
	}
	printf("\ntexture: No more 16-colour palettes");
	return NULL;
}


void textureRemoveCLUT16(unsigned short clut)
{
	int	pal, area, page, found = 0;
	RECT	rect;

	for(pal=0; pal<VRAM_PALETTES; pal++)
	{
		if ((VRAMpalBlock[pal]) && (clut==VRAMpalCLUT[pal]))
		{
			if ((--VRAMpalBlock[pal])<=0)						// Reference count = 0, free palette
			{
#ifdef _DEBUG
				palsused--;
//				printf("Palette removed: Currently used: %d\n", palsused);
#endif
				VRAMpalBlock[pal] = 0;
				VRAMpalCLUT[pal] = 0;
//				printf("Freed palette #%d\n", pal);
				area = pal/32;									// Check other palettes in area
				found = 0;
				for(pal=area*32; pal<(area+1)*32; pal++)
					found |= VRAMpalBlock[pal];
				if (!found)
				{
//					printf("Freed palette area 0x%x\n", VRAMpalHandle[area]);
					textureVRAMfree(VRAMpalHandle[area],32,1);	// Free empty palette area
					rect.x = VRAM_CALCVRAMX(VRAMpalHandle[area]);
					rect.y = VRAM_CALCVRAMY(VRAMpalHandle[area]);
					rect.w = 64;
					rect.h = 8;
					page = VRAM_GETPAGE(VRAMpalHandle[area]);
					if ((page+page/VRAM_PAGECOLS) & 1)
						ClearImage(&rect, 64,64,64);
					else
						ClearImage(&rect, 100,100,100);

					VRAMpalHandle[area] = 0;
				}
				return;
			}
		}
	}
}


unsigned short textureAddCLUT256(unsigned short *palette)
{
	int				pal,col,r,g,b;
	RECT			rect;
	int				found;
	unsigned long	palCRC;

//#ifdef _DEBUG
//	utilPrintf("\nAdding 8-bit CLUT ...");
//#endif

	for(col = 0; col < 256; col ++)								// Mask out magenta + mark transparencies
	{
		r = (palette[col] >> 10) & 31;
		g = (palette[col] >> 5) & 31;
		b = palette[col] & 31;

		if ((r > 25) && (g < 5) && (b > 25))
			palette[col] = 0x0000;
		else
			palette[col] |= 0x8000;
	}

	palCRC = utilBlockCRC((char *)palette, (256 * 2));

	for(found = 0; found < VRAM_256PALETTES; found ++)
	{
		if(VRAMpal256CRC[found] == palCRC)
		{
			VRAMpal256Block[found] ++;

			return(VRAMpal256CLUT[found]);
		}
	}
	
	
	// CLUT wasn't found in the list

	for(pal=0; pal < VRAM_256PALETTES; pal++)					// Search for new palette slot
	{
		if (VRAMpal256Block[pal] == 0)
		{
			VRAMpal256Block[pal] = 1;

			rect.x = VRAM_STARTX;		// Copy up the palette
			rect.y = pal;
			rect.w = 256;
			rect.h = 1;
			LoadImage(&rect, (unsigned long *)palette);

			VRAMpal256CLUT[pal] = getClut(rect.x,rect.y);
			VRAMpal256Block[pal] = 1;
			VRAMpal256CRC[pal] = palCRC;

			return(VRAMpal256CLUT[pal]);
		}
	}

	utilPrintf("\nOut of 8 bit palettes!");

	return NULL;
}


int textureRemoveCLUT256(unsigned short clut)
{
	int pal;


//#ifdef _DEBUG
//	utilPrintf("Removing 8 bit palette ...\n");
//#endif

	for(pal = 0; pal < VRAM_256PALETTES; pal ++)
	{
		if(VRAMpal256CLUT[pal] == clut)
		{
			if((-- VRAMpal256Block[pal]) <= 0)
			{
//#ifdef _DEBUG
//				utilPrintf("Freeing it\n");
//#endif
				VRAMpal256Block[pal] = 0;
				VRAMpal256CLUT[pal] = 0;
				VRAMpal256CRC[pal] = 0;
				return 1;
			}
		}
	}

	return 0;
}


void textureDownLoad(NSPRITE *nspr, TextureType *txPtr)
{
	RECT			rect;
	unsigned short	ww,hh,uMax,vMax;
	int				handle, loop, loop2;
	unsigned long	imageCRC, imageLength, foundMatch;

	if(nspr->flags & NEIGHTBIT)
		txPtr->clut = textureAddCLUT256(nspr->pal);
	else
		txPtr->clut = textureAddCLUT16(nspr->pal);

	// set image CRC to zero for this texture
	txPtr->imageCRC = 0;

	// copy flags info to new texture
	txPtr->flags = nspr->flags;

	foundMatch = 0;

	// reference count is one to start with
	txPtr->refCount = 1;

	// calc width
	imageLength = (nspr->w+3)/4;

	// double if 256 colour
	if(nspr->flags & NEIGHTBIT)
		imageLength *= 2;

	// multiply by height and double (cuz its 16-bit)
	imageLength = (imageLength * nspr->h) * 2;

	// calc crc for texture
	imageCRC = utilBlockCRC((char *)nspr->image, imageLength);
	
	// for each texture bank
	/*for(loop = 0; loop < MAXTEXBANKS; loop ++)
	{
		if (texBank[loop]!=NULL)
		{
			// for all textures in bank
			for(loop2 = 0; loop2 < texBank[loop]->numTextures; loop2 ++)
			{
				// check to see if image crc matches
				if (texBank[loop]->texture[loop2].imageCRC == imageCRC)
				{
					// check width and height just to make sure
					if( (texBank[loop]->texture[loop2].w == nspr->w) && (texBank[loop]->texture[loop2].h == nspr->h) )
					{
						// copy stuff (except clut of course)
						if(!foundMatch)
						{
							txPtr->h = texBank[loop]->texture[loop2].h;
							txPtr->handle = texBank[loop]->texture[loop2].handle;
							txPtr->tpage = texBank[loop]->texture[loop2].tpage;
							txPtr->u0 = texBank[loop]->texture[loop2].u0;
							txPtr->u1 = texBank[loop]->texture[loop2].u1;
							txPtr->u2 = texBank[loop]->texture[loop2].u2;
							txPtr->u3 = texBank[loop]->texture[loop2].u3;
							txPtr->v0 = texBank[loop]->texture[loop2].v0;
							txPtr->v1 = texBank[loop]->texture[loop2].v1;
							txPtr->v2 = texBank[loop]->texture[loop2].v2;
							txPtr->v3 = texBank[loop]->texture[loop2].v3;
							txPtr->w = texBank[loop]->texture[loop2].w;
							txPtr->x = texBank[loop]->texture[loop2].x;
							txPtr->y = texBank[loop]->texture[loop2].y;
						}
						texBank[loop]->texture[loop2].refCount ++;
						txPtr->refCount = texBank[loop]->texture[loop2].refCount;	// increment reference count and assign
						foundMatch ++;
					}
				}
			}
		}
	}*/

	// if we've found matching textures, return
	if(foundMatch)
	{
		txPtr->imageCRC = imageCRC;
		return;
	}

	txPtr->refCount = 1;
	
	ww = (nspr->w+7)/8;
	hh = (nspr->h+7)/8;

	if(nspr->flags & NEIGHTBIT)
		ww*=2;

	handle = textureVRAMalloc(ww,hh);
	rect.x = VRAM_CALCVRAMX(handle);
	rect.y = VRAM_CALCVRAMY(handle);
	rect.w = (nspr->w+3)/4;
	rect.h = nspr->h;

	if(nspr->flags & NEIGHTBIT)
		rect.w *= 2;	

	DrawSync(0);
	LoadImage(&rect,((unsigned long*)nspr->image)+1); 
	
	txPtr->tpage = getTPage(
		((nspr->flags & NEIGHTBIT) ? 1 : 0),
		0, rect.x, rect.y);

	txPtr->x = VRAM_GETX(handle)*8;

	if(nspr->flags & NEIGHTBIT)
		txPtr->x = txPtr->x / 2;

	txPtr->y = rect.y;

	uMax = txPtr->x + nspr->w - 1;
	if (uMax > 255)
		uMax = 255;
	vMax = txPtr->y + nspr->h - 1;
	if (vMax > 255)
		vMax = 255;

	txPtr->w = nspr->w;
	txPtr->h = nspr->h;
   	txPtr->u0 = txPtr->x;
   	txPtr->v0 = txPtr->y;
   	txPtr->u1 = uMax;
   	txPtr->v1 = txPtr->y;
   	txPtr->u2 = txPtr->x;
   	txPtr->v2 = vMax;
   	txPtr->u3 = uMax;
   	txPtr->v3 = vMax;
	txPtr->handle = handle;
	txPtr->imageCRC = imageCRC;
}


/**************************************************************************
	FUNCTION:	textureUnload()
	PURPOSE:	Unload texture from VRAM
	PARAMETERS:	Sprite info ptr
	RETURNS:	
**************************************************************************/

void textureUnload(TextureType *txPtr)
{
	RECT	rect;
	int		page, loop, loop2;

	if(txPtr->refCount > 1)
	{
		//utilPrintf("\ntexture has multiple references, decrementing all instances\n");
		for(loop = 0; loop < MAXTEXBANKS; loop ++)
		{
			if (texBank[loop] != NULL)
			{
				// for all textures in bank
				for(loop2 = 0; loop2 < texBank[loop]->numTextures; loop2 ++)
				{
					// check to see if image crc matches
					if (texBank[loop]->texture[loop2].imageCRC == txPtr->imageCRC)
					{
						if( (texBank[loop]->texture[loop2].w == txPtr->w) && (texBank[loop]->texture[loop2].h == txPtr->h) )
						{
							// decrement reference count for all matching textures
							texBank[loop]->texture[loop2].refCount --;
						}
					}
				}
			}
		}

		// remove palette
		if(txPtr->tpage & (1 << 7))
			textureRemoveCLUT256(txPtr->clut);
		else
			textureRemoveCLUT16(txPtr->clut);

		return;
	}

	page = VRAM_GETPAGE(txPtr->handle);
	rect.x = VRAM_CALCVRAMX(txPtr->handle);
	rect.y = VRAM_CALCVRAMY(txPtr->handle);
	rect.w = 2*((txPtr->w+7)/8);
	rect.h = 8*((txPtr->h+7)/8);

	if(txPtr->tpage & (1 << 7))
		rect.w *= 2;

	if ((page+page/VRAM_PAGECOLS) & 1)
		ClearImage(&rect, 64,64,64);
	else
		ClearImage(&rect, 100,100,100);
	
	if(txPtr->tpage & (1 << 7))
		textureVRAMfree(txPtr->handle, ((txPtr->w+7)/8) * 2,(txPtr->h+7)/8);
	else
		textureVRAMfree(txPtr->handle, (txPtr->w+7)/8,(txPtr->h+7)/8);

	if(txPtr->tpage & (1 << 7))
		textureRemoveCLUT256(txPtr->clut);
	else
		textureRemoveCLUT16(txPtr->clut);
	
	DrawSync(0);
}



static int textureSetSPRPointers(NSPRITE *pHeader)
{
	int n = 0;

	while(1)
	{
		pHeader[n].pal = (unsigned short *)((unsigned long)pHeader+((unsigned long)pHeader[n].pal));
		pHeader[n].image = (unsigned char *)((unsigned long)pHeader+((unsigned long)pHeader[n].image));
		if(pHeader[n].flags & NLASTSPR)
			return(++n);
		n++;
	}
}


/**************************************************************************
	FUNCTION:	textureLoadBank()
	PURPOSE:	Load texture bank
	PARAMETERS:	Filename
	RETURNS:	Ptr to texture bank info
**************************************************************************/

extern unsigned char waterimage[];
int waterBankLoaded = 0;
TextureBankType *textureLoadBank(char *sFile)
{
 	TextureBankType	*newBank;
	int				loop;
	NSPRITE			*nspr;
	int				mallocSize, numTextures;
	unsigned long	bankCRC;

	// CRC bank name
	bankCRC = utilStr2CRC(sFile);

	// check to see if this bank is already loaded
	for(loop = 0; loop < MAXTEXBANKS; loop ++)
	{
		if(texBank[loop])
		{
			// if the CRC's match, this bank is already loaded
			if(texBank[loop]->bankCRC == bankCRC)
				return texBank[loop];
		}
	}

	if(*sFile)
	{
		nspr = (NSPRITE *)fileLoad(sFile, NULL);
		numTextures = textureSetSPRPointers(nspr);
	}
	else
	{
		nspr = (NSPRITE *)waterimage;
		if(waterBankLoaded == 0)
		{
			numTextures = textureSetSPRPointers(nspr);
			waterBankLoaded = 1;
		}
		else
			numTextures = 1;
	}

	
	mallocSize = (sizeof(TextureBankType)) + (numTextures*4+((numTextures+7)/8)) + (sizeof(TextureType)*numTextures);

	newBank = MALLOC0(mallocSize);

	for(loop=0; loop<MAXTEXBANKS; loop++)
	{
		if (texBank[loop]==NULL)
		{
			texBank[loop] = newBank;
			break;
		}
	}
	if (loop==MAXTEXBANKS)
		printf("**** WARNING: OUT OF TEXTURE BANKS\n");

	newBank->numTextures = numTextures;
	newBank->pNSprite = nspr;
	newBank->CRC = (unsigned long *)((unsigned char *)newBank + (sizeof(TextureBankType)));


	for(loop=0; loop<newBank->numTextures; loop++)
	{
		newBank->CRC [ loop ] = newBank->pNSprite [ loop ].crc;
	}

	newBank->texture = (TextureType *)((unsigned char *)newBank->CRC+newBank->numTextures*4);
	memset(newBank->texture, 0, sizeof(TextureType)*newBank->numTextures);
	
	newBank->used = (unsigned char *)((unsigned char *)newBank->texture + newBank->numTextures*sizeof(TextureType));
	memset(newBank->used, 0, ((newBank->numTextures+7)/8));

	newBank->bankCRC = bankCRC;

	return newBank;
}


/**************************************************************************
	FUNCTION:	textureDownloadFromBank()
	PURPOSE:	Upload/find given texture in bank
	PARAMETERS:	Texture bank info ptr, index of texture
	RETURNS:	Sprite info ptr
**************************************************************************/

TextureType *textureDownloadFromBank(TextureBankType *bank, int idx)
{
	long	b,y;
	NSPRITE	*pNSprite = bank->pNSprite;

	b = 1<<(idx & 7);
	y = idx>>3;
	if (!(bank->used[y] & b))
	{
		if (pNSprite!=NULL)
		{
			bank->used[y] |= b;
			textureDownLoad(pNSprite+idx, (bank->texture)+idx);
		}
		else
			return NULL;
	}
	return ((bank->texture)+idx);
}


/**************************************************************************
	FUNCTION:	textureDownloadBank()
	PURPOSE:	Download entire texture bank to VRAM
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureDownloadBank(TextureBankType *bank)
{
	int	loop;

	for(loop=0; loop<bank->numTextures; loop++)
	{
		textureDownloadFromBank(bank, loop);
	}
}


/**************************************************************************
	FUNCTION:	textureDestroyBank()
	PURPOSE:	Free system RAM data for given texture bank
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureDestroyBank(TextureBankType *bank)
{
	if (bank==NULL)
		return;

	FREE(bank->pNSprite);
	bank->pNSprite = NULL;
}


/**************************************************************************
	FUNCTION:	textureUnloadBank()
	PURPOSE:	Unload given texture bank from VRAM
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureUnloadBank(TextureBankType *bank)
{
	unsigned long	loop, y,b, numTex;
	
	if (bank==NULL)
		return;

	// free up texture space in vram
	numTex = bank->numTextures;
	for(loop=0; loop<numTex; loop++)
	{
		b = 1<<(loop & 7);
		y = loop>>3;
		if (bank->used[y] & b)
			textureUnload(bank->texture+loop);
	}

	// remove internal reference to bank

	for(loop=0; loop<MAXTEXBANKS; loop++)
		if (texBank[loop]==bank)
			texBank[loop] = NULL;

	// free up ram space
	//FREE(bank->texture);
	//FREE(bank->CRC);
	FREE(bank);
}


/**************************************************************************
	FUNCTION:	textureFindCRCInBank()
	PURPOSE:	Find given texture in bank
	PARAMETERS:	Texture bank info ptr, Texture name CRC
	RETURNS:	Texture info ptr
**************************************************************************/

TextureType *textureFindCRCInBank(TextureBankType *bank, unsigned long crc)
{
	unsigned long	numTextures = bank->numTextures;
	unsigned long	loop;

	for(loop=0; loop<numTextures; loop++)
	{
		if (bank->CRC[loop] == crc)
			return textureDownloadFromBank(bank, loop);
	}
	return NULL;
}


/**************************************************************************
	FUNCTION:	textureFindCRCInAllBanks()
	PURPOSE:	Find given texture in all loaded banks
	PARAMETERS:	Texture name CRC
	RETURNS:	Texture info ptr
**************************************************************************/

TextureType *textureFindCRCInAllBanks(unsigned long crc)
{
	int			loop;
	TextureType	*txPtr = NULL;

	for(loop=0; loop<MAXTEXBANKS; loop++)
	{
		if (texBank[loop]!=NULL)
		{
			txPtr = textureFindCRCInBank(texBank[loop], crc);
			if (txPtr!=NULL)
				return txPtr;
		}
	}
//	utilPrintf("Cannot find texture %d\n",crc);
	return txPtr;
}


/**************************************************************************
	FUNCTION:	textureReallocTextureBank()
	PURPOSE:	Reallocate texture in system RAM (e.g. to defragement)
	PARAMETERS:	Texture bank info ptr
	RETURNS:	New texture bank info ptr
**************************************************************************/

TextureBankType *textureReallocTextureBank(TextureBankType *txBank)
{
	TextureBankType	*textureBank;
	int				loop;
	TextureType		*txPtr;
	int				mallocSize;

	mallocSize = (sizeof(TextureBankType)) + (txBank->numTextures*4+((txBank->numTextures+7)/8)) + (sizeof(TextureType)*txBank->numTextures);

	textureBank = MALLOC0(mallocSize);	
	
	//textureBank = MALLOC(sizeof(TextureBankType));
	textureBank->numTextures = txBank->numTextures;

	textureBank->bankCRC = txBank->bankCRC;

	txPtr = (TextureType *)((unsigned char *)textureBank + sizeof(TextureBankType));
	
	//txPtr = MALLOC(sizeof(TextureType) * txBank->numTextures);
	memcpy(txPtr, txBank->texture, sizeof(TextureType)*txBank->numTextures);
	textureBank->texture = txPtr;

	textureBank->CRC = (unsigned long *)((unsigned char *)textureBank->texture + sizeof(TextureType)*txBank->numTextures);
	//textureBank->CRC = (unsigned long *)MALLOC(textureBank->numTextures*4+(textureBank->numTextures/8)+1);
	memcpy(textureBank->CRC, txBank->CRC, textureBank->numTextures*4+((textureBank->numTextures+7)/8));

	textureBank->pNSprite = txBank->pNSprite;

	textureBank->used = (unsigned char *)((unsigned char *)textureBank->CRC+textureBank->numTextures*4);
	memcpy(textureBank->used, txBank->used, ((textureBank->numTextures+7)/8));

	//FREE(txBank->texture);
	//FREE(txBank->CRC);
	FREE(txBank);

	for(loop = 0; loop < MAXTEXBANKS; loop ++)
	{
		if(texBank[loop] == txBank)
		{
			texBank[loop] = textureBank;
			break;
		}
	}

	return textureBank;
}


/**************************************************************************
	FUNCTION:	textureShowVRAM
	PURPOSE:	Debug VRAM viewer
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void VRAMviewNormal(DISPENV *dispenv, int *xOffs,int *yOffs, unsigned char palMode)
{
	if (padData.digital[0] & PAD_LEFT)
		*xOffs -= 6;
	if (padData.digital[0] & PAD_RIGHT)
		*xOffs += 6;
	if (*xOffs<0)
		*xOffs = 0;
	if (*xOffs>1024-512)
		*xOffs = 1024-512;
	if (padData.digital[0] & PAD_UP)
		*yOffs -= 4;
	if (padData.digital[0] & PAD_DOWN)
		*yOffs += 4;
	if (*yOffs<0)
		*yOffs = 0;
	if (*yOffs>512-((palMode)?(256):(240)))
		*yOffs = 512-((palMode)?(256):(240));
	dispenv->disp.x = *xOffs;
	dispenv->disp.y = *yOffs;
	if(palMode)
		dispenv->screen.y = 24;

	VSync(0);
	PutDispEnv(dispenv);
}

TextureType *textureFindTextureN(int n)
{
	int			bankLp, texLp, texCount = 0;

	for(bankLp=0; bankLp<MAXTEXBANKS; bankLp++)
	{
		if (texBank[bankLp]!=NULL)
		{
			for(texLp=0; texLp<texBank[bankLp]->numTextures; texLp++)
			{
				if (texBank[bankLp]->used[texLp>>3] & (1<<(texLp & 7)))
				{
					if (texCount==n)
						return texBank[bankLp]->texture+texLp;
					texCount++;
				}
			}
		}
	}
	return NULL;
}

static void VRAMdrawPalette(unsigned long clut, int y)
{
	POLY_F4		*f4;
	char		str[40];
	int			pal, loop;
	RECT		rect;

	rect.x = (clut & 0x3f) << 4;
	rect.y = (clut >> 6);
	rect.w = 16;
	rect.h = 1;

	DrawSync(0);
	StoreImage(&rect, currentPal16);
   	
	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	setXYWH(f4, -230-2,y-1, 16*20+3,10);
	setRGB0(f4, 128,128,128);
	ENDPRIM(f4, 11, POLY_F4);
	for(loop=0; loop<16; loop++)
	{
		BEGINPRIM(f4, POLY_F4);
		setPolyF4(f4);
		setXYWH(f4, -230+loop*20,y, 20,8);
		setRGB0(f4, 8*((currentPal16[loop]>>0) & 0x1f),
					8*((currentPal16[loop]>>5) & 0x1f),
					8*((currentPal16[loop]>>10) & 0x1f));
   		ENDPRIM(f4, 10, POLY_F4);
   	}

	for(pal=0; pal<VRAM_PALETTES; pal++)
   	{
   		if ((VRAMpalBlock[pal]) && (clut==VRAMpalCLUT[pal]))
   		{
			sprintf(str, "Palette #%d (used %dx)", pal, VRAMpalBlock[pal]);
   			fontPrintScaled(font, -230,y+13, str, 128,128,128,4096);
			break;
		}
	}

}

static void VRAMdrawPalette256(unsigned long clut, int y)
{
	POLY_F4			*f4;
	char			str[40];
	int				pal, loop, loop2;
	RECT			rect;
	unsigned short	*palPtr;

	rect.x = (clut & 0x3f) << 4;
	rect.y = (clut >> 6);
	rect.w = 256;
	rect.h = 1;

	DrawSync(0);
	StoreImage(&rect, currentPal256);
   	
	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	setXYWH(f4, -230-2,y-1, 16*20+3,10);
	setRGB0(f4, 128,128,128);
	ENDPRIM(f4, 11, POLY_F4);

	palPtr = &currentPal256[0];

	for(loop = 0; loop < 4; loop ++)
	{
		for(loop2 = 0; loop2 < 64; loop2 ++)
		{
			BEGINPRIM(f4, POLY_F4);
			setPolyF4(f4);
			setXYWH(f4, -230+loop2*5,y, 5, 2);
			setRGB0(f4, 8*(((*palPtr)>>0) & 0x1f),
						8*(((*palPtr)>>5) & 0x1f),
						8*(((*palPtr)>>10) & 0x1f));
   			ENDPRIM(f4, 10, POLY_F4);
			palPtr ++;
   		}
		y += 2;
	}

	for(pal=0; pal<VRAM_256PALETTES; pal++)
   	{
   		if ((VRAMpal256Block[pal]) && (clut==VRAMpal256CLUT[pal]))
   		{
			sprintf(str, "256Palette #%d (used %dx)", pal, VRAMpal256Block[pal]);
   			fontPrintScaled(font, -230,y+5, str, 128,128,128,4096);
			break;
		}
	}
}

static void VRAMviewTextures(int *currTex)
{
	POLY_FT4 	*ft4;
	TextureType	*tex;
	char		str[40];
	int			loop, yu = 0, yd = 0, f;

	static int	padDelay = 0;

	currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
	ClearOTagR(currentDisplayPage->ot, 1024);
	currentDisplayPage->primPtr = currentDisplayPage->primBuffer;

	sprintf(str, "VRAM: TEXTURE VIEW");
	fontPrintScaled(font, -230,-110, str, 128,128,128,4096);

	tex = textureFindTextureN(*currTex);
	if (tex!=NULL)
	{
		BEGINPRIM(ft4, POLY_FT4);
		setPolyFT4(ft4);
		setXYWH(ft4, 150-tex->w/2,-tex->h/2, tex->w,tex->h-1);
		setRGB0(ft4, 128,128,128);
		setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
		ft4->tpage = tex->tpage;
		ft4->clut = tex->clut;
		ENDPRIM(ft4, 10, POLY_FT4);

		BEGINPRIM(ft4, POLY_FT4);
		setPolyFT4(ft4);
		setXYWH(ft4, -128-tex->w,-tex->h, tex->w*3,tex->h*2);
		setRGB0(ft4, 128,128,128);
		setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
		ft4->tpage = tex->tpage;
		ft4->clut = tex->clut;
		ENDPRIM(ft4, 11, POLY_FT4);

		if(tex->tpage & (1 << 7))
			VRAMdrawPalette256(tex->clut, -90);
		else
			VRAMdrawPalette(tex->clut, -90);

		sprintf(str, "Texture #%d (%dx%d) (used %dx)", *currTex, tex->w,tex->h, tex->refCount);
		fontPrintScaled(font, -fontExtentWScaled(font, str,4096)/2,75, str, 128,128,128,4096);
		yu = -tex->h/2-8;
		yd = tex->h/2+8;
	}

	for(loop=1; loop<5; loop++)
	{
		tex = textureFindTextureN(*currTex-loop);
		f = 100-loop*20;
		if (tex!=NULL)
		{
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			setXYWH(ft4, 150-tex->w/2,yu-tex->h, tex->w,tex->h-1);
			setRGB0(ft4, f,f,f);
			setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
			ft4->tpage = tex->tpage;
			ft4->clut = tex->clut;
			ENDPRIM(ft4, 12, POLY_FT4);
			yu -= tex->h+8;
		}
		tex = textureFindTextureN(*currTex+loop);
		if (tex!=NULL)
		{
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			setXYWH(ft4, 150-tex->w/2,yd, tex->w,tex->h-1);
			setRGB0(ft4, f,f,f);
			setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
			ft4->tpage = tex->tpage;
			ft4->clut = tex->clut;
			ENDPRIM(ft4, 12, POLY_FT4);
			yd += tex->h+8;
		}
	}

	if (padDelay==0)
	{
		if ((padData.digital[0] & (PAD_LEFT|PAD_UP)) && (*currTex>0))
		{
			*currTex = (*currTex)-1;
			padDelay = 3;
			if (padData.debounce[0] & (PAD_LEFT|PAD_UP))
				padDelay = 20;
		}
		if (padData.digital[0] & (PAD_RIGHT|PAD_DOWN))
		{
			if(textureFindTextureN(*currTex+1))
			{
				*currTex = (*currTex)+1;
				padDelay = 3;
				if (padData.debounce[0] & (PAD_RIGHT|PAD_DOWN))
					padDelay = 20;
			}
		}
	}
	else
	{
		if ((padData.digital[0] & (PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN))==0)
			padDelay = 0;
		if (padDelay>0)
			padDelay--;
	}

	DrawSync(0);
	VSync(0);
	PutDispEnv(&currentDisplayPage->dispenv);
	PutDrawEnv(&currentDisplayPage->drawenv);
	DrawOTag(currentDisplayPage->ot+(1024-1));
}

void textureShowVRAM(unsigned char palMode)
{
	DISPENV		dispenv;
	int			xOffs,yOffs, viewMode, currTex;

	currTex = 0;
	xOffs = 512;
	yOffs = 0;
	viewMode = 0;
	SetDefDispEnv(&dispenv, 0,0, 512,256);
	while ((padData.digital[0] & PAD_START)==0)
	{
		myPadHandleInput();
		switch(viewMode)
		{
		case 0:
			VRAMviewNormal(&dispenv, &xOffs,&yOffs, palMode);
			break;
		case 1:
			VRAMviewTextures(&currTex);
			break;
		}
		if (padData.debounce[0] & PAD_SELECT)
			viewMode = (viewMode+1) % 2;
	}
}


/**************************************************************************
	FUNCTION:	textureCreateAnimation
	PURPOSE:	Create an animated texture
	PARAMETERS:	pointer to dummy texture, pointer to array of animated textures, number of frames
	RETURNS:	pointer to animated texture
**************************************************************************/

TextureAnimType *textureCreateAnimation(TextureType *dummy, TextureType **anim, int numFrames)
{
	TextureAnimType	*texAnim;
	int				loop;
	int				mallocSize;

	mallocSize = sizeof(TextureAnimType) + (sizeof(TextureType *) * numFrames);

	texAnim = MALLOC0(mallocSize);

	texAnim->dest = dummy;

	texAnim->anim = (TextureType **)((unsigned char *)texAnim + sizeof(TextureAnimType));

	for(loop = 0; loop < numFrames; loop ++)
	{
		texAnim->anim[loop] = anim[loop];
	}

	return texAnim;
}


/**************************************************************************
	FUNCTION:	textureSetAnimation
	PURPOSE:	Set frame of an animated texture
	PARAMETERS:	Animated texture, frame number
	RETURNS:	
**************************************************************************/

void textureSetAnimation(TextureAnimType *texAnim, int frameNum)
{
	DR_MOVE *siMove;
	RECT	moveRect;

	moveRect.x = VRAM_CALCVRAMX(texAnim->anim[frameNum]->handle);
	moveRect.y = VRAM_CALCVRAMY(texAnim->anim[frameNum]->handle);
	moveRect.w = (texAnim->dest->w + 3) / 4;
	moveRect.h = texAnim->dest->h;

	// check for 256 colour mode
	if(texAnim->dest->tpage & (1 << 7))
		moveRect.w *= 2;

	// copy bit of vram
	BEGINPRIM(siMove, DR_MOVE);
	SetDrawMove(siMove, &moveRect, VRAM_CALCVRAMX(texAnim->dest->handle),VRAM_CALCVRAMY(texAnim->dest->handle));
	ENDPRIM(siMove, 1023, DR_MOVE);
}


/**************************************************************************
	FUNCTION:	textureDestroyAnimation
	PURPOSE:	Destroy an animated texture
	PARAMETERS:	Animated texture
	RETURNS:	
**************************************************************************/

void textureDestroyAnimation(TextureAnimType *texAnim)
{
	FREE(texAnim);
}
