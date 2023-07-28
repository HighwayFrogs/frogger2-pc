#include "newpsx.h"
#include <islutil.h>
#include "main.h"

//temp file,
//just baby frogs as sprites here at the mo'.

//bbtemp till included
//#define MALLOC0(S)	memoryAllocateZero(S, __FILE__, __LINE__)



// TextureBankType* babySpriteBank = 0;
// TextureAnimType* newBabyAnim = 0;
// TextureType**	 newBabyAnimPtrs;
// TextureType*     newBabyAnimDest;
TextureBankType* garibSpriteBank = 0;
TextureAnimType* garibAnim = 0;
TextureType**	 garibTextures;
TextureType*     garibAnimDest;


#ifdef PSX_VERSION
void InitgaribAnim(void)
{
	int i;

	//load the 8 textures (+ 1 dummy in there too)
	garibSpriteBank = textureLoadBank("TEXTURES\\GARIB.SPT");

	//upload to VRAM
	textureDownloadBank(garibSpriteBank);

	//remove from main memory
	textureDestroyBank(garibSpriteBank);


	//find the dummy texture
	garibAnimDest = textureFindCRCInBank( garibSpriteBank, utilStr2CRC("DUMMY") );
										 
	//fill array of TextureType pointers
	garibTextures = MALLOC0( sizeof(TextureType*)*garibSpriteBank->numTextures );
	for(i=0; i<garibSpriteBank->numTextures; i++)
		garibTextures[i] = &garibSpriteBank->texture[i];


	//set up the anim structure
//	newBabyAnim = textureCreateAnimation(&newBabyAnimDest, &babySpriteBank->texture, babySpriteBank->numTextures);
	garibAnim = textureCreateAnimation(garibAnimDest, garibTextures, garibSpriteBank->numTextures);
}
#else
void InitNewBabyAnim(void)
{
}
#endif
