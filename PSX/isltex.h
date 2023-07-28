/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	isltex.h			Texture and VRAM management

************************************************************************************/

#ifndef __ISLTEX_H__
#define __ISLTEX_H__

// NSPRITE flags
#define NLASTSPR	1 
#define NEIGHTBIT	2
#define NSPLIT		4
#define NBITSTREAM	8
#define NALPHA		16
#define NCOLOURKEY	32

#define MAXTEXBANKS			50


// single texture structure
typedef struct {
	unsigned char		x;
	unsigned char		y;
	unsigned char		w;
	unsigned char		h;
	unsigned short		tpage;
	unsigned short		clut;
	unsigned char		u0, v0;
	unsigned char		u1, v1;
	unsigned char		u2, v2;
	unsigned char		u3, v3;
	int					handle;
	unsigned long		imageCRC;
	unsigned short		refCount;
	unsigned short		flags;
} TextureType;

// animated texture structure
typedef struct _TextureAnimType
{
	short *waitTimes;
	TextureType		*dest;	// destination texture
	TextureType		**anim;	// source textures

} TextureAnimType;

// single texture as stored within the SPT file
typedef struct tagNSPRITE {
	unsigned char	*image;		// image data
	unsigned short	*pal;		// palette data
	unsigned char	w;			// width
	unsigned char	h;			// height
	short			u;    
	short			v;     
	unsigned short	flags;
	unsigned long	crc;
} NSPRITE;

// texture bank structure
typedef struct _TextureBankType {
	unsigned short		numTextures;
	NSPRITE				*pNSprite;
	TextureType			*texture;
	unsigned char		*used;
	unsigned long		*CRC;
	unsigned long		bankCRC;
} TextureBankType;



/**************************************************************************
	FUNCTION:	textureInitialise()
	PURPOSE:	Initialise VRAM/texture handling
	PARAMETERS:	Number of 16 & 256 colour cluts to allocate
	RETURNS:	
**************************************************************************/

void textureInitialise(int num16Cluts, int num256Cluts);


/**************************************************************************
	FUNCTION:	textureDestroy()
	PURPOSE:	Free up memory used by library
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void textureDestroy();


/**************************************************************************
	FUNCTION:	textureClearVRAM()
	PURPOSE:	Clear VRAM
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void textureClearVRAM(long screenwidth);


/**************************************************************************
	FUNCTION:	textureUnload()
	PURPOSE:	Unload texture from VRAM
	PARAMETERS:	Sprite info ptr
	RETURNS:	
**************************************************************************/

void textureUnload(TextureType *txPtr);


/**************************************************************************
	FUNCTION:	textureLoadBank()
	PURPOSE:	Load texture bank
	PARAMETERS:	Filename
	RETURNS:	Ptr to texture bank info
**************************************************************************/

TextureBankType *textureLoadBank(char *sFile);


/**************************************************************************
	FUNCTION:	textureDownloadBank()
	PURPOSE:	Download entire texture bank to VRAM
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureDownloadBank(TextureBankType *bank);


/**************************************************************************
	FUNCTION:	textureDestroyBank()
	PURPOSE:	Free system RAM data for given texture bank
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureDestroyBank(TextureBankType *bank);


/**************************************************************************
	FUNCTION:	textureUnloadBank()
	PURPOSE:	Unload given texture bank from VRAM
	PARAMETERS:	Texture bank info ptr
	RETURNS:	
**************************************************************************/

void textureUnloadBank(TextureBankType *bank);


/**************************************************************************
	FUNCTION:	textureVRAMalloc()
	PURPOSE:	Allocate area of VRAM
	PARAMETERS:	Width, height in blocks
	RETURNS:	VRAM handle
**************************************************************************/

int textureVRAMalloc(short w, short h);


/**************************************************************************
	FUNCTION:	textureAddCLUT16()
	PURPOSE:	Download 16-color palette to VRAM
	PARAMETERS:	Ptr to palette
	RETURNS:	CLUT address
**************************************************************************/

unsigned short textureAddCLUT16(unsigned short *palette);


/**************************************************************************
	FUNCTION:	textureFindCRCInBank()
	PURPOSE:	Find given texture in bank
	PARAMETERS:	Texture bank info ptr, Texture name CRC
	RETURNS:	Texture info ptr
**************************************************************************/

TextureType *textureFindCRCInBank(TextureBankType *bank, unsigned long crc);


/**************************************************************************
	FUNCTION:	textureFindCRCInAllBanks()
	PURPOSE:	Find given texture in all loaded banks
	PARAMETERS:	Texture name CRC
	RETURNS:	Texture info ptr
**************************************************************************/

TextureType *textureFindCRCInAllBanks(unsigned long crc);


/**************************************************************************
	FUNCTION:	textureFindTextureN()
	PURPOSE:	Find given texture by number
	PARAMETERS:	Texture number
	RETURNS:	Texture info ptr
**************************************************************************/

TextureType *textureFindTextureN(int n);


/**************************************************************************
	FUNCTION:	textureReallocTextureBank()
	PURPOSE:	Reallocate texture in system RAM (e.g. to defragement)
	PARAMETERS:	Texture bank info ptr
	RETURNS:	New texture bank info ptr
**************************************************************************/

TextureBankType *textureReallocTextureBank(TextureBankType *textures);


/**************************************************************************
	FUNCTION:	textureShowVRAM
	PURPOSE:	Debug VRAM viewer
	PARAMETERS:	PAL mode 1/0
	RETURNS:	
**************************************************************************/

void textureShowVRAM(unsigned char palMode);


/**************************************************************************
	FUNCTION:	textureCreateAnimation
	PURPOSE:	Create an animated texture
	PARAMETERS:	pointer to dummy texture, pointer to array of animated textures, number of frames
	RETURNS:	pointer to animated texture
**************************************************************************/

TextureAnimType *textureCreateAnimation(TextureType *dummy, TextureType **anim, int numFrames);


/**************************************************************************
	FUNCTION:	textureSetAnimation
	PURPOSE:	Set frame of an animated texture
	PARAMETERS:	Animated texture, frame number
	RETURNS:	
**************************************************************************/

void textureSetAnimation(TextureAnimType *texAnim, int frameNum);


/**************************************************************************
	FUNCTION:	textureDestroyAnimation
	PURPOSE:	Destroy an animated texture
	PARAMETERS:	Animated texture
	RETURNS:	
**************************************************************************/

void textureDestroyAnimation(TextureAnimType *texAnim);


/**************************************************************************
	FUNCTION:	textureDownLoad
	PURPOSE:	Create a TextureType from an NSPRITE
	PARAMETERS:	NSPRITE pointer, TextureType pointer
	RETURNS:	
**************************************************************************/

void textureDownLoad(NSPRITE *nspr, TextureType *txPtr);

// useful macros for finding textures by name

#define textureFindInBank(b,n)		textureFindCRCInBank((b),utilStr2CRC((n)))
#define textureFindInAllBanks(n)	textureFindCRCInAllBanks(utilStr2CRC((n)))

// Just for fred :)
extern TextureBankType *texBank[MAXTEXBANKS];


#endif
