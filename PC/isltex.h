#ifndef _ISLTEX_H_INCLUDE
#define _ISLTEX_H_INCLUDE

#include <ddraw.h>
#include <d3d.h>
#include <windows.h>
#include <mdx.h>

#ifdef __cplusplus
extern "C"
{
#endif


//#define textureFindInAllBanks(n)	(0)

// jhubbard
#define textureFindCRCInAllBanks(x) GetTexEntryFromCRC(x)

typedef struct
{
	int temp;
} TextureType;

typedef struct
{
	int numTextures;
	TextureType* texture;
} TextureBankType;

typedef struct
{
	int temp;
} TextureAnimType;


TextureBankType *textureLoadBank(char *sFile);

void textureDownloadBank(TextureBankType *bank);
void textureDestroyBank(TextureBankType *bank);
TextureType *textureFindCRCInBank(TextureBankType *bank, unsigned long crc);
TextureAnimType *textureCreateAnimation(TextureType *dummy, TextureType **anim, int numFrames);

#ifdef __cplusplus
}
#endif

#endif