#ifndef _PSXSPRITE_H_INCLUDE
#define _PSXSPRITE_H_INCLUDE

#if PALMODE==1
	#define SCALEY 640
#else
	#define SCALEY 512
#endif

#define SCALEX 900

void PrintSpriteOverlays ( char num );
void InitTiledBackdrop(char *filename);
void DrawTiledBackdrop(int scroll);
void FreeTiledBackdrop();

extern  TextureType *tileTexture[4];

#endif