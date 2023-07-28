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
#define _LOADPAL_PRINT_OUT_PALETTE

#include "Ultra64.h"
#include <stdio.h>

#include "isltex.h"
#include <islutil.h>
#include <islmem.h>

#include "LayOut.h"
#include "Textures.h"

#include "Main.h"
#include "memload.h"
#include "shell.h"
#include "flatpack.h"

int numTextureBanks = 0;
int numUsedDummys		= 0;
int numUsedDummys64 = 0;
int numUsedDummys16 = 0;


//char *palNames [ 5 ] = { "BFG01", "BFG02", "BFG03", "BFG04", "BFG05"};
char *palNames [ 5 ] = { "GREENBABY.PAL", "YELLOWBABY.PAL", "BLUEBABY.PAL", "PINKBABY.PAL", "REDBABY.PAL"};

TextureBankType *textureBanks [ MAX_TEXTURE_BANKS ] = { NULL, NULL, NULL, NULL, NULL };



TEXTUREANIMLIST textureAnimList;


TextureType *CreateSpareTextureSpace ( unsigned long dummyCrc );


/*	--------------------------------------------------------------------------------
	Function		: LoadTextureBank
	Purpose			: Set of textures
	Parameters		: 
	Returns			: void
	Info			: 
*/
void LoadTextureBank ( int textureBank )
{
	char fileName[256];

	switch ( textureBank )
	{
		case GARDEN_TEX_BANK: sprintf ( fileName, "TEXTURES\\GARDEN.SPT" );	break;
		case ANCIENT_TEX_BANK: sprintf ( fileName, "TEXTURES\\ANCIENTS.SPT" ); break;
		case SPACE_TEX_BANK: sprintf ( fileName, "TEXTURES\\SPACE.SPT" ); break;
		case CITY_TEX_BANK: sprintf ( fileName, "TEXTURES\\CITY.SPT" ); break;
		case SUBTERRANEAN_TEX_BANK: sprintf ( fileName, "TEXTURES\\SUB.SPT" ); break;
		case LABORATORY_TEX_BANK: sprintf ( fileName, "TEXTURES\\LAB.SPT" ); break;
		case HALLOWEEN_TEX_BANK: sprintf ( fileName, "TEXTURES\\HALLOWEEN.SPT" ); break;
		case SUPERRETRO_TEX_BANK: sprintf ( fileName, "TEXTURES\\SUPER.SPT" ); break;
		case FRONTEND_TEX_BANK: sprintf ( fileName, "TEXTURES\\HUB.SPT" ); break;
		case INGAMEGENERIC_TEX_BANK: sprintf ( fileName, "TEXTURES\\GENERIC.SPT" ); break;
		case TITLES_TEX_BANK: sprintf ( fileName, "TEXTURES\\TITLES.SPT" ); break;
		case LOADING_TEX_BANK: fileName[0] = 0; break;
	}

	utilPrintf("FileName : %s\n", fileName );

	if ( numTextureBanks < MAX_TEXTURE_BANKS )
	{
		textureBanks [ numTextureBanks ] = textureLoadBank ( fileName );

		textureDownloadBank ( textureBanks [ numTextureBanks ]   );
		textureDestroyBank  ( textureBanks [ numTextureBanks++ ] );
	}
	else
	{
		utilPrintf("Error Loading: %s, Max Texture Banks Reached", fileName );
	}
}

/*	--------------------------------------------------------------------------------
	Function		: LoadTextureAnimBank
	Purpose			: Animating textures from .tit file
	Parameters		: 
	Returns			: void
	Info			: Hur hur - you said tit :)
*/
void LoadTextureAnimBank( int textureBank )
{
	TextureType *dummyTexture;
	int fileLength, counter, counter1, numframes, waitTime;
	unsigned long crc, destCrc, dummyCrc;
	char dummyString [ 256 ];
	char titFileName[256];
	unsigned long *p, *textureAnims;

	TEXTUREANIM *textureAnim;
	short numAnimations = 0;

	switch ( textureBank )
	{
		case GARDEN_TEX_BANK: sprintf ( titFileName, "GARDEN.TIT" ); break;
		case ANCIENT_TEX_BANK: sprintf ( titFileName, "ANCIENTS.TIT" ); break;
		case SPACE_TEX_BANK: sprintf ( titFileName, "SPACE.TIT" ); break;
		case SUBTERRANEAN_TEX_BANK: sprintf ( titFileName, "SUB.TIT" ); break;
		case LABORATORY_TEX_BANK: sprintf ( titFileName, "LAB.TIT" ); break;
		case HALLOWEEN_TEX_BANK: sprintf ( titFileName, "HALLOWEEN.TIT" ); break;
		case SUPERRETRO_TEX_BANK: sprintf ( titFileName, "RETRO.TIT" ); break;
		case FRONTEND_TEX_BANK: sprintf ( titFileName, "HUB.TIT" ); break;
		case INGAMEGENERIC_TEX_BANK: sprintf ( titFileName, "GENERIC.TIT" ); break;
		default: return;
	}

	textureAnims = FindStakFileInAllBanks ( titFileName, &fileLength );

	if ( !textureAnims )
		return;
	
	p = textureAnims;
	numAnimations = *p; p++;

	for ( counter = 0; counter < numAnimations; counter++ )
	{
		RECT moveRect;

		numframes = *p;	p++;
		crc = *p; p++;

		destCrc = crc;

		textureAnim = CreateTextureAnimation( crc, numframes );

		if ( textureAnim->animation->dest->h == 64 )
			sprintf ( dummyString, "64DUMMY%d", numUsedDummys64++ );
		else if ( textureAnim->animation->dest->h == 16 )
			sprintf ( dummyString, "16DUMMY%d", numUsedDummys16++ );
		else
			sprintf ( dummyString, "DUMMY%d", numUsedDummys++ );

		dummyCrc = utilStr2CRC ( dummyString );
		dummyTexture = textureFindCRCInAllBanks ( dummyCrc );

		if ( !dummyTexture )
			utilPrintf("Cound Not Find Dummy Texture....: %s\n", dummyString);

		moveRect.x = VRAM_CALCVRAMX(textureAnim->animation->dest->handle);
		moveRect.y = VRAM_CALCVRAMY(textureAnim->animation->dest->handle);
		moveRect.w = (textureAnim->animation->dest->w + 3) / 4;
		moveRect.h = textureAnim->animation->dest->h;

		if ( textureAnim->animation->dest->tpage & (1 << 7) )
			moveRect.w <<= 1;

		if(dummyTexture)
		{
			DrawSync(0);
			MoveImage ( &moveRect, VRAM_CALCVRAMX(dummyTexture->handle), VRAM_CALCVRAMY(dummyTexture->handle) );
 			DrawSync(0);
		}

		/*moveRect.x = (textureAnim->animation->dest->clut & 0x3f) << 4;		// Copy up the palette
		moveRect.y = (textureAnim->animation->dest->clut >> 6);

		if ( dummyTexture->tpage & (1 << 7) )
			moveRect.w = 256;
		else		
			moveRect.w = 16;

		moveRect.h = 1;

		DrawSync(0);
		MoveImage ( &moveRect, (dummyTexture->clut & 0x3f) << 4, (dummyTexture->clut >> 6) );
 		DrawSync(0);*/

		//BEGINPRIM		( siMove, DR_MOVE );
		//SetDrawMove ( siMove, &moveRect, (dummyTexture->clut & 0x3f) << 4, (dummyTexture->clut >> 6) );
		//ENDPRIM			( siMove, 1023, DR_MOVE );

		for ( counter1 = 0; counter1 < numframes; counter1++ )
		{
			crc = *p;
			p++;
			waitTime = *p; p++;

			if ( crc == destCrc )
				AddAnimFrame ( textureAnim, dummyCrc, waitTime, counter1 );
			else
				AddAnimFrame ( textureAnim, crc, waitTime, counter1 );
		}
	}
}

/*	--------------------------------------------------------------------------------
	Function		: FreeTextureBank
	Purpose			: Free textures
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeTextureBank ( TextureBankType *textureBank )
{
	textureDestroyBank	( textureBank );
	textureUnloadBank		( textureBank );
}


/*	--------------------------------------------------------------------------------
	Function		: FreeAllTextureBanks
	Purpose			: Free all textures
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeAllTextureBanks ( void )
{
	short c;

	utilPrintf("Freeing All Texture Banks : TEXTURE\n");

	for ( c = 0; c < MAX_TEXTURE_BANKS; c++ )
	{
		if ( textureBanks[c] )
			FreeTextureBank ( textureBanks [c] );

		textureBanks[c] = NULL;
	}

	numTextureBanks = 0;
	numUsedDummys		= 0;
	numUsedDummys64	= 0;
	numUsedDummys16	= 0;

	textureDestroy();
}





static int LOADPAL_LoadPCPalette(unsigned char *palette, LOADPAL_PCPalette16Str * const pc_palette);
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
signed short LOADPAL_Load16(unsigned char *palette)
{
	signed short psx_palette[16];
	LOADPAL_PCPalette16Str pc_palette;
	int clut;

	// Load the palette into 24bit rgb components
	if (!LOADPAL_LoadPCPalette(palette, &pc_palette))
		return 0;

	// Now convert the pc palette to the playstation version
	{
		int i;
		LOADPAL_24BitRGBStr *element;
		for (i=0;i<16;i++)
		{
			element = &pc_palette.component[i];

#ifdef LOADPAL_PRINT_OUT_PALETTE
			utilPrintf("pc rgb = %d, %d, %d\n", element->r, element->g, element->b);
#endif
			// Divide each element by 8 to get in the range 0-31
			// Remember the psx holds elements in the order b,g,r
			psx_palette[i] = ((element->r/8) << 0) +
							 ((element->g/8) << 5) +
							 ((element->b/8) << 10);
		}
	}

	// Finally try and load this PSX palette into VRAM
	clut = textureAddCLUT16(&psx_palette[0]);

	return clut;
}
//////////////////////////////////////////////////////////////////
//					Internal functions							//
//////////////////////////////////////////////////////////////////
static int LOADPAL_LoadPCPalette(unsigned char *palette, 
							   LOADPAL_PCPalette16Str * const pc_palette)
{
	//unsigned char *palette = NULL;
	unsigned char *file_ptr;
	int file_length;
	int i;

/*	if (!file)
	{
		utilPrintf("ERROR: File address passed to palette load is NULL.\n");
		utilPrintf("Palette not loaded.\n");
		return 0;
	}*/

	//palette = (unsigned char*)fileLoad(file, &file_length);

/*	if (!palette)
	{
//		utilPrintf("ERROR: Could not load palette file %s.\n", file);
//		utilPrintf("Palette not loaded.\n");
		return 0;
	}*/

	// Let's just check the 1st 3 letters for the right format
	if ( (palette[0] != 'J') ||
		 (palette[1] != 'A') ||
		 (palette[2] != 'S') )
	{
		utilPrintf("ERROR: Palette file is not in JASC format.\n");
		utilPrintf("Use Paint Shop Pro or Batchmaster.\n");
		utilPrintf("Palette not loaded.\n");
		return 0;
	}

	// go past the string and version number
	file_ptr = palette + 16;

	// Find out how many colours there are
	{
		unsigned char nb;			// next byte
		int nc;						// number of colours

		nb = *file_ptr;
		nc = nb - 0x30;
		file_ptr++;
		nb = *file_ptr;
		// Keep on readin in bytes until return is found
		while (nb != 0x0D)
		{
			nc *= 10;
			nc += (nb - 0x30);
			file_ptr++;
			nb = *file_ptr;
		}
		// If the number of colours is not equal to 16 throw an error
		if (nc != 16)
		{
			utilPrintf("ERROR: Palette has %d instead of 16 colours.\n");
			utilPrintf("Palette not loaded.\n");
			return 0;
		}
	}

	// increment again past the newline
	file_ptr+=2;

	// The components should be next
	for (i=0;i<16;i++)
	{
		unsigned char nb;		// next byte
		unsigned char r,g,b;

		// initialise these vars
		// get the r component (separated by a space from the green)
		nb = *file_ptr;
		r = (nb - 0x30);
		file_ptr++;
		nb = *file_ptr;
		while (nb != 0x20)
		{
			r *= 10;
			r += (nb - 0x30);
			file_ptr++;
			nb = *file_ptr;
		}
		// Go past the space
		file_ptr++;
		// get the g component (separated by a space from the green)
		nb = *file_ptr;
		g = (nb - 0x30);
		file_ptr++;
		nb = *file_ptr;
		while (nb != 0x20)
		{
			g *= 10;
			g += (nb - 0x30);
			file_ptr++;
			nb = *file_ptr;
		}
		// Go past the space
		file_ptr++;
		// get the b component (separated by a return and newline)
		nb = *file_ptr;
		b = (nb - 0x30);
		file_ptr++;
		nb = *file_ptr;
		while (nb != 0x0D)
		{
			b *= 10;
			b += (nb - 0x30);
			file_ptr++;
			nb = *file_ptr;
		}
		// Now get past the newline
		file_ptr += 2;

		// Finally store in the pc palette
		pc_palette->component[i].r = r;
		pc_palette->component[i].g = g;
		pc_palette->component[i].b = b;
	}

	//FREE(palette);

	return 1;
}
//////////////////////////////////////////////////////////////////


/*	--------------------------------------------------------------------------------
	Function		: CreateTextureAnimation
	Purpose			: 
	Parameters		: 
	Returns			: void
	Info			: 
*/
TEXTUREANIM *CreateTextureAnimation ( unsigned long crc, int numframes )
{
	TEXTUREANIM *textureAnim = NULL;

	textureAnim = MALLOC0 ( sizeof ( TEXTUREANIM ) );

	if ( !textureAnim )
	{
		utilPrintf ( "Could Not Malloc Texture Anim.\n" );
		return NULL;
	}

	AddTextureAnim ( textureAnim );

	textureAnim->numFrames = numframes;
	textureAnim->frame = 0;
	textureAnim->lastTime = 0;
	textureAnim->animation = MALLOC0 ( sizeof(TextureAnimType) + (sizeof(TextureType *) * textureAnim->numFrames ) );

	if ( !textureAnim->animation )
	{
		utilPrintf ( "Could Not Malloc Animation. %d\n",  textureAnim->numFrames );
		FREE ( textureAnim );
		return NULL;
	}

	textureAnim->animation->dest = textureFindCRCInAllBanks ( crc );
	textureAnim->animation->anim = (TextureType **)((unsigned char *)textureAnim->animation + sizeof(TextureAnimType));
	textureAnim->animation->waitTimes = (short *)MALLOC0( sizeof(short)*numframes );

	return textureAnim;
}


/*	--------------------------------------------------------------------------------
	Function		: AddAnimFrame
	Purpose			: Add a frame to an animating texture
	Parameters		: 
	Returns			: void
	Info			: 
*/
void AddAnimFrame( TEXTUREANIM *anim, unsigned long crc, short waitTime, int num )
{
	anim->animation->waitTimes[num] = waitTime;
	anim->animation->anim[num] = textureFindCRCInAllBanks( crc );
	if ( !anim->animation->anim[num] )
		utilPrintf("Could Not Find Texture For Anim: %lu\n", crc );
}


/*	--------------------------------------------------------------------------------
	Function		: UpdateTextureAnimations
	Purpose			: Advance frames if necessary
	Parameters		: 
	Returns			: void
	Info			: 
*/
void UpdateTextureAnimations( void )
{
	TEXTUREANIM *cur;

	if ( textureAnimList.numEntries == 0 )
		return;

	for ( cur = textureAnimList.head.next; cur != &textureAnimList.head; cur = cur->next )
	{
		if( cur->numFrames > 1 )
		{
			// If special waitTime then advance frame by 1
			if( cur->animation->waitTimes[cur->frame] == -1 )
			{
				cur->lastTime++;

				cur->frame++;
				if( cur->frame >= cur->numFrames )
					cur->frame = 0;
			}
			else
			{
				while( cur->lastTime + cur->animation->waitTimes[cur->frame] < actFrameCount )
				{
					cur->lastTime += cur->animation->waitTimes[cur->frame];
					
					cur->frame++;
					if( cur->frame >= cur->numFrames )
						cur->frame = 0;
				}
			}

			// JH: Copy the required texture into vram.
			CopyTexture ( cur->animation->dest, cur->animation->anim[cur->frame], 0 );
		}
	}
}


/*	--------------------------------------------------------------------------------
	Function		: Boring list funcs
	Purpose			: 
	Parameters		: 
	Returns			: void
	Info			: 
*/
void InitTextureAnimLinkedList ( void )
{
	textureAnimList.numEntries = 0;
	textureAnimList.head.next = textureAnimList.head.prev = &textureAnimList.head;
}

void AddTextureAnim ( TEXTUREANIM *textureAnim )
{
	if ( textureAnim->next == NULL )
	{
		textureAnimList.numEntries++;
		textureAnim->prev								= &textureAnimList.head;
		textureAnim->next								= textureAnimList.head.next;
		textureAnimList.head.next->prev	= textureAnim;
		textureAnimList.head.next				=	textureAnim;
	}
}

void FreeTextureAnimList ( void )
{
	TEXTUREANIM *cur,*next;

	if( !textureAnimList.numEntries )
		return;

	for ( cur = textureAnimList.head.next; cur != &textureAnimList.head; cur = next )
	{
		next = cur->next;
		SubTextureAnim(cur);
	}
}


void SubTextureAnim ( TEXTUREANIM *textureAnim )
{
	if(textureAnim->next == NULL)
		return;

	textureAnim->prev->next = textureAnim->next;
	textureAnim->next->prev = textureAnim->prev;
	textureAnim->next = NULL;
	textureAnimList.numEntries--;

	//FREE(textureAnim);
	textureAnim = NULL;
}


/*	--------------------------------------------------------------------------------
	Function		: CreateSpareTextureSurface
	Purpose			: Make dummy surface in vram to copy ani textures to
	Parameters		: 
	Returns			: void
	Info			: 
*/
TextureType *CreateSpareTextureSpace ( unsigned long dummyCrc )
{
	RECT			rect;
	DR_MOVE *siMove;
	RECT	moveRect;

	TextureType *texture, *dummy;

	dummy = ( TextureType* ) MALLOC0 ( sizeof ( TextureType ) );

	texture = textureFindCRCInAllBanks ( dummyCrc );

	if ( !texture )
	{
		utilPrintf ("Could Not Find Replacement Texture.....\n");
		return NULL;
	}

	dummy->handle = textureVRAMalloc ( texture->w, texture->h );

	if ( !dummy->handle )
	{
		utilPrintf("Could Not VRAM Alloc : Width : %d : Height : %d\n", texture->w, texture->h );
		return NULL;
	}

	rect.x = VRAM_CALCVRAMX(dummy->handle);
	rect.y = VRAM_CALCVRAMY(dummy->handle);
	rect.w = (texture->w+3)/4;
	rect.h = texture->h;

	dummy->x = VRAM_GETX ( dummy->handle ) * 8;
	dummy->y = rect.y;

	dummy->w = texture->w;
	dummy->h = texture->h;

	dummy->u0 = texture->x;
	dummy->v0 = texture->y;
	dummy->u1 = 255;
	dummy->v1 = texture->y;
	dummy->u2 = texture->x;
	dummy->v2 = 255;
	dummy->u3 = 255;
	dummy->v3 = 255;

	dummy->imageCRC = dummyCrc;

	dummy->tpage = getTPage(
		((texture->flags & NEIGHTBIT) ? 1 : 0),
		0, rect.x, rect.y);

	moveRect.x = VRAM_CALCVRAMX(dummy->handle);
	moveRect.y = VRAM_CALCVRAMY(dummy->handle);
	moveRect.w = dummy->w;
	moveRect.h = dummy->h;

	// check for 256 colour mode
	if(dummy->tpage & (1 << 7))
		moveRect.w *= 2;

	BEGINPRIM		( siMove, DR_MOVE );
	SetDrawMove	( siMove, &moveRect, VRAM_CALCVRAMX(texture->handle),VRAM_CALCVRAMY(texture->handle));
	ENDPRIM			( siMove, 1023, DR_MOVE );


	return dummy;
}


/*	--------------------------------------------------------------------------------
	Function		: CopyTexture
	Purpose			: Copy vram->vram
	Parameters		: 
	Returns			: void
	Info			: 
*/
void CopyTexture ( TextureType *dest, TextureType *src, int copyPalette )
{
	DR_MOVE *siMove;
	RECT	moveRect;

	if((dest == NULL) || (src == NULL))
		return;

	moveRect.x = VRAM_CALCVRAMX(src->handle);
	moveRect.y = VRAM_CALCVRAMY(src->handle);
	moveRect.w = (dest->w + 3)/4;
	moveRect.h = dest->h;

	// check for 256 colour mode
	if ( dest->tpage & (1 << 7) )
		moveRect.w <<= 1;

//	dest->tpage = src->tpage;
	//dest->clut = src->clut;

	// copy bit of vram
	BEGINPRIM		( siMove, DR_MOVE );
	SetDrawMove ( siMove, &moveRect, VRAM_CALCVRAMX(dest->handle), VRAM_CALCVRAMY(dest->handle) );
	ENDPRIM			( siMove, 1023, DR_MOVE );

	if ( copyPalette )
	{
		moveRect.x = (src->clut & 0x3f) << 4;		// Copy up the palette
		moveRect.y = (src->clut >> 6);

		if ( dest->tpage & (1 << 7) )
			moveRect.w = 256;
		else		
			moveRect.w = 16;

		moveRect.h = 1;

		BEGINPRIM		( siMove, DR_MOVE );
		SetDrawMove ( siMove, &moveRect, (dest->clut & 0x3f) << 4, (dest->clut >> 6) );
		ENDPRIM			( siMove, 1023, DR_MOVE );
	}
}
