#ifndef _TEXTURES_H_INCLUDE
#define _TEXTURES_H_INCLUDE

#include <isltex.h>
#include "sonylibs.h"

#define MAX_TEXTURE_BANKS		5

typedef struct _LOADPAL_24BitRGBStr
{
	unsigned char r, g, b;
} LOADPAL_24BitRGBStr;
//////////////////////////////////////////////////////////////////
typedef struct _LOADPAL_PCPalette16Str
{
	LOADPAL_24BitRGBStr component[16];
} LOADPAL_PCPalette16Str;

extern TextureBankType *textureBanks [ MAX_TEXTURE_BANKS ];


typedef struct _TEXTUREANIM
{
	struct _TEXTUREANIM *next, *prev;			// ptr to next / prev platform
	unsigned short numFrames, frame;
	unsigned long lastTime;
	char txtName[20];
	TextureAnimType *animation;
} TEXTUREANIM;

typedef struct _TEXTUREANIMLIST
{
	int				numEntries;
	TEXTUREANIM	head;
} TEXTUREANIMLIST;

extern TEXTUREANIMLIST textureAnimList;

extern char *palNames [ 5 ];

// For PSX, no need to add .bmp extension
#define FindTexture(x) textureFindCRCInAllBanks(utilStr2CRC(x))

extern unsigned char *redpal[];
extern unsigned char *greenpal[];
extern unsigned char *bluepal[];
extern unsigned char *pinkpal[];
extern unsigned char *yellowpal[];

void LoadTextureBank			( int textureBank );
void LoadTextureAnimBank ( int textureBank );
void FreeTextureBank			( TextureBankType *textureBank );
void FreeAllTextureBanks	( void );

TEXTUREANIM *CreateTextureAnimation ( unsigned long crc, int numframes );
//void CreateTextureAnimation			( char *fileName, TextureType *dummy, int numFrames );
void InitTextureAnimLinkedList	( void );
void FreeTextureAnimList				( void );
void AddTextureAnim							( TEXTUREANIM *textureAnim );
void SubTextureAnim							( TEXTUREANIM *textureAnim );
void AddAnimFrame ( TEXTUREANIM *anim, unsigned long crc, short waitTime, int num );

void CopyTexture ( TextureType *dest, TextureType *src, int copyPalette );


#endif